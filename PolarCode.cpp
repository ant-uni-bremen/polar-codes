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


static inline float _mm256_reduce_add_ps(__m256 x) {
    /* ( x3+x7, x2+x6, x1+x5, x0+x4 ) */
    const __m128 x128 = _mm_add_ps(_mm256_extractf128_ps(x, 1), _mm256_castps256_ps128(x));
    /* ( -, -, x1+x3+x5+x7, x0+x2+x4+x6 ) */
    const __m128 x64 = _mm_add_ps(x128, _mm_movehl_ps(x128, x128));
    /* ( -, -, -, x0+x1+x2+x3+x4+x5+x6+x7 ) */
    const __m128 x32 = _mm_add_ss(x64, _mm_shuffle_ps(x64, x64, 0x55));
    /* Conversion to float is a no-op on x86-64 */
    return _mm_cvtss_f32(x32);
}

static inline float _mm256_reduce_xor_ps(__m256 x) {
    /* ( x3+x7, x2+x6, x1+x5, x0+x4 ) */
    const __m128 x128 = _mm_xor_ps(_mm256_extractf128_ps(x, 1), _mm256_castps256_ps128(x));
    /* ( -, -, x1+x3+x5+x7, x0+x2+x4+x6 ) */
    const __m128 x64 = _mm_xor_ps(x128, _mm_movehl_ps(x128, x128));
    /* ( -, -, -, x0+x1+x2+x3+x4+x5+x6+x7 ) */
    const __m128 x32 = _mm_xor_ps(x64, _mm_shuffle_ps(x64, x64, 0x55));
    /* Conversion to float is a no-op on x86-64 */
    return _mm_cvtss_f32(x32);
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
		iLLRout[i] = (a&0x80000000) ^ (b&0x80000000);
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
	aligned_float_vector absLLRVec(size);
	
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
		store_ps(absLLRVec.data()+i, Bit_o);
	}
	
	*fPar = _mm256_reduce_xor_ps(parityVec);
	
	if(parity)
	{
		for(int i=0; i<size; ++i)
		{
			if(absLLRVec[i] < absLLRVec[index])
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

	unsigned int parity=0;float *fPar = reinterpret_cast<float*>(&parity);
	int index=0;
	
	vec parityVec = set1_ps(0.0);
	aligned_float_vector absLLRVec(size);
	
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
		store_ps(absLLRVec.data()+i, Bit_o);
	}
	
	*fPar = _mm256_reduce_xor_ps(parityVec);
	
	if(parity)
	{
		for(int i=0; i<size; ++i)
		{
			if(absLLRVec[i] < absLLRVec[index])
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
	if(size >= 8)
	{
		for(int i=0; i<size; i+=FLOATSPERVECTOR)
		{
			vec Bitsl = load_ps(Bits+i);
			vec Bitsr = load_ps(Bits+i+size);
			vec Bitso = xor_ps(Bitsl, Bitsr);
			store_ps(Bits+i, Bitso);
		}
	}
	else
	{
		unsigned int *inlptr = reinterpret_cast<unsigned int*>(Bits),
		             *inrptr = reinterpret_cast<unsigned int*>(Bits+size);
		for(int i=0; i<size; i++)
		{
			inlptr[i] ^= inrptr[i];
		}
	}
}

void PolarCode::Combine_0RSimple(float *Bits, int size)
{
	memcpy(Bits, Bits+size, sizeof(float)*size);
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
		*iRepAcc ^= (*ia&0x80000000) ^ (*ib&0x80000000);
		RepSum += RepAcc;
		
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
		
		//Decide both possibilities for SPC code
		a = LLRin[i+subSize] + LLRin[i];
		b = LLRin[i+subSize] - LLRin[i];
		SPC0[i] = *ia & 0x80000000;
		SPC1[i] = *ib & 0x80000000;
		//and update the parity check
		parA ^= SPC0[i];
		parB ^= SPC1[i];
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
		memcpy(SPC1, SPC0, sizeof(float)*subSize);
	}
}



void PolarCode::RepSPC_8(float *LLRin, float *BitsOut)
{
	unsigned int *iBits = reinterpret_cast<unsigned int*>(BitsOut);
	float Parity;unsigned int *iParity = reinterpret_cast<unsigned int*>(&Parity);
	
	__m128 sgnMask = _mm256_extractf128_ps(SIGN_MASK, 0);
	__m128 absMask = _mm256_extractf128_ps(ABS_MASK, 0);
	
	__m128 LLR_l = _mm_load_ps(LLRin);
	__m128 LLR_r = _mm_load_ps(LLRin+4);
	__m128 sign = _mm_and_ps(_mm_xor_ps(LLR_l,LLR_r), sgnMask);
	__m128 abs_l = _mm_and_ps(LLR_l, absMask);
	__m128 abs_r = _mm_and_ps(LLR_r, absMask);
	__m128 RepSumVec = _mm_or_ps(sign, _mm_min_ps(abs_l, abs_r));

	__m128 x64 = _mm_add_ps(RepSumVec, _mm_movehl_ps(RepSumVec, RepSumVec));
    __m128 x32 = _mm_add_ss(x64, _mm_shuffle_ps(x64, x64, 0x55));
    x32 = _mm_and_ps(x32, sgnMask);//Decide Repetition code
	RepSumVec = _mm_broadcastss_ps(x32);
	
	
	__m128 SPCVec = _mm_add_ps(_mm_xor_ps(LLR_l, RepSumVec), LLR_r);//Load SPC-LLRs
	__m128 SPCsgn = _mm_and_ps(SPCVec, sgnMask);//Hard decision
	RepSumVec = _mm_xor_ps(RepSumVec, SPCsgn);
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
		if(SPCabs[1] < SPCabs[0])
		{
			index=1;
			if(SPCabs[2] < SPCabs[1])
			{
				index=2;
				if(SPCabs[3] < SPCabs[2])
				{
					index=3;
				}
			}
		}
		iBits[index] ^= 0x80000000;
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
	Sum = _mm256_reduce_add_ps(LLRsum);
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



PolarCode::PolarCode(int N, int K, int L, float designSNR)
{
	AlignedVector = (float*)_mm_malloc(FLOATSPERVECTOR * sizeof(float), sizeof(vec));
	SIGN_MASK = set1_ps(-0.0);
	
	{
		float absmask; unsigned int *absptr=reinterpret_cast<unsigned int*>(&absmask);
		*absptr = 0x7FFFFFFF;
		ABS_MASK = set1_ps(absmask);
	}
	
	this->N = N;
	this->K = K;
	this->L = L;
	this->designSNR = designSNR;
	
	n = ceil(log2(N));

	//Initialize all the arrays
	FZLookup.resize(N);
	simplifiedTree.resize(2*N-1);

	resetMemory();
	
	
	pcc();

}
		
PolarCode::~PolarCode()
{
	_mm_free(AlignedVector);
}

void PolarCode::resetMemory()
{
	Metric.assign(1, 0.0);
	PathCount = 1;
	
	SimpleBits.assign(N, 0.0);

	LLR.resize(1);
	Bits.resize(1);

	LLR[0].resize(n+1);
	Bits[0].assign(N, 0.0);
	for(int stage=0; stage<=n; ++stage)
	{
		LLR[0][stage].assign(std::max(FLOATSPERVECTOR, 1<<stage), 0.0);		
	}
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
	z[0] = -((float)K/N)*designSNRlin;
	
	float T; int B;

	for(int lev=n-1; lev >= 0; --lev)
	{
		B = 1<<lev;//pow(2, lev);
		for(int j = 0; j < N; j+=(B<<1))
		{
			T = z[j];
			z[j] = logdomain_diff(log(2.0)+T, 2*T);
			z[j+B] = 2*T;
		}
	}
	
	trackingSorter sorter(z);
	sorter.stableSort();
	
	for(int i = 0; i<K; ++i)
	{
		if(sorter.sorted[i] == 0)
		{
			std::cout << "Caution: Lookup uses unreliable channel " << sorter.permuted[i] << " for designSNR of " << designSNR << "!" << std::endl;
		}
		FZLookup[sorter.permuted[i]] = true;//Bit is available for user data
		simplifiedTree[N-1+sorter.permuted[i]] = nodeInfo::RateOne;
	}
	for(int i = K; i<N; ++i)
	{
		FZLookup[sorter.permuted[i]] = false;//Freeze bit
		simplifiedTree[N-1+sorter.permuted[i]] = nodeInfo::RateZero;
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
			else if((Left == RateHalf || Left == SPCnode) && Right == RateOne)
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

void PolarCode::encode(vector<bool> &encoded, vector<bool> &data)
{
	encoded.assign(N, 0);
#ifdef CRCSIZE
	CRC8 *CrcGenerator = new CRC8();
	
	//Calculate CRC
	CrcGenerator->addChecksum(data);
#endif
	
	//Insert the bits into Rate-1 channels
	auto bitptr = data.begin();
	for(int i=0; i<N; ++i)
	{
		if(FZLookup[i])
		{
			encoded[i] = *bitptr;
			++bitptr;
		}
	}
	
	//Encode
	int B, nB, base;
	for(int i=n-1; i>=0; --i)
	{
		B = 1<<(n-i-1);
		nB = 1<<i;
		for(int j=0; j<nB; ++j)
		{
			base = j*(B<<1);
			for(int l=0; l<B; ++l)
			{
				encoded[base+l] = (encoded[base+l] != encoded[base+l +B]);// here '!=' works as XOR-operator. '^' doesn't...
			}
		}
	}
#ifdef CRCSIZE	
	delete CrcGenerator;
#endif
}

void PolarCode::transform(aligned_float_vector &BitsIn, vector<bool> &BitsOut)
{
	int size = BitsOut.size();
	unsigned int *FloatBit = reinterpret_cast<unsigned int *>(BitsIn.data());
	for(int i=0; i<size; ++i)
	{
		BitsOut[i] = FloatBit[i]&0x80000000;
	}

	int B, nB, base;
	for(int i=n-1; i>=0; --i)
	{
		B = 1<<(n-i-1);
		nB = 1<<i;
		for(int j=0; j<nB; ++j)
		{
			base = j*(B<<1);
			for(int l=0; l<B; ++l)
			{
				BitsOut[base+l] = (BitsOut[base+l] != BitsOut[base+l +B]);// here '!=' works as XOR-operator. '^' doesn't...
			}
		}
	}
}

bool PolarCode::decode(vector<bool> &decoded, vector<float> &initialLLR)
{
	if(decodeOnePath(decoded, initialLLR))
	{
		return true;
	}
	else if(L > 1)
	{
		return decodeMultiPath(decoded, initialLLR);
	}
	else
	{
		/* For a list size of one, there is no need to try again.
		   Every path pruning would decide for the ML path.
		*/
		return false;
	}
}

bool PolarCode::decodeOnePath(vector<bool> &decoded, vector<float> &initialLLR)
{
	int backupL = L;
	L = 1;
	resetMemory();
#ifdef CRCSIZE
	CRC8 *Crc = new CRC8();
#endif
//	for(int i=0; i<N; ++i)
//	{
//		/* copy element by element to the aligned storage */
//		LLR[0][n][i] = initialLLR[i];
//	}

	std::copy(initialLLR.begin(), initialLLR.end(), LLR[0][n].begin());

#ifdef ONLY_SCDECODING
	decodeOnePathRecursive(n,SimpleBits.data(),0);
#else

	float *BitPtr = SimpleBits.data();

#include "SpecialDecoder.cpp"

#endif

	vector<bool> Dhat(N);
	
	transform(SimpleBits, Dhat);

	auto bitptrA = decoded.begin();
	auto bitptrB = Dhat.begin();
	for(int bit=0; bit<N; ++bit)
	{
		if(FZLookup[bit])
		{
			*bitptrA = *bitptrB;
			++bitptrA;
		}
		++bitptrB;
	}
#ifdef CRCSIZE
	bool success = Crc->check(decoded);	
	delete Crc;
#else
	bool success = true;
#endif

	L = backupL;
	
	return success;
}

void PolarCode::decodeOnePathRecursive(int stage, float *nodeBits, int nodeID)
{
	int leftNode  = (nodeID<<1)+1;
	int rightNode = leftNode+1;
	int subStageLength = 1<<(stage-1);
	float *rightBits = nodeBits+subStageLength;

	if(simplifiedTree[leftNode] != RateZero)
	{
		F_function_hybrid(LLR[0][stage].data(), LLR[0][stage-1].data(), subStageLength);
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
		SPC(LLR[0][stage-1].data(), nodeBits, subStageLength);
		break;
	case RepSPCnode:
		RepSPC(LLR[0][stage-1].data(), nodeBits, subStageLength);
		break;
	default:
		decodeOnePathRecursive(stage-1, nodeBits, leftNode);
	}

	if(simplifiedTree[leftNode] != RateZero)
	{
		G_function_hybrid(LLR[0][stage].data(), LLR[0][stage-1].data(), nodeBits, subStageLength);
	}
	else
	{
		G_function_0R_hybrid(LLR[0][stage].data(), LLR[0][stage-1].data(), subStageLength);
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
		SPC(LLR[0][stage-1].data(), rightBits, subStageLength);
		break;
	case RepSPCnode:
		RepSPC(LLR[0][stage-1].data(), rightBits, subStageLength);
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


