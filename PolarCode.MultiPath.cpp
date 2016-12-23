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

void PolarCode::Combine(float *BitsIn_l, float *BitsIn_r, float *BitsOut, int size)
{
	for(int i=0; i<size; i+=FLOATSPERVECTOR)
	{
		vec Bitsl = load_ps(BitsIn_l+i);
		vec Bitsr = load_ps(BitsIn_r+i);
		vec Bitso = xor_ps(Bitsl, Bitsr);
		store_ps(BitsOut+i, Bitso);
//		store_ps(BitsOut+size+i, Bitsr);
	}
	memcpy(BitsOut+size, BitsIn_r, size<<2);
}

void PolarCode::Combine_0R(float *BitsIn_r, float *BitsOut, int size)
{
	memcpy(BitsOut,      BitsIn_r, size<<2);
	memcpy(BitsOut+size, BitsIn_r, size<<2);
}

void PolarCode::Rate0_multiPath(int stage, int BitLocation)
{
	int size = 1<<stage;
	for(int path = 0; path < PathCount; ++path)
	{
		//Set the all-zero codeword
		memset(Bits[path].data()+BitLocation, 0, size*sizeof(float));
		float punishment = 0;
		for(int bit=0; bit<size; ++bit)
		{
			//Bits[path][stage][BitLocation][bit] = 0.0;
			//And decrease the path metric, if LLR was erroneous
			punishment += fmin(LLR[path][stage][bit], 0.0);
		}
		Metric[path] += punishment;
	}
}

void PolarCode::Rate1_multiPath(int stage, int BitLocation)
{
	vector<Candidate> cand; Candidate tmpCand;
	vector<float> newMetrics;
	aligned_float_vector absLLR;
	trackingSorter sorter;
	float MLmetric;
	int size = 1<<stage;
	
	int candCtr=0;
	cand.resize(PathCount<<2);
	newMetrics.assign(PathCount<<2, INFINITY);
	absLLR.resize(std::max(size, FLOATSPERVECTOR));
	
	//Generate candidates for all paths
	for(int path=0; path < PathCount; ++path)
	{
		//Find the 2 least reliable LLRs
		quick_abs(LLR[path][stage].data(), absLLR.data(), size);
		sorter.set(absLLR, size);
		sorter.partialSort(2);
		
		tmpCand.srcPath = path;
		//Candidate one
		tmpCand.decisionIndex = 0;
		tmpCand.nHints = 0;
		MLmetric = Metric[path];
		tmpCand.newMetric = MLmetric;
		newMetrics[candCtr] = tmpCand.newMetric;
		cand[candCtr++] = tmpCand;

		//Candidate two
		tmpCand.decisionIndex = 1;
		tmpCand.nHints = 0;
		tmpCand.hints[tmpCand.nHints++] = sorter.permuted[0];
		tmpCand.newMetric = MLmetric - sorter.sorted[0];
		newMetrics[candCtr] = tmpCand.newMetric;
		cand[candCtr++] = tmpCand;

		if(size >=2)
		{
			//Candidate three
			tmpCand.decisionIndex = 2;
			tmpCand.nHints = 0;
			tmpCand.hints[tmpCand.nHints++] = sorter.permuted[1];
			tmpCand.newMetric = MLmetric - sorter.sorted[1];
			newMetrics[candCtr] = tmpCand.newMetric;
			cand[candCtr++] = tmpCand;
	
			//Candidate four
			tmpCand.decisionIndex = 3;
			tmpCand.nHints = 0;
			tmpCand.hints[tmpCand.nHints++] = sorter.permuted[0];
			tmpCand.hints[tmpCand.nHints++] = sorter.permuted[1];
			tmpCand.newMetric = MLmetric - sorter.sorted[0] - sorter.sorted[1];
			newMetrics[candCtr] = tmpCand.newMetric;
			cand[candCtr++] = tmpCand;
		}
	}

	absLLR.clear();

	int newPathCount = std::min(L, candCtr);

	sorter.set(newMetrics);
	sorter.partialSortDescending(newPathCount);
	
//	newLLR.resize(newPathCount);
//	newBits.resize(newPathCount);
	Metric.resize(newPathCount);
	
	for(int path=0; path<newPathCount; ++path)
	{
		Candidate &selCand = cand[sorter.permuted[path]];//selected Candidate
		
//		newLLR[path].resize(n+1);
//		newBits[path].resize(N);
		//Save everything
		for(int cstage=0; cstage <= n; ++cstage)
		{
//			newLLR[path][cstage].resize(1<<cstage);
			memcpy(newLLR[path][cstage].data(), LLR[selCand.srcPath][cstage].data(), 4<<cstage);
		}
		memcpy(newBits[path].data(), Bits[selCand.srcPath].data(), 4<<n);
		
		//Compose the new path
		Metric[path] = selCand.newMetric;
		Rate1(newLLR[path][stage].data(), newBits[path].data()+BitLocation, 1<<stage);
		for(int i=0; i<selCand.nHints; ++i)
		{
			//Flip the bits
			*(reinterpret_cast<unsigned int*>(newBits[path].data()+BitLocation)+selCand.hints[i]) ^= 0x80000000;
		}	
	}
/*	Bits.clear();
	LLR.clear();
	Bits = std::move(newBits);
	LLR = std::move(newLLR);*/
	std::swap(Bits, newBits);
	std::swap(LLR, newLLR);
	PathCount = newPathCount;
}

void PolarCode::SPC_multiPath(int stage, int BitLocation)
{
	vector<Candidate> cand; Candidate tmpCand;
	vector<float> newMetrics;
	aligned_float_vector absLLR;
	trackingSorter sorter;
	unsigned int *iLLR;
	unsigned int parity;
	bool parityOK;
	float MLmetric;
	int size = 1<<stage;
	int candCtr=0;
	
	cand.resize(PathCount<<3);
	newMetrics.assign(PathCount<<3, INFINITY);
	absLLR.resize(std::max(size, FLOATSPERVECTOR));
	
	//Generate candidates for all paths
	for(int path=0; path < PathCount; ++path)
	{
		//Find the 4 least reliable LLRs
		quick_abs(LLR[path][stage].data(), absLLR.data(), size);
		sorter.set(absLLR, size);
		sorter.partialSort(4);
		
		iLLR = reinterpret_cast<unsigned int*>(LLR[path][stage].data());
		parity = 0x80000000;
		for(int i=0; i<size; ++i)
		{
			parity ^= iLLR[i];//XOR the parity bit, not caring about the magnitudes
		}
		parityOK = parity & 0x80000000;//check parity
		
		tmpCand.srcPath = path;
		//Candidate one
		tmpCand.decisionIndex = 0;
		tmpCand.nHints = 0;
		if(!parityOK) tmpCand.hints[tmpCand.nHints++] = sorter.permuted[0];
		MLmetric = Metric[path];
		if(!parityOK) MLmetric -= sorter.sorted[0];
		tmpCand.newMetric = MLmetric;
		newMetrics[candCtr] = tmpCand.newMetric;
		cand[candCtr++] = tmpCand;

		if(size >= 2)
		{
			//Candidate two
			tmpCand.decisionIndex = 1;
			tmpCand.nHints = 0;
			if(parityOK) tmpCand.hints[tmpCand.nHints++] = sorter.permuted[0];
			tmpCand.hints[tmpCand.nHints++] = sorter.permuted[1];
			tmpCand.newMetric = MLmetric - parityOK*sorter.sorted[0] - sorter.sorted[1];
			newMetrics[candCtr] = tmpCand.newMetric;
			cand[candCtr++] = tmpCand;
	
			if(size >= 4)
			{
				//Candidate three
				tmpCand.decisionIndex = 2;
				tmpCand.nHints = 0;
				if(parityOK) tmpCand.hints[tmpCand.nHints++] = sorter.permuted[0];
				tmpCand.hints[tmpCand.nHints++] = sorter.permuted[2];
				tmpCand.newMetric = MLmetric - parityOK*sorter.sorted[0] - sorter.sorted[2];
				newMetrics[candCtr] = tmpCand.newMetric;
				cand[candCtr++] = tmpCand;
		
				//Candidate four
				tmpCand.decisionIndex = 3;
				tmpCand.nHints = 0;
				if(parityOK) tmpCand.hints[tmpCand.nHints++] = sorter.permuted[0];
				tmpCand.hints[tmpCand.nHints++] = sorter.permuted[3];
				tmpCand.newMetric = MLmetric - parityOK*sorter.sorted[0] - sorter.sorted[3];
				newMetrics[candCtr] = tmpCand.newMetric;
				cand[candCtr++] = tmpCand;
		
				//Candidate five
				tmpCand.decisionIndex = 4;
				tmpCand.nHints = 0;
				tmpCand.hints[tmpCand.nHints++] = sorter.permuted[1];
				tmpCand.hints[tmpCand.nHints++] = sorter.permuted[2];
				tmpCand.newMetric = MLmetric - sorter.sorted[1] - sorter.sorted[2];
				newMetrics[candCtr] = tmpCand.newMetric;
				cand[candCtr++] = tmpCand;
		
				//Candidate six
				tmpCand.decisionIndex = 5;
				tmpCand.nHints = 0;
				tmpCand.hints[tmpCand.nHints++] = sorter.permuted[1];
				tmpCand.hints[tmpCand.nHints++] = sorter.permuted[3];
				tmpCand.newMetric = MLmetric - sorter.sorted[1] - sorter.sorted[3];
				newMetrics[candCtr] = tmpCand.newMetric;
				cand[candCtr++] = tmpCand;
		
				//Candidate seven
				tmpCand.decisionIndex = 6;
				tmpCand.nHints = 0;
				tmpCand.hints[tmpCand.nHints++] = sorter.permuted[2];
				tmpCand.hints[tmpCand.nHints++] = sorter.permuted[3];
				tmpCand.newMetric = MLmetric - sorter.sorted[2] - sorter.sorted[3];
				newMetrics[candCtr] = tmpCand.newMetric;
				cand[candCtr++] = tmpCand;
		
				//Candidate eight
				tmpCand.decisionIndex = 7;
				tmpCand.nHints = 0;
				if(parityOK) tmpCand.hints[tmpCand.nHints++] = sorter.permuted[0];
				tmpCand.hints[tmpCand.nHints++] = sorter.permuted[1];
				tmpCand.hints[tmpCand.nHints++] = sorter.permuted[2];
				tmpCand.hints[tmpCand.nHints++] = sorter.permuted[3];
				tmpCand.newMetric = MLmetric - parityOK*sorter.sorted[0] - sorter.sorted[1] - sorter.sorted[2] - sorter.sorted[3];
				newMetrics[candCtr] = tmpCand.newMetric;
				cand[candCtr++] = tmpCand;
			}
		}
	}
	
	absLLR.clear();

	int newPathCount = std::min(L, candCtr);	

	sorter.set(newMetrics);
	sorter.partialSortDescending(newPathCount);
	
//	newLLR.resize(newPathCount);
//	newBits.resize(newPathCount);
	Metric.resize(newPathCount);
	
	for(int path=0; path<newPathCount; ++path)
	{
		Candidate &selCand = cand[sorter.permuted[path]];//selected Candidate
		
//		newLLR[path].resize(n+1);
//		newBits[path].resize(N);
		//Save everything
		for(int cstage=0; cstage <= n; ++cstage)
		{
//			newLLR[path][cstage].resize(1<<cstage);
			memcpy(newLLR[path][cstage].data(), LLR[selCand.srcPath][cstage].data(), 4<<cstage);
		}
		memcpy(newBits[path].data(), Bits[selCand.srcPath].data(), 4<<n);
		
		//Compose the new path
		Metric[path] = selCand.newMetric;
		Rate1(newLLR[path][stage].data(), newBits[path].data()+BitLocation, 1<<stage);
		for(int i=0; i<selCand.nHints; ++i)
		{
			//Flip the bits
			*(reinterpret_cast<unsigned int*>(newBits[path].data()+BitLocation)+selCand.hints[i]) ^= 0x80000000;
		}
	}

/*	Bits.clear();
	LLR.clear();
	Bits = std::move(newBits);
	LLR = std::move(newLLR);*/
	std::swap(Bits, newBits);
	std::swap(LLR, newLLR);
	PathCount = newPathCount;
}

void PolarCode::Repetition_multiPath(int stage, int BitLocation)
{
	vector<Candidate> cand; Candidate tmpCand0,tmpCand1;
	vector<float> newMetrics;
	trackingSorter sorter;
	float MLmetric;
	int size = 1<<stage;
	int candCtr=0;
	
	cand.resize(PathCount<<1);
	newMetrics.assign(PathCount<<1, INFINITY);
	
	//Generate candidates for all paths
	for(int path=0; path < PathCount; ++path)
	{	
		tmpCand0.srcPath = path;
		tmpCand1.srcPath = path;
		
		//Candidate one
		tmpCand0.decisionIndex = 0;
		MLmetric = Metric[path];
		tmpCand0.newMetric = MLmetric;

		//Candidate two
		tmpCand1.decisionIndex = 1;
		tmpCand1.newMetric = MLmetric;
		
		//Calculate the metrics
		for(int i=0; i<size; ++i)
		{
			if(LLR[path][stage][i] < 0.0)
			{
				tmpCand0.newMetric += LLR[path][stage][i];
			}
			else
			{
				tmpCand1.newMetric -= LLR[path][stage][i];
			}
		}

		newMetrics[candCtr] = tmpCand0.newMetric;
		cand[candCtr++] = tmpCand0;

		newMetrics[candCtr] = tmpCand1.newMetric;
		cand[candCtr++] = tmpCand1;
	}

	int newPathCount = std::min(L, candCtr);	

	sorter.set(newMetrics);
	sorter.partialSortDescending(newPathCount);
	
//	newLLR.resize(newPathCount);
//	newBits.resize(newPathCount);
	Metric.resize(newPathCount);
	
	for(int path=0; path<newPathCount; ++path)
	{
		Candidate &selCand = cand[sorter.permuted[path]];//selected Candidate
			
//		newLLR[path].resize(n+1);
//		newBits[path].resize(N);
		//Save everything
		for(int cstage=0; cstage <= n; ++cstage)
		{
//			newLLR[path][cstage].resize(1<<cstage);
			memcpy(newLLR[path][cstage].data(), LLR[selCand.srcPath][cstage].data(), 4<<cstage);
		}
		memcpy(newBits[path].data(), Bits[selCand.srcPath].data(), 4<<n);
		
		//Compose the new path
		Metric[path] = selCand.newMetric;
		float value = (selCand.decisionIndex?-0.0:0.0);
		for(int bit=0; bit<size; ++bit)
		{
			newBits[path][BitLocation+bit] = value;
		}
	}
/*	Bits.clear();
	LLR.clear();
	Bits = std::move(newBits);
	LLR = std::move(newLLR);*/
	std::swap(Bits, newBits);
	std::swap(LLR, newLLR);
	PathCount = newPathCount;
}

bool PolarCode::decodeMultiPath(vector<bool> &decoded, vector<float> &initialLLR)
{
	resetMemory();
	
	//Initialize LLR-structure for first path
	std::copy(initialLLR.begin(), initialLLR.end(), LLR[0][n].begin());

//	decodeMultiPathRecursive(n, 0, 0);

#include "SpecialMultiPathDecoder.cpp"

	vector<vector<bool>> Dhat(PathCount, vector<bool>(N));
	
	for(int i=0; i<PathCount; ++i)
	{
		transform(Bits[i], Dhat[i]);
	}
	

	//Select the most likely path which passes the CRC test
	bool success = false;
	
	for(int path=1; path<PathCount; ++path)
	{
		decoded.assign(K, 0);
		for(int bit=0; bit<K; ++bit)
		{
			decoded[bit] = Dhat[path][AcceleratedLookup[bit]];
		}
		if(Crc->check(decoded))
		{
			success = true;
			break;
		}
	}

	if(!success)//Give out the most likely path, if no crc is fulfilled
	{
		auto bitptrA = decoded.begin();
		auto bitptrB = Dhat[0].begin();
		for(int bit=0; bit<N; ++bit)
		{
			if(FZLookup[bit])
			{
				*bitptrA = *bitptrB;
				++bitptrA;
			}
			++bitptrB;
		}
	}
	
	return success;
}

void PolarCode::decodeMultiPathRecursive(int stage, int BitLocation, int nodeID)
{
	int leftNode  = (nodeID<<1)+1;
	int rightNode = leftNode+1;
	int subStageLength = 1<<(stage-1);

	//Calculate LLRs for all paths
	for(int currentPath=0; currentPath<PathCount; ++currentPath)
	{
		F_function_hybrid(LLR[currentPath][stage].data(), LLR[currentPath][stage-1].data(), subStageLength);
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
		
		/* Implementing the RepSPC-shortcut for multipath-decoding is a lot of work,
		 * which I didn't want to do.
		 */
		 
	default:
		decodeMultiPathRecursive(stage-1, BitLocation, leftNode);
	}
	

	
	/* Now, for all the paths, generate the bit decision-dependant
	   subpaths and proceed as above */
	if(simplifiedTree[leftNode] != RateZero)
	{
		for(int currentPath=0; currentPath<PathCount; ++currentPath)
		{
			G_function_hybrid(LLR[currentPath][stage].data(), LLR[currentPath][stage-1].data(), Bits[currentPath].data()+BitLocation, subStageLength);
		}
	}
	else
	{
		for(int currentPath=0; currentPath<PathCount; ++currentPath)
		{
			G_function_0R_hybrid(LLR[currentPath][stage].data(), LLR[currentPath][stage-1].data(), subStageLength);
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

