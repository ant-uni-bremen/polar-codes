#include <cmath>
#include <cstring>
#include <algorithm>
#include <utility>

#include "PolarCode.h"
#include "ArrayFuncs.h"
#include "crc8.h"

using namespace std;

void PolarCode::quick_abs(float *LLRin, float *LLRout, int size)
{
	if(size >= FLOATSPERVECTOR)
	{
		for(int i=0; i<size; i+=FLOATSPERVECTOR)
		{
			vec LLRabs = load_ps(LLRin+i);
			LLRabs = andnot_ps(sgnMask256, LLRabs);
			store_ps(LLRout+i, LLRabs);
		}
	}
	else if(size == 4)
	{
		__m128 LLRabs = _mm_load_ps(LLRin);
		LLRabs = _mm_andnot_ps(sgnMask128, LLRabs);
		_mm_store_ps(LLRout, LLRabs);
	}
	else
	{
		unsigned int *iLLRin = reinterpret_cast<unsigned int*>(LLRin);
		unsigned int *iLLRout = reinterpret_cast<unsigned int*>(LLRout);
		for(int i=0; i<size; ++i)
		{
			iLLRout[i] = iLLRin[i]&0x7FFFFFFF;
		}
	}
}

void PolarCode::Rate0_multiPath(int stage, int BitLocation)
{
	int size = 1<<stage;
	if(size<4)
	{
		for(int path = 0; path < PathCount; ++path)
		{
			//Set the all-zero codeword
			memset(Bits[path].data()+BitLocation, 0, size<<2);
			float punishment = 0;
			for(int i=0; i<size; ++i)
			{
				//Decrease the path metric, if LLR was erroneous
				punishment += fmin(LlrTree[path][stage]->data[i], 0.0);
			}
			Metric[path] += punishment;
		}
	}
	else if(size == 4)
	{
		for(int path = 0; path < PathCount; ++path)
		{
			//Set the all-zero codeword
			memset(Bits[path].data()+BitLocation, 0, 16);

			//Decrease the path metric, if LLR was erroneous
			__m128 LLRin = _mm_load_ps(LlrTree[path][stage]->data);

			Metric[path] += _mm_reduce_add_ps(_mm_min_ps(LLRin, _mm_setzero_ps()));
		}
	}
	else
	{
		for(int path = 0; path < PathCount; ++path)
		{
			//Set the all-zero codeword
			memset(Bits[path].data()+BitLocation, 0, size<<2);
			vec punishment = set1_ps(0.0);
			for(int i=0; i<size; i+=FLOATSPERVECTOR)
			{
				//Decrease the path metric, if LLR was erroneous
				vec LLRin = load_ps(LlrTree[path][stage]->data+i);
				punishment = add_ps(punishment, min_ps(LLRin, _mm256_setzero_ps()));
			}
			Metric[path] += reduce_add_ps(punishment);
		}
	}
}

void PolarCode::Rate1_multiPath(int stage, int BitLocation)
{
	Candidate tmpCand;
	float MLmetric;
	int size = 1<<stage;
	
	int candCtr=0;
	
	//Generate candidates for all paths
	for(int path=0; path < PathCount; ++path)
	{
		//Find the 2 least reliable LLRs
		quick_abs(LlrTree[path][stage]->data, absLLR.data(), size);
		sorter->simplePartialSort(absLLR.data(), size, std::min(2,size));
		
		MLmetric = Metric[path];

		tmpCand.srcPath = path;
		//Candidate one, no bits flipped
		tmpCand.nHints = 0;
		newMetrics[candCtr] = MLmetric;
		cand[candCtr++] = tmpCand;

		//Candidate two, least reliable bit flipped
		tmpCand.nHints = 1;
		tmpCand.hints[0] = sorter->permuted[0];
		newMetrics[candCtr] = MLmetric - absLLR[0];
		cand[candCtr++] = tmpCand;

		if(size >=2)
		{
			//Candidate three, second least reliable bit flipped
			tmpCand.nHints = 1;
			tmpCand.hints[0] = sorter->permuted[1];
			newMetrics[candCtr] = MLmetric - absLLR[1];
			cand[candCtr++] = tmpCand;
	
			//Candidate four, two bits flipped
			tmpCand.nHints = 2;
			tmpCand.hints[0] = sorter->permuted[0];
			tmpCand.hints[1] = sorter->permuted[1];
			newMetrics[candCtr] = MLmetric - absLLR[0] - absLLR[1];
			cand[candCtr++] = tmpCand;
		}
	}

	int newPathCount = std::min(L, candCtr);

	sorter->set(newMetrics, candCtr);
	sorter->partialSortDescending(newPathCount);

	vector<int> lastOne(PathCount, -1);
	for(int path=0; path<newPathCount; ++path)
	{
		lastOne[cand[sorter->permuted[path]].srcPath] = path;
	}

	for(int path=0; path<newPathCount; ++path)
	{
		Candidate selCand = cand[sorter->permuted[path]];//selected Candidate

		//Save everything
		for(int cstage=stage; cstage < n; ++cstage)
		{
			newLlrTree[path][cstage] = pool->lazyDuplicate(LlrTree[selCand.srcPath][cstage]);
		}

		if(path != lastOne[selCand.srcPath])
		{
			memcpy(newBits[path].data(), Bits[selCand.srcPath].data(), 4<<n);
		}
		else
		{
			std::swap(newBits[path], Bits[selCand.srcPath]);
		}

		//Compose the new path
		Rate1(newLlrTree[path][stage]->data, newBits[path].data()+BitLocation, size);
		
		//Flip the bits
		unsigned int *BitPtr = reinterpret_cast<unsigned int*>(newBits[path].data()+BitLocation);
		for(int i=0; i<selCand.nHints; ++i)
		{
			BitPtr[selCand.hints[i]] ^= 0x80000000;
		}	
	}

	for(int path = 0; path < PathCount; ++path)
	{
		for(int cstage=stage; cstage < n; ++cstage)
		{
			pool->release(LlrTree[path][cstage]);
		}
	}

	std::swap(Metric, newMetrics);
	std::swap(Bits, newBits);
	std::swap(LlrTree, newLlrTree);
	PathCount = newPathCount;
}

void PolarCode::SPC_multiPath(int stage, int BitLocation)
{
	Candidate tmpCand;
	unsigned int *iLLR;
	
	bool parityOK;
	float q;
	float MLmetric;
	int size = 1<<stage;
	int candCtr=0;
	
	//Generate candidates for all paths
	for(int path=0; path < PathCount; ++path)
	{
		if(size<4)
		{
			iLLR = reinterpret_cast<unsigned int*>(LlrTree[path][stage]->data);
			unsigned int parity;
			parity = 0x80000000;
			for(int i=0; i<size; ++i)
			{
				parity ^= iLLR[i];
			}
			parityOK = !!(parity & 0x80000000);//check parity
		}
		else if(size == 4)
		{
			float parity = _mm_reduce_xor_ps(_mm_and_ps(sgnMask128, _mm_load_ps(LlrTree[path][stage]->data)));
			unsigned *iPar=reinterpret_cast<unsigned*>(&parity);
			parityOK = !(*iPar);
		}
		else
		{
			cerr << "Long SPC!!!"<<endl;
			vec parity = _mm256_setzero_ps();
			for(int i=0; i<size; i+=FLOATSPERVECTOR)
			{
				vec LLRv = load_ps(LlrTree[path][stage]->data+i);
				parity = xor_ps(parity, LLRv);
			}
			float par = reduce_xor_ps(parity);unsigned int *iPar=reinterpret_cast<unsigned int*>(&par);
			parityOK = !(*iPar & 0x80000000);
		}
		q = static_cast<float>(parityOK);

		//Find the 4 least reliable LLRs
		quick_abs(LlrTree[path][stage]->data, absLLR.data(), size);
		sorter->simplePartialSort(absLLR.data(), size, std::min(4,size));
		
		
		tmpCand.srcPath = path;
		//Candidate one
		tmpCand.nHints = 0;
		if(!parityOK) tmpCand.hints[tmpCand.nHints++] = sorter->permuted[0];
		MLmetric = Metric[path] - (1.0-q) * absLLR[0];
		newMetrics[candCtr] = MLmetric;
		cand[candCtr++] = tmpCand;

		if(size >= 2)
		{
			//Candidate two
			tmpCand.nHints = 1;
			tmpCand.hints[0] = sorter->permuted[1];
			if(parityOK) tmpCand.hints[tmpCand.nHints++] = sorter->permuted[0];
			newMetrics[candCtr] = MLmetric - q*absLLR[0] - absLLR[1];
			cand[candCtr++] = tmpCand;
	
			if(size >= 4 && L>2)
			{
				//Candidate three
				tmpCand.nHints = 1;
				tmpCand.hints[0] = sorter->permuted[2];
				if(parityOK) tmpCand.hints[tmpCand.nHints++] = sorter->permuted[0];
				newMetrics[candCtr] = MLmetric - q*absLLR[0] - absLLR[2];
				cand[candCtr++] = tmpCand;
		
				//Candidate four
				tmpCand.nHints = 1;
				tmpCand.hints[tmpCand.nHints++] = sorter->permuted[3];
				if(parityOK) tmpCand.hints[tmpCand.nHints++] = sorter->permuted[0];
				newMetrics[candCtr] = MLmetric - q*absLLR[0] - absLLR[3];
				cand[candCtr++] = tmpCand;
		
				//Candidate five
				tmpCand.nHints = 2;
				tmpCand.hints[0] = sorter->permuted[1];
				tmpCand.hints[1] = sorter->permuted[2];
				if(!parityOK) tmpCand.hints[tmpCand.nHints++] = sorter->permuted[0];
				newMetrics[candCtr] = MLmetric - absLLR[1] - absLLR[2];
				cand[candCtr++] = tmpCand;
		
				//Candidate six
				tmpCand.nHints = 2;
				tmpCand.hints[0] = sorter->permuted[1];
				tmpCand.hints[1] = sorter->permuted[3];
				if(!parityOK) tmpCand.hints[tmpCand.nHints++] = sorter->permuted[0];
				newMetrics[candCtr] = MLmetric - absLLR[1] - absLLR[3];
				cand[candCtr++] = tmpCand;
		
				//Candidate seven
				tmpCand.nHints = 2;
				tmpCand.hints[0] = sorter->permuted[2];
				tmpCand.hints[1] = sorter->permuted[3];
				if(!parityOK) tmpCand.hints[tmpCand.nHints++] = sorter->permuted[0];
				newMetrics[candCtr] = MLmetric - absLLR[2] - absLLR[3];
				cand[candCtr++] = tmpCand;
		
				//Candidate eight
				tmpCand.nHints = 3;
				tmpCand.hints[0] = sorter->permuted[1];
				tmpCand.hints[1] = sorter->permuted[2];
				tmpCand.hints[2] = sorter->permuted[3];
				if(parityOK) tmpCand.hints[tmpCand.nHints++] = sorter->permuted[0];
				newMetrics[candCtr] = MLmetric - q*absLLR[0] - absLLR[1] - absLLR[2] - absLLR[3];
				cand[candCtr++] = tmpCand;
			}
		}
	}
	
	int newPathCount = std::min(L, candCtr);	

	sorter->set(newMetrics, candCtr);
	sorter->partialSortDescending(newPathCount);

	vector<int> lastOne(PathCount, -1);
	for(int path=0; path<newPathCount; ++path)
	{
		lastOne[cand[sorter->permuted[path]].srcPath] = path;
	}

	for(int path=0; path<newPathCount; ++path)
	{
		Candidate selCand = cand[sorter->permuted[path]];//selected Candidate

		//Save everything
		for(int cstage=stage; cstage < n; ++cstage)
		{
			newLlrTree[path][cstage] = pool->lazyDuplicate(LlrTree[selCand.srcPath][cstage]);
		}

		if(path != lastOne[selCand.srcPath])
		{
			memcpy(newBits[path].data(), Bits[selCand.srcPath].data(), 4<<n);
		}
		else
		{
			std::swap(newBits[path], Bits[selCand.srcPath]);
		}

		//Compose the new path
		Rate1(newLlrTree[path][stage]->data, newBits[path].data()+BitLocation, size);

		//Flip the bits
		unsigned int *BitPtr = reinterpret_cast<unsigned int*>(newBits[path].data()+BitLocation);
		for(int i=0; i<selCand.nHints; ++i)
		{
			BitPtr[selCand.hints[i]] ^= 0x80000000;
		}	
	}

	for(int path = 0; path < PathCount; ++path)
	{
		for(int cstage=stage; cstage < n; ++cstage)
		{
			pool->release(LlrTree[path][cstage]);
		}
	}

	std::swap(Metric, newMetrics);
	std::swap(Bits, newBits);
	std::swap(LlrTree, newLlrTree);
	PathCount = newPathCount;
}

void PolarCode::Repetition_multiPath(int stage, int BitLocation)
{
	Candidate tmpCand0,tmpCand1;
	float MLmetric, Metric0, Metric1;
	int size = 1<<stage;
	int candCtr=0;
	
	//Generate candidates for all paths
	for(int path=0; path < PathCount; ++path)
	{	
		tmpCand0.srcPath = path;
		tmpCand1.srcPath = path;
		
		//Candidate one
		tmpCand0.nHints = 0;//Note: 'decisionIndex' removed, nHints used instead
		MLmetric = Metric[path];
		Metric0 = MLmetric;

		//Candidate two
		tmpCand1.nHints = 1;
		Metric1 = MLmetric;
		
		//Calculate the metrics
		if(size < 4)
		{
			float l;
			for(int i=0; i<size; ++i)
			{
				l = LlrTree[path][stage]->data[i];
				if(l < 0.0)
				{
					Metric0 += l;
				}
				else
				{
					Metric1 -= l;
				}
			}
		}
		else if(size == 4)
		{
			__m128 llr = _mm_load_ps(LlrTree[path][stage]->data);
			Metric0 += _mm_reduce_add_ps(_mm_min_ps(llr, _mm_setzero_ps()));
			Metric1 -= _mm_reduce_add_ps(_mm_max_ps(llr, _mm_setzero_ps()));
		}
		else
		{
			vec m0 = _mm256_setzero_ps(), m1 = m0;
			for(int i=0; i<size; i+=FLOATSPERVECTOR)
			{
				vec llr = load_ps(LlrTree[path][stage]->data+i);
				m0 = add_ps(m0, min_ps(llr, _mm256_setzero_ps()));
				m1 = add_ps(m1, max_ps(llr, _mm256_setzero_ps()));
			}
			Metric0 += reduce_add_ps(m0);
			Metric1 -= reduce_add_ps(m1);
		}

		newMetrics[candCtr] = Metric0;
		cand[candCtr++] = tmpCand0;

		newMetrics[candCtr] = Metric1;
		cand[candCtr++] = tmpCand1;
	}

	int newPathCount = std::min(L, candCtr);	

	sorter->set(newMetrics, candCtr);
	sorter->partialSortDescending(newPathCount);

	vector<int> lastOne(PathCount, -1);
	for(int path=0; path<newPathCount; ++path)
	{
		lastOne[cand[sorter->permuted[path]].srcPath] = path;
	}

	for(int path=0; path<newPathCount; ++path)
	{
		Candidate selCand = cand[sorter->permuted[path]];//selected Candidate

		//Save everything
		for(int cstage=stage; cstage < n; ++cstage)
		{
			newLlrTree[path][cstage] = pool->lazyDuplicate(LlrTree[selCand.srcPath][cstage]);
		}

		if(path != lastOne[selCand.srcPath])
		{
			memcpy(newBits[path].data(), Bits[selCand.srcPath].data(), 4<<n);
		}
		else
		{
			std::swap(newBits[path], Bits[selCand.srcPath]);
		}

		//Compose the new path
		float value; unsigned int *iValue = reinterpret_cast<unsigned int*>(&value);
		*iValue = selCand.nHints<<31;//nHints = bit decision
		if(size < 4)
		{
			for(int bit=0; bit<size; ++bit)
			{
				newBits[path][BitLocation+bit] = value;
			}
		}
		else if(size == 4)
		{
			__m128 valueVec = _mm_set1_ps(value);
			_mm_store_ps(newBits[path].data()+BitLocation, valueVec);
		}
		else
		{
			vec valueVector = set1_ps(value);
			float *bitPtr = newBits[path].data()+BitLocation;
			for(int bit=0; bit<size; bit+=FLOATSPERVECTOR)
			{
				store_ps(bitPtr+bit, valueVector);
			}
		}
	}

	for(int path = 0; path < PathCount; ++path)
	{
		for(int cstage=stage; cstage < n; ++cstage)
		{
			pool->release(LlrTree[path][cstage]);
		}
	}

	std::swap(Metric, newMetrics);
	std::swap(Bits, newBits);
	std::swap(LlrTree, newLlrTree);
	PathCount = newPathCount;
}

bool PolarCode::decodeMultiPath(unsigned char* decoded)
{
	Metric[0] = 0.0;
	PathCount = 1;

	decodeMultiPathRecursive(n, 0, 0);

	if(useCRC)
	{
		//Select the most likely path which passes the CRC test
#ifndef SYSTEMATIC_CODING
		for(int path=0; path<PathCount; ++path)
		{
			transform(Bits[path]);
		}
#endif

		int bytes = K>>3;
		int bit = 0;
		unsigned int **iBit = new unsigned int*[L];
		for(int path=0; path<PathCount; ++path)
		{
			iBit[path] = reinterpret_cast<unsigned int*>(Bits[path].data());
		}
		unsigned char *thisByte = new unsigned char[PathCount];
		int bitIndex;
		for(int byte = 0; byte<bytes; ++byte)
		{
			memset(thisByte, 0, PathCount);
			for(int b=0;b<8;++b)
			{
				bitIndex = AcceleratedLookup[bit++];
				for(int path=0; path<PathCount; ++path)
				{
					thisByte[path] |= (iBit[path][bitIndex]>>(24+b));
				}
			}
			for(int path=0; path<PathCount; ++path)
			{
				decodedData[path][byte] = thisByte[path];
			}
		}
		delete [] thisByte;
		delete [] iBit;
		int bestIndex = Crc->multiCheck(decodedData, PathCount, bytes);
		if(bestIndex == -1)
		{
			memcpy(decoded, decodedData[0], bytes);
			return false;
		}
		else
		{
			memcpy(decoded, decodedData[bestIndex], bytes);
			return true;
		}
	}
	else
	{
		//Select the most likely path
#ifndef SYSTEMATIC_CODING
		transform(Bits[0]);
#endif
	}

	//Give out the most likely path, if no crc is used or passed the check
	int bytes = K>>3;
	int bit = 0;
	unsigned int *iBit = reinterpret_cast<unsigned int*>(Bits[0].data());
	for(int byte = 0; byte<bytes; ++byte)
	{
		unsigned char thisByte = 0;
		for(int b=0;b<8;++b)
		{
			thisByte |= (iBit[AcceleratedLookup[bit++]]>>(24+b));
		}
		decoded[byte] = thisByte;
	}

	/* Return true, if CRC-check is deactivated */	
	return !useCRC;
}

void PolarCode::decodeMultiPathRecursive(int stage, int BitLocation, int nodeID)
{
	int leftNode  = (nodeID<<1)+1;
	int rightNode = leftNode+1;
	int subStageLength = 1<<(stage-1);

	//Calculate LLRs for all paths
	for(int currentPath=0; currentPath<PathCount; ++currentPath)
	{
		LlrTree[currentPath][stage-1] = pool->allocate(stage-1);
		F_function(stage==n ? initialLLR : LlrTree[currentPath][stage]->data
				  , LlrTree[currentPath][stage-1]->data, subStageLength);
	}

	switch(simplifiedTree[leftNode])
	{
	case RateZero:
		/* In contrast to the L=1-special case decoding, here we can't omit the
		 * left Rate0()-call, because the path metrics have to be updated
		 */
		Rate0_multiPath(stage-1, BitLocation);
		break;
	case RateOne:
		Rate1_multiPath(stage-1, BitLocation);
		break;
	case RepetitionNode:
	case RateHalf:
		Repetition_multiPath(stage-1, BitLocation);
		break;
	case SPCnode:
		SPC_multiPath(stage-1, BitLocation);
		break;
	default:
		decodeMultiPathRecursive(stage-1, BitLocation, leftNode);
	}

	/* Now, for all the paths, generate the bit decision-dependent
	   subpaths and proceed as above */
	if(simplifiedTree[leftNode] != RateZero)
	{
		for(int currentPath=0; currentPath<PathCount; ++currentPath)
		{
			if(LlrTree[currentPath][stage-1]->useCount > 1)
			{
				LlrTree[currentPath][stage-1] = pool->duplicate(LlrTree[currentPath][stage-1]);
			}
			G_function(stage==n ? initialLLR : LlrTree[currentPath][stage]->data
					  , LlrTree[currentPath][stage-1]->data, Bits[currentPath].data()+BitLocation, subStageLength);
		}
	}
	else
	{
		for(int currentPath=0; currentPath<PathCount; ++currentPath)
		{
			if(LlrTree[currentPath][stage-1]->useCount > 1)
			{
				LlrTree[currentPath][stage-1] = pool->duplicate(LlrTree[currentPath][stage-1]);
			}
			G_function_0R(stage==n ? initialLLR : LlrTree[currentPath][stage]->data
						 , LlrTree[currentPath][stage-1]->data, subStageLength);
		}
	}
	
	switch(simplifiedTree[rightNode])
	{
	case RateZero:
		Rate0_multiPath(stage-1, BitLocation+subStageLength);
		break;
	case RateOne:
		Rate1_multiPath(stage-1, BitLocation+subStageLength);
		break;
	case RepetitionNode:
	case RateHalf:
		Repetition_multiPath(stage-1, BitLocation+subStageLength);
		break;
	case SPCnode:
		SPC_multiPath(stage-1, BitLocation+subStageLength);
		break;
	default:
		decodeMultiPathRecursive(stage-1, BitLocation+subStageLength, rightNode);
	}
	
	/* Finally merge both bit arrays for all paths */
	if(simplifiedTree[leftNode] != RateZero)
	{
		for(int currentPath=0; currentPath<PathCount; ++currentPath)
		{
			CombineSimple(Bits[currentPath].data()+BitLocation, subStageLength);
		}
	}
	else
	{
		for(int currentPath=0; currentPath<PathCount; ++currentPath)
		{
			Combine_0RSimple(Bits[currentPath].data()+BitLocation, subStageLength);
		}
	}

	for(int currentPath=0; currentPath<PathCount; ++currentPath)
	{
		pool->release(LlrTree[currentPath][stage-1]);
	}
}

