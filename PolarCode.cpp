#include <cmath>
#include <cstring>
#include <algorithm>
#include <utility>

#include "PolarCode.h"
#include "ArrayFuncs.h"
#include "crc8.h"

using namespace std;

inline float logdomain_sum(float x, float y)
{
	if(x<y)
	{
		return y+log1p(exp(x-y));
	}
	else
	{
		return x+log1p(exp(y-x));
	}
}

inline float logdomain_diff(float x, float y)
{
	return x+log1p(-exp(y-x));
}

template <typename T> int sgn(T val) {
    return (T(0) < val) - (val < T(0));
}

void PolarCode::F_function(float *LLRin, float *LLRout, int size)
{
	unsigned int *iLLRin = reinterpret_cast<unsigned int*>(LLRin);
	unsigned int *iLLRout = reinterpret_cast<unsigned int*>(LLRout);
	unsigned int a,b;
	float *fa = reinterpret_cast<float*>(&a);
	float *fb = reinterpret_cast<float*>(&b);
	float tmp; unsigned int* iTmp = reinterpret_cast<unsigned int*>(&tmp);
	for(int i=0; i<size; ++i)
	{
		a = iLLRin[i];
		b = iLLRin[i+size];
		//LLRout[i] = sgn(a) * sgn(b) * fmin(fabs(a),fabs(b));
		iLLRout[i] = (a ^ b) & 0x80000000;
		a &= 0x7FFFFFFF;
		b &= 0x7FFFFFFF;
		tmp = fmin(*fa, *fb);
		iLLRout[i] ^= *iTmp;
	}
}

void PolarCode::F_function_vectorized(float *LLRin, float *LLRout, int size)
{
	for(int i=0; i<size; i+=FLOATSPERVECTOR)
	{
		vec LLR_l = load_ps(LLRin+i);
		vec LLR_r = load_ps(LLRin+i+size);
		vec sign  = and_ps(xor_ps(LLR_l, LLR_r), SIGN_MASK);
		vec abs_l = andnot_ps(SIGN_MASK, LLR_l);
		vec abs_r = andnot_ps(SIGN_MASK, LLR_r);
		vec LLR_o = or_ps(sign, min_ps(abs_l, abs_r));
		store_ps(LLRout+i, LLR_o);
	}
}

void PolarCode::F_function_hybrid(float *LLRin, float *LLRout, int size)
{
	if(size < FLOATSPERVECTOR)
		F_function(LLRin, LLRout, size);
	else
		F_function_vectorized(LLRin, LLRout, size);
}

void PolarCode::G_function(float *LLRin, float *LLRout, float *Bits, int size)
{
	unsigned int *FloatBit = reinterpret_cast<unsigned int*>(Bits);
	float tmp;unsigned int *iTmp = reinterpret_cast<unsigned int*>(&tmp);
	for(int i=0; i<size; ++i)
	{
		tmp = LLRin[i];
		*iTmp ^= FloatBit[i];
		LLRout[i] = LLRin[i+size] + tmp;
	}
}

void PolarCode::G_function_vectorized(float *LLRin, float *LLRout, float *Bits, int size)
{
	for(int i=0; i<size; i+=FLOATSPERVECTOR)
	{
		vec LLR_l  = load_ps(LLRin+i);
		vec LLR_r  = load_ps(LLRin+i+size);
		vec Bits_l = load_ps(Bits+i);
		vec LLR_n  = xor_ps(Bits_l, LLR_l);
		vec LLR_o = add_ps(LLR_r, LLR_n);
		store_ps(LLRout+i, LLR_o);
	}
}

void PolarCode::G_function_hybrid(float *LLRin, float *LLRout, float *Bits, int size)
{
	if(size < FLOATSPERVECTOR)
		G_function(LLRin, LLRout, Bits, size);
	else
		G_function_vectorized(LLRin, LLRout, Bits, size);
}

void PolarCode::G_function_0R(float *LLRin, float *LLRout, int size)
{
	for(int i=0; i<size; ++i)
	{
		LLRout[i] = LLRin[i] + LLRin[i+size];
	}
}

void PolarCode::G_function_0R_vectorized(float *LLRin, float *LLRout, int size)
{
	for(int i=0; i<size; i+=FLOATSPERVECTOR)
	{
		vec LLR_l  = load_ps(LLRin+i);
		vec LLR_r  = load_ps(LLRin+i+size);
		vec LLR_o = add_ps(LLR_l, LLR_r);
		store_ps(LLRout+i, LLR_o);
	}
}

void PolarCode::G_function_0R_hybrid(float *LLRin, float *LLRout, int size)
{
	if(size < FLOATSPERVECTOR)
		G_function_0R(LLRin, LLRout, size);
	else
		G_function_0R_vectorized(LLRin, LLRout, size);
}

void PolarCode::Rate0(float *BitsOut, int size)
{
	memset(BitsOut, 0, size*sizeof(float));
}

void PolarCode::Rate1(float *LLRin, float *BitsOut, int size)
{
	unsigned int *iLLR = reinterpret_cast<unsigned int*>(LLRin);
	unsigned int *iBit = reinterpret_cast<unsigned int*>(BitsOut);

	for(int i=0; i<size; ++i)
	{
		iBit[i] = iLLR[i]&0x80000000;
	}
}

void PolarCode::Rate1_vectorized(float *LLRin, float *BitsOut, int size)
{
	for(int i=0; i<size; i+=FLOATSPERVECTOR)
	{
		vec LLR_i = load_ps(LLRin+i);
		LLR_i = and_ps(LLR_i, SIGN_MASK);
		store_ps(BitsOut+i, LLR_i);
	}
}

void PolarCode::P_R1(float *LLRin, float *BitsOut, int size)
{
	unsigned int *iBit = reinterpret_cast<unsigned int*>(BitsOut);

	float tmp;unsigned int *iTmp = reinterpret_cast<unsigned int*>(&tmp);
	for(int i=0; i<size; ++i)
	{
		tmp = LLRin[i];
		*iTmp ^= iBit[i];
		tmp = LLRin[i+size] + tmp;//G-function
		*iTmp &= 0x80000000;//Rate 1 decoder
		BitsOut[i+size] = tmp;//Right bit
		iBit[i] ^= *iTmp;//Left bit
	}
}

void PolarCode::P_R1_vectorized(float *LLRin, float *BitsOut, int size)
{
	for(int i=0; i<size; i+=FLOATSPERVECTOR)
	{
		vec LLR_l = load_ps(LLRin+i);
		vec LLR_r = load_ps(LLRin+i+size);
		vec Bits_l = load_ps(BitsOut+i);

		vec LLR_o = add_ps(xor_ps(LLR_l, Bits_l), LLR_r);//G-function
		LLR_o = and_ps(LLR_o, SIGN_MASK);//Rate 1 decoder
		store_ps(BitsOut+i+size, LLR_o);//Right bit
		LLR_o = xor_ps(LLR_o, Bits_l);
		store_ps(BitsOut+i, LLR_o);//Left bit
	}
}


void PolarCode::P_01(float *LLRin, float *BitsOut, int size)
{
	float tmp;unsigned int *iTmp = reinterpret_cast<unsigned int*>(&tmp);
	for(int i=0; i<size; ++i)
	{
		tmp = LLRin[i+size] + LLRin[i];//G-function
		*iTmp &= 0x80000000;//Rate 1 decoder
		BitsOut[i] = tmp;//Left bit
		BitsOut[i+size] = tmp;//Right bit
	}
}


void PolarCode::P_RSPC(float *LLRin, float *BitsOut, int size)
{
	unsigned int *iBit = reinterpret_cast<unsigned int*>(BitsOut);

	unsigned int parity=0;float *fPar = reinterpret_cast<float*>(&parity);
	int index=0;

	vec parityVec = set1_ps(0.0);

	for(int i=0; i<size; i+=FLOATSPERVECTOR)
	{
		//Load data
		vec LLR_l = load_ps(LLRin+i);
		vec LLR_r = load_ps(LLRin+i+size);
		vec Bit_l = load_ps(BitsOut+i);

		vec LLR1 = add_ps(xor_ps(LLR_l, Bit_l), LLR_r);//G-function
		vec Bit_o = and_ps(LLR1, SIGN_MASK);//Rate-1 decoder
		store_ps(BitsOut+i+size, Bit_o);//Save bit decision
		parityVec = xor_ps(Bit_o, parityVec);//Calculate parity

		Bit_l = xor_ps(Bit_l, Bit_o);
		store_ps(BitsOut+i, Bit_l);//Save upper bit

		Bit_o = and_ps(LLR1, ABS_MASK);
		store_ps(absLLR.data()+i, Bit_o);
	}

	*fPar = reduce_xor_ps(parityVec);

	if(parity)
	{
		for(int i=1; i<size; ++i)
		{
			if(absLLR[i] < absLLR[index])
			{
				index = i;
			}
		}

		//Flip least reliable bit
		iBit[index] ^= parity;
		iBit[index+size] ^= parity;
	}
}

void PolarCode::P_0SPC(float *LLRin, float *BitsOut, int size)
{
	unsigned int *iBit = reinterpret_cast<unsigned int*>(BitsOut);

	unsigned int parity=0;
	int index=0;
	float tmp; unsigned int *iTmp = reinterpret_cast<unsigned int*>(&tmp);

	unsigned int *iAbsVec = reinterpret_cast<unsigned int*>(absLLR.data());

	for(int i=0; i<size; i++)
	{
		tmp = LLRin[i]+LLRin[i+size];
		parity ^= (iBit[i] = iBit[i+size] = *iTmp&0x80000000);
		iAbsVec[i] = *iTmp&0x7FFFFFFF;
	}

	if(parity)
	{
		for(int i=1; i<size; ++i)
		{
			if(absLLR[i] < absLLR[index])
			{
				index = i;
			}
		}

		//Flip least reliable bit
		iBit[index] = parity;
		iBit[index+size] ^= parity;
	}
}

void PolarCode::P_0SPC_vectorized(float *LLRin, float *BitsOut, int size)
{
	unsigned int *iBit = reinterpret_cast<unsigned int*>(BitsOut);

	unsigned int parity=0;float *fPar = reinterpret_cast<float*>(&parity);
	int index=0;

	vec parityVec = set1_ps(0.0);

	for(int i=0; i<size; i+=FLOATSPERVECTOR)
	{
		//Load data
		vec LLR_l = load_ps(LLRin+i);
		vec LLR_r = load_ps(LLRin+i+size);

		vec LLR1 = add_ps(LLR_l, LLR_r);//G-function
		vec Bit_o = and_ps(LLR1, SIGN_MASK);//Rate-1 decoder
		store_ps(BitsOut+i+size, Bit_o);//Save bit decision
		parityVec = xor_ps(Bit_o, parityVec);//Calculate parity

		store_ps(BitsOut+i, Bit_o);//Save upper bit

		Bit_o = and_ps(LLR1, ABS_MASK);
		store_ps(absLLR.data()+i, Bit_o);
	}

	*fPar = reduce_xor_ps(parityVec);

	if(parity)
	{
		for(int i=1; i<size; ++i)
		{
			if(absLLR[i] < absLLR[index])
			{
				index = i;
			}
		}

		//Flip least reliable bit
		iBit[index] = parity;
		iBit[index+size] ^= parity;
	}
}




void PolarCode::CombineSimple(float *Bits, int size)
{
	unsigned int *inlptr = reinterpret_cast<unsigned int*>(Bits),
	             *inrptr = reinterpret_cast<unsigned int*>(Bits+size);
	for(int i=0; i<size; i++)
	{
		inlptr[i] ^= inrptr[i];
	}
}

void PolarCode::CombineSimple_vectorized(float *Bits, int size)
{
	for(int i=0; i<size; i+=FLOATSPERVECTOR)
	{
		vec Bitsl = load_ps(Bits+i);
		vec Bitsr = load_ps(Bits+i+size);
		vec Bitso = xor_ps(Bitsl, Bitsr);
		store_ps(Bits+i, Bitso);
	}
}

void PolarCode::Combine_0RSimple(float *Bits, int size)
{
	memcpy(Bits, Bits+size, size<<2);
}


void PolarCode::SPC(float *LLRin, float *BitsOut, int size)
{
	unsigned int *iLLR = reinterpret_cast<unsigned int*>(LLRin);
	unsigned int *iBit = reinterpret_cast<unsigned int*>(BitsOut);

	unsigned int parity = 0;
	int index = 0;
	float minLLR; unsigned int *iMinLLR = reinterpret_cast<unsigned int*>(&minLLR);
	float testLLR;unsigned int *iTestLLR = reinterpret_cast<unsigned int*>(&testLLR);
	*iMinLLR = iLLR[0]&0x7FFFFFFF;
	for(int i=0; i<size; ++i)
	{
		iBit[i]   = iLLR[i]&0x80000000;
		parity ^= iBit[i];

		*iTestLLR = iLLR[i]&0x7FFFFFFF;
		if(testLLR < minLLR)
		{
			index = i;
			minLLR = testLLR;
		}
	}

	//Flip least reliable bit, if neccessary
	iBit[index] ^= parity;
}

void PolarCode::SPC_4(float *LLRin, float *BitsOut)
{
	__m128 LLRi = _mm_load_ps(LLRin);
	__m128 Sgn = _mm_and_ps(LLRi, sgnMask);

	_mm_store_ps(BitsOut, Sgn);

	__m128 Par =  _mm_xor_ps(Sgn, _mm_movehl_ps(Sgn, Sgn));
    Par = _mm_xor_ps(Par, _mm_shuffle_ps(Par, Par, 0x55));
    //float parity = _mm_cvtss_f32(Par);
    unsigned int *iPar = reinterpret_cast<unsigned int*>(&Par);
    if(*iPar)
	{
		__m128 Abs = _mm_and_ps(LLRi, absMask);
		int index = (Abs[1]<Abs[0]);
		if(Abs[2]<Abs[index])index=2;
		if(Abs[3]<Abs[index])index=3;
		unsigned int *iBit = reinterpret_cast<unsigned int*>(BitsOut);
		iBit[index] ^= 0x80000000;
	}
}

void PolarCode::RepSPC(float *LLRin, float *BitsOut, int size)
{
	unsigned int subSize = size>>1;

	unsigned int *SPC0 = reinterpret_cast<unsigned int*>(BitsOut);
	unsigned int *SPC1 = reinterpret_cast<unsigned int*>(BitsOut+subSize);
	unsigned int *decidedSPC;
	float RepSum = 0.0, RepAcc;

	float a, absA, minA = INFINITY,
	      b, absB, minB = INFINITY;
	unsigned int indA = 0, parA = 0,
	             indB = 0, parB = 0;

	unsigned int *ia = reinterpret_cast<unsigned int*>(&a);
	unsigned int *ib = reinterpret_cast<unsigned int*>(&b);
	unsigned int *iabsA = reinterpret_cast<unsigned int*>(&absA);
	unsigned int *iabsB = reinterpret_cast<unsigned int*>(&absB);
	unsigned int *iRepSum = reinterpret_cast<unsigned int*>(&RepSum);
	unsigned int *iRepAcc = reinterpret_cast<unsigned int*>(&RepAcc);

	for(unsigned int i=0; i<subSize; ++i)
	{
		//Prepare the LLRs for i-th bit
		a = LLRin[i];
		b = LLRin[i+subSize];
		*iabsA = *ia&0x7FFFFFFF;
		*iabsB = *ib&0x7FFFFFFF;

		//Accumulate the value for Repetition code
		//RepSum += sgn(a) * sgn(b) * fmin(absA,absB);
		RepAcc = fmin(absA, absB);
		*iRepAcc ^= (*ia ^ *ib) & 0x80000000;
		RepSum += RepAcc;

		//Decide both possibilities for SPC code
		a = LLRin[i+subSize] + LLRin[i];
		b = LLRin[i+subSize] - LLRin[i];
		SPC0[i] = *ia & 0x80000000;
		SPC1[i] = *ib & 0x80000000;
		//and update the parity check
		parA ^= SPC0[i];
		parB ^= SPC1[i];

		*iabsA = *ia&0x7FFFFFFF;
		*iabsB = *ib&0x7FFFFFFF;

		//Find least reliable bit for SPC code
		if(absA < minA)
		{
			minA = absA;
			indA = i;
		}
		if(absB < minB)
		{
			minB = absB;
			indB = i;
		}
	}

	//Hard decision for repetition code
	*iRepSum &= 0x80000000;

	//Decide, which SPC-bits and parity bit to use
	decidedSPC = *iRepSum ? SPC1 : SPC0;
	unsigned int parity = *iRepSum ? parB : parA;
	unsigned int index = *iRepSum ? indB : indA;


	//Flip the the least reliable bit
	decidedSPC[index] ^= parity;

	//Save the decisions
	if(*iRepSum)
	{
		//Copy the inverted bits up, if Repetition bit was 1
		if(subSize < FLOATSPERVECTOR)
		{
			for(unsigned int i=0; i<subSize; ++i)
			{
				SPC0[i] = SPC1[i] ^ 0x80000000;
			}
		}
		else
		{
			for(unsigned int i=0; i<subSize; i+=FLOATSPERVECTOR)
			{
				vec Bits = load_ps(BitsOut+subSize+i);
				Bits = xor_ps(Bits, SIGN_MASK);
				store_ps(BitsOut+i, Bits);
			}
		}
	}
	else
	{
		//Copy the bits down, if Repetition bit was 0
		memcpy(SPC1, SPC0, subSize<<2);
	}
}



void PolarCode::RepSPC_8(float *LLRin, float *BitsOut)
{
	unsigned int *iBits = reinterpret_cast<unsigned int*>(BitsOut);
	float Parity;unsigned int *iParity = reinterpret_cast<unsigned int*>(&Parity);

	__m128 LLR_l = _mm_load_ps(LLRin);
	__m128 LLR_r = _mm_load_ps(LLRin+4);
	__m128 sign = _mm_and_ps(_mm_xor_ps(LLR_l,LLR_r), sgnMask);
	__m128 abs_l = _mm_and_ps(LLR_l, absMask);
	__m128 abs_r = _mm_and_ps(LLR_r, absMask);
	__m128 RepSumVec = _mm_or_ps(sign, _mm_min_ps(abs_l, abs_r));

	__m128 x64 = _mm_add_ps(RepSumVec, _mm_movehl_ps(RepSumVec, RepSumVec));
    __m128 x32 = _mm_add_ss(x64, _mm_shuffle_ps(x64, x64, 0x55));
    x32 = _mm_and_ps(x32, sgnMask);//Decide Repetition code
	//RepSumVec = _mm_broadcastss_ps(x32);
	RepSumVec = _mm_set1_ps(_mm_cvtss_f32(x32));


	__m128 SPCVec = _mm_add_ps(_mm_xor_ps(LLR_l, RepSumVec), LLR_r);//Load SPC-LLRs
	__m128 SPCsgn = _mm_and_ps(SPCVec, sgnMask);//Hard decision
	RepSumVec = _mm_xor_ps(RepSumVec, SPCsgn);//Combine
	_mm_store_ps(BitsOut, RepSumVec);//Store Repetition bits
	_mm_store_ps(BitsOut+4, SPCsgn);//Store SPC bits

	x64 = _mm_xor_ps(SPCsgn, _mm_movehl_ps(SPCsgn, SPCsgn));
    x32 = _mm_xor_ps(x64, _mm_shuffle_ps(x64, x64, 0x55));
    Parity = _mm_cvtss_f32(x32);//Calculate parity

	if(*iParity)
	{
		//Find least reliable bit
		int index=0;
		__m128 SPCabs = _mm_and_ps(SPCVec, absMask);
		for(int i=1;i<4;++i)
		{
			if(SPCabs[i] < SPCabs[index])
			{
				index=i;
			}
		}
		iBits[index]   ^= 0x80000000;
		iBits[index+4] ^= 0x80000000;
	}
}



void PolarCode::Repetition(float *LLRin, float *BitsOut, int size)
{
	float sum = 0.0;
	for(int i=0; i<size; ++i)
	{
		sum += LLRin[i];
	}
	float bit = sum<0?-0.0:0.0;
	for(int i=0; i<size; ++i)
	{
		BitsOut[i] = bit;
	}
}

void PolarCode::Repetition_vectorized_4(float *LLRin, float *BitsOut)
{
	float Sum=0;

	__m128 LLRsum = _mm_set1_ps(0.0);
	LLRsum = _mm_load_ps(LLRin);
	Sum = _mm_reduce_add_ps(LLRsum);
	__m128 BitLLR = _mm_set1_ps(Sum);
	__m128 BitDecision = _mm_and_ps(BitLLR, sgnMask);
	_mm_store_ps(BitsOut, BitDecision);
}

void PolarCode::Repetition_vectorized(float *LLRin, float *BitsOut, int size)
{
	float Sum=0;

	vec LLRsum = set1_ps(0.0);
	vec TmpVec;
	for(int i=0; i<size; i+=FLOATSPERVECTOR)
	{
		TmpVec = load_ps(LLRin+i);
		LLRsum = add_ps(LLRsum, TmpVec);
	}
	Sum = reduce_add_ps(LLRsum);
	vec BitLLR = set1_ps(Sum);
	vec BitDecision = and_ps(BitLLR, SIGN_MASK);
	for(int i=0; i<size; i+=FLOATSPERVECTOR)
	{
		store_ps(BitsOut+i, BitDecision);
	}
}

void PolarCode::Repetition_hybrid(float *LLRin, float *BitsOut, int size)
{
	if(size < FLOATSPERVECTOR)
		Repetition(LLRin, BitsOut, size);
	else
		Repetition_vectorized(LLRin, BitsOut, size);
}



PolarCode::PolarCode(int N, int K, int L, bool useCRC, float designSNR, bool encodeOnly)
{
	this->N = N;
	this->K = K;
	this->L = L;
	this->useCRC = useCRC;
	this->designSNR = designSNR;
	n = ceil(log2(N));

	FZLookup.resize(N);
	simplifiedTree.resize(2*N-1);

	Crc = useCRC ? new CRC8() : nullptr;
	
	unsigned __int64 randomseed[4];
	
	_rdseed64_step(randomseed+0);
	_rdseed64_step(randomseed+1);
	_rdseed64_step(randomseed+2);
	_rdseed64_step(randomseed+3);
	
	r.seed(_mm256_set_epi64x(randomseed[0], randomseed[1], randomseed[2], randomseed[3]));

	if(!encodeOnly)
	{
		AlignedVector = (float*)_mm_malloc(FLOATSPERVECTOR * sizeof(float), sizeof(vec));
		SIGN_MASK = set1_ps(-0.0);
		ZERO = set1_ps(0.0);

		{
			float absmask; unsigned int *absptr=reinterpret_cast<unsigned int*>(&absmask);
			*absptr = 0x7FFFFFFF;
			ABS_MASK = set1_ps(absmask);
		}

		sgnMask = _mm256_extractf128_ps(SIGN_MASK, 0);
		absMask = _mm256_extractf128_ps(ABS_MASK, 0);

		maxCandCount = L<<3;
		Metric.resize(L);
		newMetrics.resize(maxCandCount);
		cand.resize(maxCandCount);
		SimpleBits.resize(N);



		absLLR.resize(N);

		initialLLR.resize(N);
		LLR.resize(L);
		Bits.resize(L);
		newLLR.resize(L);
		newBits.resize(L);

		for(int path=0; path<L; ++path)
		{
			LLR[path].resize(n);
			newLLR[path].resize(n);
			Bits[path].resize(N);
			newBits[path].resize(N);
			for(int stage=0; stage<n; ++stage)
			{
				LLR[path][stage].resize(std::max(FLOATSPERVECTOR, 1<<stage));
				newLLR[path][stage].resize(std::max(FLOATSPERVECTOR, 1<<stage));
			}
		}
	}
	else
	{
		AlignedVector = nullptr;
	}

	pcc();

}

PolarCode::~PolarCode()
{
	if(AlignedVector != nullptr)
	{
		_mm_free(AlignedVector);
	}
	if(Crc != nullptr)
	{
		delete Crc;
	}
}

void PolarCode::clear()
{
	Metric.clear();
	newMetrics.clear();
	cand.clear();
	SimpleBits.clear();
	absLLR.clear();
	initialLLR.clear();
	LLR.clear();
	Bits.clear();
	newLLR.clear();
	newBits.clear();
	FZLookup.clear();
	AcceleratedLookup.clear();
	simplifiedTree.clear();
}

unsigned int PolarCode::bitreversed_slow(unsigned int j)
{
	//Do a complete 32-bit-reversal
    j = (((j & 0xaaaaaaaa) >>  1) | ((j & 0x55555555) <<  1));
    j = (((j & 0xcccccccc) >>  2) | ((j & 0x33333333) <<  2));
    j = (((j & 0xf0f0f0f0) >>  4) | ((j & 0x0f0f0f0f) <<  4));
    j = (((j & 0xff00ff00) >>  8) | ((j & 0x00ff00ff) <<  8));
    j = (( j               >> 16) | ( j               << 16));
    return j>>(32-n);//return the reversed n-bit value
}

void PolarCode::pcc()
{
	vector<float> z(N, 0.0);
	float designSNRlin = pow(10.0, designSNR/10.0);
	z[0] = exp(-((float)K/N)*designSNRlin);

	float T; int B;

	for(int lev=n-1; lev >= 0; --lev)
	{
		B = 1<<lev;//pow(2, lev);
		for(int j = 0; j < N; j+=(B<<1))
		{
			T = z[j];
			z[j+B] = T*T;
			z[j] = 2*T - z[j+B];
		}
	}

	sorter.set(z);
	sorter.stableSortDescending();

	for(int i = 0; i<K; ++i)
	{
		FZLookup[sorter.permuted[N-i-1]] = true;//Bit is available for user data
		simplifiedTree[N-1+sorter.permuted[N-i-1]] = nodeInfo::RateOne;
	}
	for(int i = K; i<N; ++i)
	{
		FZLookup[sorter.permuted[N-i-1]] = false;//Freeze bit
		simplifiedTree[N-1+sorter.permuted[N-i-1]] = nodeInfo::RateZero;
	}

	for(int i=0; i<N; ++i)
	{
		if(FZLookup[i])
		{
			AcceleratedLookup.push_back(i);
		}
		else
		{
			AcceleratedFrozenLookup.push_back(i);
		}
	}

	for(int lev=n-1; lev>=0; --lev)
	{
		int st = (1<<lev)-1;
		int ed = (1<<(lev+1))-1;
		int idx, ctr;
		for(idx=st, ctr=ed; idx<ed; ++idx)
		{
#ifndef ONLY_SCDECODING
			nodeInfo Left  = simplifiedTree[ctr++];
			nodeInfo Right = simplifiedTree[ctr++];
			if(Left == RateZero && Right == RateZero)
			{
				simplifiedTree[idx] = RateZero;
			}
			else if(Left == RateOne && Right == RateOne)
			{
				simplifiedTree[idx] = RateOne;
			}
			else if((Left == RateHalf || Left == SPCnode) && Right == RateOne && lev>=n-2)
			{
				simplifiedTree[idx] = SPCnode;
			}
			else if(Left == RateZero && (Right == RateHalf || Right == RepetitionNode))
			{
				simplifiedTree[idx] = RepetitionNode;
			}
			else if(Left == RateZero && Right == RateOne && lev==n-1)
			{
				simplifiedTree[idx] = RateHalf;
			}
			else if(Left == RepetitionNode && Right == SPCnode)
			{
				simplifiedTree[idx] = RepSPCnode;
			}
			else
			{
#endif
				simplifiedTree[idx] = RateR;
#ifndef ONLY_SCDECODING
			}
#endif
		}
	}
}

void PolarCode::encode(aligned_float_vector &encoded, unsigned char* data)
{
	encoded.assign(N, 0.0);

	//Calculate CRC
	if(useCRC)
	{
		int lastByte = (K>>3)-1;
		Crc->generate(data, lastByte, data+lastByte);
	}

	//Insert the bits into Rate-1 channels as float bits (sign bit: 0 => 0.0, 1 => -0.0)
	int bit=0; int bytes=K>>3;
	unsigned int bitpool, currentBit; float* fBit=reinterpret_cast<float*>(&currentBit);
	for(int byte=0; byte<bytes; ++byte)
	{
		bitpool = data[byte];
		bitpool <<= 24;
		for(int tbit=0; tbit<8; ++tbit)
		{
			currentBit = bitpool&0x80000000;
			encoded[AcceleratedLookup[bit++]] = *fBit;
			bitpool <<= 1;
		}
	}

	//Encode

#ifndef SYSTEMATIC_CODING
	transform(encoded);
#else

#ifdef FLEXIBLE_DECODING
	subEncodeSystematic(encoded, n, 0, 0);
#else
#include "SpecialSystematicEncoder.cpp"
#endif
#endif
}

void PolarCode::subEncodeSystematic(aligned_float_vector &encoded, int stage, int BitLocation, int nodeID)
{
	int leftNode  = (nodeID<<1)+1;
	int rightNode = leftNode+1;
	int subStageLength = 1<<(stage-1);

	unsigned int* iData = reinterpret_cast<unsigned int*>(encoded.data());
	float *fData = encoded.data();

	//Calculate right node
	if(simplifiedTree[rightNode] != RateOne)
		subEncodeSystematic(encoded, stage-1, BitLocation+subStageLength, rightNode);

	//Calculate left node
	if(simplifiedTree[leftNode] != RateZero)
	{
		//XOR left and right to left, to get the left child node bits
		if(subStageLength<FLOATSPERVECTOR)
		{
			for(int i=0; i<subStageLength; ++i)
			{
				iData[BitLocation+i] ^= iData[BitLocation+subStageLength+i];
			}
		}
		else
		{
			for(int i=0; i<subStageLength; i+=FLOATSPERVECTOR)
			{
				vec a = load_ps(fData+BitLocation+i);
				vec b = load_ps(fData+BitLocation+subStageLength+i);
				vec c = xor_ps(a, b);
				store_ps(fData+BitLocation+i, c);
			}
		}
		//Calculate left node
		if(simplifiedTree[leftNode] != RateOne)
			subEncodeSystematic(encoded, stage-1, BitLocation, leftNode);
		//XOR left and right to left part of parent node
		if(subStageLength<FLOATSPERVECTOR)
		{
			for(int i=0; i<subStageLength; ++i)
			{
				iData[BitLocation+i] ^= iData[BitLocation+subStageLength+i];
			}
		}
		else
		{
			for(int i=0; i<subStageLength; i+=FLOATSPERVECTOR)
			{
				vec a = load_ps(fData+BitLocation+i);
				vec b = load_ps(fData+BitLocation+subStageLength+i);
				vec c = xor_ps(a, b);
				store_ps(fData+BitLocation+i, c);
			}
		}
	}
	else
	{
		//copy right to left
/*		for(int i=0; i<subStageLength; ++i)
		{
			iData[BitLocation+i] = iData[BitLocation+subStageLength+i];
		}*/
		memcpy(iData+BitLocation, iData+BitLocation+subStageLength, subStageLength<<2);
	}
}


void PolarCode::transform(aligned_float_vector &Bits)
{
	float *BitPtr = Bits.data();
	unsigned int *iBit = reinterpret_cast<unsigned int *>(BitPtr);

	int B, nB, base, inc;
	for(int i=n-1; i>=0; --i)
	{
		B = 1<<(n-i-1);
		nB = 1<<i;
		inc = B<<1;
		if(B>=FLOATSPERVECTOR)
		{
			base = 0;
			for(int j=0; j<nB; ++j)
			{
				for(int l=0; l<B; l+=FLOATSPERVECTOR)
				{
					vec Bit_l = load_ps(BitPtr+base+l);
					vec Bit_r = load_ps(BitPtr+base+l+B);
					Bit_l = xor_ps(Bit_l, Bit_r);
					store_ps(BitPtr+base+l, Bit_l);
				}
				base += inc;
			}
		}
		else
		{
			base = 0;
			for(int j=0; j<nB; ++j)
			{
				for(int l=0; l<B; ++l)
				{
					iBit[base+l] ^= iBit[base+l+B];
				}
				base += inc;
			}
		}
	}
}

void PolarCode::modulateAndDistort(float *signal, aligned_float_vector &data, int size, float factor)
{
	vec facVec = set1_ps(factor);
	for(int i=0; i<size; i+=16)
	{
		vec siga = load_ps(data.data()+i);
		vec sigb = load_ps(data.data()+i+8);
		siga = or_ps(siga, one); sigb = or_ps(sigb, one);//Modulate
		siga = mul_ps(siga, facVec); sigb = mul_ps(sigb, facVec);//Scale

		//Generate Gaussian noise
        __m256 u1 = _mm256_sub_ps(one, r()); // [0, 1) -> (0, 1]
        __m256 u2 = r();
		__m256 radius = _mm256_sqrt_ps(_mm256_mul_ps(minustwo, log256_ps(u1)));
		__m256 theta = _mm256_mul_ps(twopi, u2);
        __m256 sintheta, costheta;
        sincos256_ps(theta, &sintheta, &costheta);

		//Add noise to signal
#ifdef __FMA__
		siga = _mm256_fmadd_ps(radius, costheta, siga);
		sigb = _mm256_fmadd_ps(radius, sintheta, sigb);
#else
		siga = add_ps(mul_ps(radius, costheta), siga);
		sigb = add_ps(mul_ps(radius, sintheta), sigb);
#endif

		//Demodulate
		siga = mul_ps(siga, facVec);
		sigb = mul_ps(sigb, facVec);

		//Save
		store_ps(signal+i, siga);
		store_ps(signal+i+8, sigb);
	}
}

bool PolarCode::decode(unsigned char* decoded, float* initLLR)
{
	memcpy(initialLLR.data(), initLLR, N<<2);

	if(useCRC)
	{
		if(decodeOnePath(decoded))
		{
			return true;
		}
		else if(L > 1)
		{
			return decodeMultiPath(decoded);
		}
		else
		{
			/* For a list size of one, there is no need to try again.
			   Every path pruning would decide for the ML path.
			*/
			return false;
		}
	} else {
		if(L == 1)
		{
			return decodeOnePath(decoded);
		}
		else
		{
			return decodeMultiPath(decoded);
		}
	}
}

bool PolarCode::decodeOnePath(unsigned char* decoded)
{

#ifdef FLEXIBLE_DECODING
	decodeOnePathRecursive(n,SimpleBits.data(),0);
#else
	float *BitPtr = SimpleBits.data();
#include "SpecialDecoder.cpp"
#endif


#ifndef SYSTEMATIC_CODING
	transform(SimpleBits);
#endif


	int bytes = K>>3;
	int bit = 0;
	unsigned int *iBit = reinterpret_cast<unsigned int*>(SimpleBits.data());
	for(int byte = 0; byte<bytes; ++byte)
	{
		unsigned char thisByte = 0;
		for(int b=0;b<8;++b)
		{
			thisByte |= (iBit[AcceleratedLookup[bit++]]>>(24+b));
		}
		decoded[byte] = thisByte;
	}

	if(useCRC)
		return Crc->check(decoded, bytes-1, decoded[bytes-1]);
	else
		return true;
}

void PolarCode::decodeOnePathRecursive(int stage, float *nodeBits, int nodeID)
{
	int leftNode  = (nodeID<<1)+1;
	int rightNode = leftNode+1;
	int subStageLength = 1<<(stage-1);
	float *rightBits = nodeBits+subStageLength;
	float *LLRptr = stage==n ? initialLLR.data() : LLR[0][stage].data();

	if(simplifiedTree[leftNode] != RateZero)
	{
		F_function_hybrid(LLRptr, LLR[0][stage-1].data(), subStageLength);
	}

	switch(simplifiedTree[leftNode])
	{
	case RateZero:
//		Rate0(Bits[0][stage-1][0].data(), subStageLength);
		break;
	case RateOne:
		Rate1(LLR[0][stage-1].data(), nodeBits, subStageLength);
		break;
	case RepetitionNode:
	case RateHalf:
		Repetition_hybrid(LLR[0][stage-1].data(), nodeBits, subStageLength);
		break;
	case SPCnode:
		if(subStageLength == 4)
		{
			SPC_4(LLR[0][stage-1].data(), nodeBits);
		}
		else
		{
			SPC(LLR[0][stage-1].data(), nodeBits, subStageLength);
		}
		break;
	case RepSPCnode:
		if(subStageLength == 8)
		{
			RepSPC_8(LLR[0][stage-1].data(), nodeBits);
		}
		else
		{
			RepSPC(LLR[0][stage-1].data(), nodeBits, subStageLength);
		}
		break;
	default:
		decodeOnePathRecursive(stage-1, nodeBits, leftNode);
	}

	if(simplifiedTree[rightNode] == RateOne)
	{
		if(simplifiedTree[leftNode] == RateZero)
		{
			P_01(LLRptr, nodeBits, subStageLength);
		}
		else
		{
			if(subStageLength<FLOATSPERVECTOR)
			{
				P_R1(LLRptr, nodeBits, subStageLength);
			}
			else
			{
				P_R1_vectorized(LLRptr, nodeBits, subStageLength);
			}
		}
	}
	else if(simplifiedTree[rightNode] == SPCnode)
	{
		if(simplifiedTree[leftNode] == RateZero)
		{
			if(subStageLength<FLOATSPERVECTOR)
			{
				P_0SPC(LLRptr, nodeBits, subStageLength);
			}
			else
			{
				P_0SPC_vectorized(LLRptr, nodeBits, subStageLength);
			}
		}
		else
		{
			P_RSPC(LLRptr, nodeBits, subStageLength);
		}
	}
	else
	{
		if(simplifiedTree[leftNode] != RateZero)
		{
			G_function_hybrid(LLRptr, LLR[0][stage-1].data(), nodeBits, subStageLength);
		}
		else
		{
			G_function_0R_hybrid(LLRptr, LLR[0][stage-1].data(), subStageLength);
		}

		switch(simplifiedTree[rightNode])
		{
		case RateZero:
			Rate0(rightBits, subStageLength);
			break;
		case RateOne:
			Rate1(LLR[0][stage-1].data(), rightBits, subStageLength);
			break;
		case RepetitionNode:
		case RateHalf:
			Repetition_hybrid(LLR[0][stage-1].data(), rightBits, subStageLength);
			break;
		case SPCnode:
			if(subStageLength == 4)
			{
				SPC_4(LLR[0][stage-1].data(), rightBits);
			}
			else
			{
				SPC(LLR[0][stage-1].data(), rightBits, subStageLength);
			}
			break;
		case RepSPCnode:
			if(subStageLength == 8)
			{
				RepSPC_8(LLR[0][stage-1].data(), rightBits);
			}
			else
			{
				RepSPC(LLR[0][stage-1].data(), rightBits, subStageLength);
			}
			break;
		default:
			decodeOnePathRecursive(stage-1, rightBits, rightNode);
		}

		if(simplifiedTree[leftNode] != RateZero)
		{
			CombineSimple(nodeBits, subStageLength);
		}
		else
		{
			Combine_0RSimple(nodeBits, subStageLength);
		}
	}
}

