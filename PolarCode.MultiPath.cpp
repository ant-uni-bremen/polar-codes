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
	for(int i=0; i<size; i+=FLOATSPERVECTOR)
	{
		vec LLRabs = load_ps(LLRin+i);
		LLRabs = and_ps(LLRabs, ABS_MASK);
		store_ps(LLRout+i, LLRabs);
	}
}

void PolarCode::Rate0_multiPath(int stage, int BitLocation)
{
	int size = 1<<stage;
	if(size<FLOATSPERVECTOR)
	{
		for(int path = 0; path < PathCount; ++path)
		{
			//Set the all-zero codeword
			memset(Bits[path].data()+BitLocation, 0, size<<2);
			float punishment = 0;
			for(int i=0; i<size; ++i)
			{
				//Decrease the path metric, if LLR was erroneous
				punishment += fmin(LLR[path][stage][i], 0.0);
			}
			Metric[path] += punishment;
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
				vec LLRin = load_ps(LLR[path][stage].data()+i);
				punishment = add_ps(punishment, min_ps(LLRin, ZERO));
			}
			float pun = reduce_add_ps(punishment);
			Metric[path] += pun;
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
		quick_abs(LLR[path][stage].data(), absLLR.data(), size);
		sorter.simplePartialSort(absLLR.data(), size, std::min(2,size));
		
		tmpCand.srcPath = path;
		//Candidate one
		tmpCand.decisionIndex = 0;
		tmpCand.nHints = 0;
		MLmetric = Metric[path];
		newMetrics[candCtr] = MLmetric;
		cand[candCtr++] = tmpCand;

		//Candidate two
		tmpCand.decisionIndex = 1;
		tmpCand.nHints = 0;
		tmpCand.hints[tmpCand.nHints++] = sorter.permuted[0];
		newMetrics[candCtr] = MLmetric - sorter.sorted[0];
		cand[candCtr++] = tmpCand;

		if(size >=2)
		{
			//Candidate three
			tmpCand.decisionIndex = 2;
			tmpCand.nHints = 0;
			tmpCand.hints[tmpCand.nHints++] = sorter.permuted[1];
			newMetrics[candCtr] = MLmetric - sorter.sorted[1];
			cand[candCtr++] = tmpCand;
	
			//Candidate four
			tmpCand.decisionIndex = 3;
			tmpCand.nHints = 0;
			tmpCand.hints[tmpCand.nHints++] = sorter.permuted[0];
			tmpCand.hints[tmpCand.nHints++] = sorter.permuted[1];
			newMetrics[candCtr] = MLmetric - sorter.sorted[0] - sorter.sorted[1];
			cand[candCtr++] = tmpCand;
		}
	}

	int newPathCount = std::min(L, candCtr);

	sorter.set(newMetrics, candCtr);
	sorter.stableSortDescending();
	
	for(int path=0; path<newPathCount; ++path)
	{
		Candidate selCand = cand[sorter.permuted[path]];//selected Candidate

		//Save everything
		for(int cstage=stage; cstage < n; ++cstage)
		{
			memcpy(newLLR[path][cstage].data(), LLR[selCand.srcPath][cstage].data(), 4<<cstage);
		}
		memcpy(newBits[path].data(), Bits[selCand.srcPath].data(), 4<<n);
		
		//Compose the new path
		Metric[path] = sorter.sorted[path];
		if(size<FLOATSPERVECTOR)
		{
			Rate1(newLLR[path][stage].data(), newBits[path].data()+BitLocation, size);
		}
		else
		{
			Rate1_vectorized(newLLR[path][stage].data(), newBits[path].data()+BitLocation, size);
		}
		
		//Flip the bits
		unsigned int *BitPtr = reinterpret_cast<unsigned int*>(newBits[path].data()+BitLocation);
		for(int i=0; i<selCand.nHints; ++i)
		{
			BitPtr[selCand.hints[i]] ^= 0x80000000;
		}	
	}

	std::swap(Bits, newBits);
	std::swap(LLR, newLLR);
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
		if(size<FLOATSPERVECTOR)
		{
			iLLR = reinterpret_cast<unsigned int*>(LLR[path][stage].data());
			unsigned int parity;
			parity = 0x80000000;
			for(int i=0; i<size; ++i)
			{
				parity ^= iLLR[i];
			}
			parityOK = !!(parity & 0x80000000);//check parity
		}
		else
		{
			vec parity = set1_ps(0.0);
			for(int i=0; i<size; i+=FLOATSPERVECTOR)
			{
				vec LLRv = load_ps(LLR[path][stage].data()+i);
				parity = xor_ps(parity, LLRv);
			}
			float par = reduce_xor_ps(parity);unsigned int *iPar=reinterpret_cast<unsigned int*>(&par);
			parityOK = !(*iPar & 0x80000000);
		}
		q = static_cast<float>(parityOK);

		//Find the 4 least reliable LLRs
		quick_abs(LLR[path][stage].data(), absLLR.data(), size);
		sorter.simplePartialSort(absLLR.data(), size, std::min(4,size));
		
		
		tmpCand.srcPath = path;
		//Candidate one
		tmpCand.decisionIndex = 0;
		tmpCand.nHints = 0;
		if(!parityOK) tmpCand.hints[tmpCand.nHints++] = sorter.permuted[0];
		MLmetric = Metric[path] - (1.0-q) * sorter.sorted[0];
		newMetrics[candCtr] = MLmetric;
		cand[candCtr++] = tmpCand;

		if(size >= 2)
		{
			//Candidate two
			tmpCand.decisionIndex = 1;
			tmpCand.nHints = 0;
			if(parityOK) tmpCand.hints[tmpCand.nHints++] = sorter.permuted[0];
			tmpCand.hints[tmpCand.nHints++] = sorter.permuted[1];
			newMetrics[candCtr] = MLmetric - q*sorter.sorted[0] - sorter.sorted[1];
			cand[candCtr++] = tmpCand;
	
			if(size >= 4 && L>2)
			{
				//Candidate three
				tmpCand.decisionIndex = 2;
				tmpCand.nHints = 0;
				if(parityOK) tmpCand.hints[tmpCand.nHints++] = sorter.permuted[0];
				tmpCand.hints[tmpCand.nHints++] = sorter.permuted[2];
				newMetrics[candCtr] = MLmetric - q*sorter.sorted[0] - sorter.sorted[2];
				cand[candCtr++] = tmpCand;
		
				//Candidate four
				tmpCand.decisionIndex = 3;
				tmpCand.nHints = 0;
				if(parityOK) tmpCand.hints[tmpCand.nHints++] = sorter.permuted[0];
				tmpCand.hints[tmpCand.nHints++] = sorter.permuted[3];
				newMetrics[candCtr] = MLmetric - q*sorter.sorted[0] - sorter.sorted[3];
				cand[candCtr++] = tmpCand;
		
				//Candidate five
				tmpCand.decisionIndex = 4;
				tmpCand.nHints = 0;
				if(!parityOK) tmpCand.hints[tmpCand.nHints++] = sorter.permuted[0];
				tmpCand.hints[tmpCand.nHints++] = sorter.permuted[1];
				tmpCand.hints[tmpCand.nHints++] = sorter.permuted[2];
				newMetrics[candCtr] = MLmetric - sorter.sorted[1] - sorter.sorted[2];
				cand[candCtr++] = tmpCand;
		
				//Candidate six
				tmpCand.decisionIndex = 5;
				tmpCand.nHints = 0;
				if(!parityOK) tmpCand.hints[tmpCand.nHints++] = sorter.permuted[0];
				tmpCand.hints[tmpCand.nHints++] = sorter.permuted[1];
				tmpCand.hints[tmpCand.nHints++] = sorter.permuted[3];
				newMetrics[candCtr] = MLmetric - sorter.sorted[1] - sorter.sorted[3];
				cand[candCtr++] = tmpCand;
		
				//Candidate seven
				tmpCand.decisionIndex = 6;
				tmpCand.nHints = 0;
				if(!parityOK) tmpCand.hints[tmpCand.nHints++] = sorter.permuted[0];
				tmpCand.hints[tmpCand.nHints++] = sorter.permuted[2];
				tmpCand.hints[tmpCand.nHints++] = sorter.permuted[3];
				newMetrics[candCtr] = MLmetric - sorter.sorted[2] - sorter.sorted[3];
				cand[candCtr++] = tmpCand;
		
				//Candidate eight
				tmpCand.decisionIndex = 7;
				tmpCand.nHints = 0;
				if(parityOK) tmpCand.hints[tmpCand.nHints++] = sorter.permuted[0];
				tmpCand.hints[tmpCand.nHints++] = sorter.permuted[1];
				tmpCand.hints[tmpCand.nHints++] = sorter.permuted[2];
				tmpCand.hints[tmpCand.nHints++] = sorter.permuted[3];
				newMetrics[candCtr] = MLmetric - q*sorter.sorted[0] - sorter.sorted[1] - sorter.sorted[2] - sorter.sorted[3];
				cand[candCtr++] = tmpCand;
			}
		}
	}
	
	int newPathCount = std::min(L, candCtr);	

	sorter.set(newMetrics, candCtr);
	sorter.stableSortDescending();

	for(int path=0; path<newPathCount; ++path)
	{
		Candidate selCand = cand[sorter.permuted[path]];//selected Candidate
		
		//Save everything
		for(int cstage=stage; cstage < n; ++cstage)
		{
			memcpy(newLLR[path][cstage].data(), LLR[selCand.srcPath][cstage].data(), 4<<cstage);
		}
		memcpy(newBits[path].data(), Bits[selCand.srcPath].data(), 4<<n);
		
		//Compose the new path
		Metric[path] = sorter.sorted[path];
		if(size<FLOATSPERVECTOR)
		{
			Rate1(newLLR[path][stage].data(), newBits[path].data()+BitLocation, size);
		}
		else
		{
			Rate1_vectorized(newLLR[path][stage].data(), newBits[path].data()+BitLocation, size);
		}

		//Flip the bits
		unsigned int *BitPtr = reinterpret_cast<unsigned int*>(newBits[path].data()+BitLocation);
		for(int i=0; i<selCand.nHints; ++i)
		{
			BitPtr[selCand.hints[i]] ^= 0x80000000;
		}	
	}

	std::swap(Bits, newBits);
	std::swap(LLR, newLLR);
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
		tmpCand0.decisionIndex = 0;
		MLmetric = Metric[path];
		Metric0 = MLmetric;

		//Candidate two
		tmpCand1.decisionIndex = 1;
		Metric1 = MLmetric;
		
		//Calculate the metrics
		if(size < FLOATSPERVECTOR)
		{
			for(int i=0; i<size; ++i)
			{
				if(LLR[path][stage][i] < 0.0)
				{
					Metric0 += LLR[path][stage][i];
				}
				else
				{
					Metric1 -= LLR[path][stage][i];
				}
			}
		}
		else
		{
			vec m0 = set1_ps(0.0), m1 = set1_ps(0.0);
			for(int i=0; i<size; i+=FLOATSPERVECTOR)
			{
				vec llr = load_ps(LLR[path][stage].data()+i);
				m0 = add_ps(m0, min_ps(llr, ZERO));
				m1 = add_ps(m1, max_ps(llr, ZERO));
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

	sorter.set(newMetrics, candCtr);
	sorter.stableSortDescending();

	for(int path=0; path<newPathCount; ++path)
	{
		Candidate selCand = cand[sorter.permuted[path]];//selected Candidate
			
		//Save everything
		for(int cstage=stage; cstage < n; ++cstage)
		{
			memcpy(newLLR[path][cstage].data(), LLR[selCand.srcPath][cstage].data(), 4<<cstage);
		}
		memcpy(newBits[path].data(), Bits[selCand.srcPath].data(), 4<<n);
		
		//Compose the new path
		Metric[path] = sorter.sorted[path];
		float value; unsigned int *iValue = reinterpret_cast<unsigned int*>(&value);
		*iValue = selCand.decisionIndex<<31;
		for(int bit=0; bit<size; ++bit)
		{
			newBits[path][BitLocation+bit] = value;
		}
	}

	std::swap(Bits, newBits);
	std::swap(LLR, newLLR);
	PathCount = newPathCount;
}

bool PolarCode::decodeMultiPath(vector<float> &decoded)
{
	Metric[0] = 0.0;
	PathCount = 1;

#ifdef FLEXIBLE_DECODING
	decodeMultiPathRecursive(n, 0, 0);
#else

#include "SpecialMultiPathDecoder.cpp"

#endif

	//Select the most likely path which passes the CRC test	
	for(int path=0; path<PathCount; ++path)
	{
		transform(Bits[path]);
		for(int bit=0; bit<K; ++bit)
		{
			decoded[bit] = Bits[path][AcceleratedLookup[bit]];
		}
		if(Crc->check(decoded))
		{
			return true;
		}
	}

	//Give out the most likely path, if no crc is fulfilled
	for(int bit=0; bit<K; ++bit)
	{
		decoded[bit] = Bits[0][AcceleratedLookup[bit]];
	}
	
	return false;
}

void PolarCode::decodeMultiPathRecursive(int stage, int BitLocation, int nodeID)
{
	int leftNode  = (nodeID<<1)+1;
	int rightNode = leftNode+1;
	int subStageLength = 1<<(stage-1);
	vector<float*> LLRptr(PathCount);

	//Calculate LLRs for all paths
	for(int currentPath=0; currentPath<PathCount; ++currentPath)
	{
		LLRptr[currentPath] = stage==n ? initialLLR.data() : LLR[currentPath][stage].data();
		F_function_hybrid(LLRptr[currentPath], LLR[currentPath][stage-1].data(), subStageLength);
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
			G_function_hybrid(LLRptr[currentPath], LLR[currentPath][stage-1].data(), Bits[currentPath].data()+BitLocation, subStageLength);
		}
	}
	else
	{
		for(int currentPath=0; currentPath<PathCount; ++currentPath)
		{
			G_function_0R_hybrid(LLRptr[currentPath], LLR[currentPath][stage-1].data(), subStageLength);
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
}

