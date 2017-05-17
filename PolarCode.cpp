#include <cmath>
#include <cstring>
#include <algorithm>
#include <utility>

#include "PolarCode.h"
#include "crc8.h"

using namespace std;


template <typename T> int sgn(T val) {
	return (T(0) < val) - (val < T(0));
}

void PolarCode::F_function(float *LLRin, float *LLRout, int size)
{
	if(size >= FLOATSPERVECTOR)
		return F_function_vectorized(LLRin, LLRout, size);
	if(size == 4)
		return F_function_vectorized_4(LLRin, LLRout);

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
		vec sign  = and_ps(xor_ps(LLR_l, LLR_r), sgnMask256);
		vec abs_l = and_ps(LLR_l, absMask256);
		vec abs_r = and_ps(LLR_r, absMask256);
		vec LLR_o = or_ps(sign, min_ps(abs_l, abs_r));
		store_ps(LLRout+i, LLR_o);
	}
}

void PolarCode::F_function_vectorized_4(float *LLRin, float *LLRout)
{
	__m128 LLR_l = _mm_load_ps(LLRin);
	__m128 LLR_r = _mm_load_ps(LLRin+4);
	__m128 sign  = _mm_and_ps(_mm_xor_ps(LLR_l, LLR_r), sgnMask128);
	__m128 abs_l = _mm_and_ps(LLR_l, absMask128);
	__m128 abs_r = _mm_and_ps(LLR_r, absMask128);
	__m128 LLR_o = _mm_or_ps(sign, _mm_min_ps(abs_l, abs_r));
	_mm_store_ps(LLRout, LLR_o);
}

void PolarCode::G_function(float *LLRin, float *LLRout, float *Bits, int size)
{
	if(size >= FLOATSPERVECTOR)
		return G_function_vectorized(LLRin, LLRout, Bits, size);
	if(size == 4)
		return G_function_vectorized_4(LLRin, LLRout, Bits);

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

void PolarCode::G_function_vectorized_4(float *LLRin, float *LLRout, float *Bits)
{
	__m128 LLR_l  = _mm_load_ps(LLRin);
	__m128 LLR_r  = _mm_load_ps(LLRin+4);
	__m128 Bits_l = _mm_load_ps(Bits);
	__m128 LLR_n  = _mm_xor_ps(Bits_l, LLR_l);
	__m128 LLR_o = _mm_add_ps(LLR_r, LLR_n);
	_mm_store_ps(LLRout, LLR_o);
}

void PolarCode::G_function_0R(float *LLRin, float *LLRout, int size)
{
	if(size >= FLOATSPERVECTOR)
		return G_function_0R_vectorized(LLRin, LLRout, size);
	if(size == 4)
		return G_function_0R_vectorized_4(LLRin, LLRout);

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

void PolarCode::G_function_0R_vectorized_4(float *LLRin, float *LLRout)
{
	__m128 LLR_l  = _mm_load_ps(LLRin);
	__m128 LLR_r  = _mm_load_ps(LLRin+4);
	__m128 LLR_o = _mm_add_ps(LLR_l, LLR_r);
	_mm_store_ps(LLRout, LLR_o);
}

void PolarCode::Rate0(float *BitsOut, int size)
{
	memset(BitsOut, 0, size<<2);
}

void PolarCode::Rate1(float *LLRin, float *BitsOut, int size)
{
	if(size >= FLOATSPERVECTOR)
		return Rate1_vectorized(LLRin, BitsOut, size);
	if(size == 4)
		return Rate1_vectorized_4(LLRin, BitsOut);

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
		LLR_i = and_ps(LLR_i, sgnMask256);
		store_ps(BitsOut+i, LLR_i);
	}
}

void PolarCode::Rate1_vectorized_4(float *LLRin, float *BitsOut)
{
	__m128 LLR_i = _mm_load_ps(LLRin);
	LLR_i = _mm_and_ps(LLR_i, sgnMask128);
	_mm_store_ps(BitsOut, LLR_i);
}

void PolarCode::P_R1(float *LLRin, float *BitsOut, int size)
{
	if(size >= FLOATSPERVECTOR)
		return P_R1_vectorized(LLRin, BitsOut, size);
	if(size == 4)
		return P_R1_vectorized_4(LLRin, BitsOut);


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
		LLR_o = and_ps(LLR_o, sgnMask256);//Rate 1 decoder
		store_ps(BitsOut+i+size, LLR_o);//Right bit
		LLR_o = xor_ps(LLR_o, Bits_l);
		store_ps(BitsOut+i, LLR_o);//Left bit
	}
}

void PolarCode::P_R1_vectorized_4(float *LLRin, float *BitsOut)
{
	__m128 LLR_l = _mm_load_ps(LLRin);
	__m128 LLR_r = _mm_load_ps(LLRin+4);
	__m128 Bits_l = _mm_load_ps(BitsOut);

	__m128 LLR_o = _mm_add_ps(_mm_xor_ps(LLR_l, Bits_l), LLR_r);//G-function
	LLR_o = _mm_and_ps(LLR_o, sgnMask128);//Rate 1 decoder
	_mm_store_ps(BitsOut+4, LLR_o);//Right bit
	LLR_o = _mm_xor_ps(LLR_o, Bits_l);
	_mm_store_ps(BitsOut, LLR_o);//Left bit
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
	if(size == 4)
		return P_RSPC_4(LLRin, BitsOut);

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
		vec Bit_o = and_ps(LLR1, sgnMask256);//Rate-1 decoder
		store_ps(BitsOut+i+size, Bit_o);//Save bit decision
		parityVec = xor_ps(Bit_o, parityVec);//Calculate parity

		Bit_l = xor_ps(Bit_l, Bit_o);
		store_ps(BitsOut+i, Bit_l);//Save upper bit

		Bit_o = and_ps(LLR1, absMask256);
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
void PolarCode::P_RSPC_4(float *LLRin, float *BitsOut)
{
	unsigned int *iBit = reinterpret_cast<unsigned int*>(BitsOut);

	unsigned int parity=0;float *fPar = reinterpret_cast<float*>(&parity);
	int index=0;

	__m128 parityVec = _mm_set1_ps(0.0);

	__m128 LLR_l = _mm_load_ps(LLRin);
	__m128 LLR_r = _mm_load_ps(LLRin+4);
	__m128 Bit_l = _mm_load_ps(BitsOut);

	__m128 LLR1 = _mm_add_ps(_mm_xor_ps(LLR_l, Bit_l), LLR_r);//G-function
	__m128 Bit_o = _mm_and_ps(LLR1, sgnMask128);//Rate-1 decoder
	_mm_store_ps(BitsOut+4, Bit_o);//Save bit decision
	parityVec = _mm_xor_ps(Bit_o, parityVec);//Calculate parity

	Bit_l = _mm_xor_ps(Bit_l, Bit_o);
	_mm_store_ps(BitsOut, Bit_l);//Save upper bit

	Bit_o = _mm_and_ps(LLR1, absMask128);
	_mm_store_ps(absLLR.data(), Bit_o);

	*fPar = _mm_reduce_xor_ps(parityVec);

	if(parity)
	{
		for(int i=1; i<4; ++i)
		{
			if(absLLR[i] < absLLR[index])
			{
				index = i;
			}
		}

		//Flip least reliable bit
		iBit[index] ^= parity;
		iBit[index+4] ^= parity;
	}
}

void PolarCode::P_0SPC(float *LLRin, float *BitsOut, int size)
{
	if(size >= FLOATSPERVECTOR)
		return P_0SPC_vectorized(LLRin, BitsOut, size);
	if(size == 4)
		return P_0SPC_vectorized_4(LLRin, BitsOut);

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
		vec Bit_o = and_ps(LLR1, sgnMask256);//Rate-1 decoder

		store_ps(BitsOut+i, Bit_o);//Save upper bit
		store_ps(BitsOut+i+size, Bit_o);//Save lower bit

		parityVec = xor_ps(Bit_o, parityVec);//Calculate parity

		Bit_o = and_ps(LLR1, absMask256);
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

void PolarCode::P_0SPC_vectorized_4(float *LLRin, float *BitsOut)
{
	unsigned int *iBit = reinterpret_cast<unsigned int*>(BitsOut);

	unsigned int parity=0;float *fPar = reinterpret_cast<float*>(&parity);
	int index=0;

	__m128 parityVec = _mm_set1_ps(0.0);

	//Load data
	__m128 LLR_l = _mm_load_ps(LLRin);
	__m128 LLR_r = _mm_load_ps(LLRin+4);

	__m128 LLR1 = _mm_add_ps(LLR_l, LLR_r);//G-function
	__m128 Bit_o = _mm_and_ps(LLR1, sgnMask128);//Rate-1 decoder
	_mm_store_ps(BitsOut, Bit_o);//Save upper bit
	_mm_store_ps(BitsOut+4, Bit_o);//Save lower bit

	parityVec = _mm_xor_ps(Bit_o, parityVec);//Calculate parity

	Bit_o = _mm_and_ps(LLR1, absMask128);
	_mm_store_ps(absLLR.data(), Bit_o);

	*fPar = _mm_reduce_xor_ps(parityVec);

	if(parity)
	{
		if(absLLR[1] < absLLR[index]) index = 1;
		if(absLLR[2] < absLLR[index]) index = 2;
		if(absLLR[3] < absLLR[index]) index = 3;

		//Flip least reliable bit
		iBit[index] = parity;//No XOR needed here, as left bit was frozen to 0
		iBit[index+4] ^= parity;
	}
}



void PolarCode::CombineSimple(float *Bits, int size)
{
	if(size >= FLOATSPERVECTOR)
		return CombineSimple_vectorized(Bits, size);
	if(size == 4)
		return CombineSimple_vectorized_4(Bits);

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

void PolarCode::CombineSimple_vectorized_4(float *Bits)
{
	__m128 Bitsl = _mm_load_ps(Bits);
	__m128 Bitsr = _mm_load_ps(Bits+4);
	__m128 Bitso = _mm_xor_ps(Bitsl, Bitsr);
	_mm_store_ps(Bits, Bitso);
}

void PolarCode::Combine_0RSimple(float *Bits, int size)
{
	memcpy(Bits, Bits+size, size<<2);
}


void PolarCode::SPC(float *LLRin, float *BitsOut, int size)
{
	if(size == 4)
		return SPC_4(LLRin, BitsOut);

	else
		cerr << "This should not be called!" << endl << flush;

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
	__m128 Sgn = _mm_and_ps(LLRi, sgnMask128);

	_mm_store_ps(BitsOut, Sgn);

	__m128 Par =  _mm_xor_ps(Sgn, _mm_movehl_ps(Sgn, Sgn));
	Par = _mm_xor_ps(Par, _mm_shuffle_ps(Par, Par, 0x55));
	//float parity = _mm_cvtss_f32(Par);
	unsigned int *iPar = reinterpret_cast<unsigned int*>(&Par);
	if(*iPar)
	{
		__m128 Abs = _mm_and_ps(LLRi, absMask128);
		int index = (Abs[1]<Abs[0]);
		if(Abs[2]<Abs[index])index=2;
		if(Abs[3]<Abs[index])index=3;
		unsigned int *iBit = reinterpret_cast<unsigned int*>(BitsOut);
		iBit[index] ^= 0x80000000;
	}
}

void PolarCode::RepSPC_8(float *LLRin, float *BitsOut)
{
	unsigned int *iBits = reinterpret_cast<unsigned int*>(BitsOut);
	float Parity;unsigned int *iParity = reinterpret_cast<unsigned int*>(&Parity);

	__m128 LLR_l = _mm_load_ps(LLRin);
	__m128 LLR_r = _mm_load_ps(LLRin+4);
	__m128 sign = _mm_and_ps(_mm_xor_ps(LLR_l,LLR_r), sgnMask128);
	__m128 abs_l = _mm_and_ps(LLR_l, absMask128);
	__m128 abs_r = _mm_and_ps(LLR_r, absMask128);
	__m128 RepSumVec = _mm_or_ps(sign, _mm_min_ps(abs_l, abs_r));

	__m128 x64 = _mm_add_ps(RepSumVec, _mm_movehl_ps(RepSumVec, RepSumVec));
	__m128 x32 = _mm_add_ss(x64, _mm_shuffle_ps(x64, x64, 0x55));
	x32 = _mm_and_ps(x32, sgnMask128);//Decide Repetition code
	//RepSumVec = _mm_broadcastss_ps(x32);
	RepSumVec = _mm_set1_ps(_mm_cvtss_f32(x32));


	__m128 SPCVec = _mm_add_ps(_mm_xor_ps(LLR_l, RepSumVec), LLR_r);//Load SPC-LLRs
	__m128 SPCsgn = _mm_and_ps(SPCVec, sgnMask128);//Hard decision
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
		__m128 SPCabs = _mm_and_ps(SPCVec, absMask128);
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
	if(size>=FLOATSPERVECTOR)
		return Repetition_vectorized(LLRin, BitsOut, size);
	if(size==4)
		return Repetition_vectorized_4(LLRin, BitsOut);

	float sum = 0.0; unsigned int *iSum = reinterpret_cast<unsigned int*>(&sum);
	for(int i=0; i<size; ++i)
	{
		sum += LLRin[i];
	}
	*iSum &= 0x80000000;
	for(int i=0; i<size; ++i)
	{
		BitsOut[i] = sum;
	}
}

void PolarCode::Repetition_vectorized_4(float *LLRin, float *BitsOut)
{
	__m128 LLRsum = _mm_load_ps(LLRin);
	float Sum = _mm_reduce_add_ps(LLRsum);
	__m128 BitLLR = _mm_set1_ps(Sum);
	__m128 BitDecision = _mm_and_ps(BitLLR, sgnMask128);
	_mm_store_ps(BitsOut, BitDecision);
}

void PolarCode::Repetition_vectorized(float *LLRin, float *BitsOut, int size)
{
	float Sum;

	vec LLRsum = set1_ps(0.0);
	vec TmpVec;
	for(int i=0; i<size; i+=FLOATSPERVECTOR)
	{
		TmpVec = load_ps(LLRin+i);
		LLRsum = add_ps(LLRsum, TmpVec);
	}
	Sum = reduce_add_ps(LLRsum);
	vec BitLLR = set1_ps(Sum);
	vec BitDecision = and_ps(BitLLR, sgnMask256);
	for(int i=0; i<size; i+=FLOATSPERVECTOR)
	{
		store_ps(BitsOut+i, BitDecision);
	}
}


PolarCode::PolarCode(int N, int K, int L, bool useCRC, float designSNR, bool encodeOnly)
{
	n = ceil(log2(N));
	N = 1<<n;
	this->N = N;
	this->K = K;
	this->L = L;
	this->useCRC = useCRC;
	this->designSNR = designSNR;

	FZLookup.resize(N);
	simplifiedTree.resize(2*N-1);

	/* TODO: This is very bad, change CRC8 to do nothing when called */
	Crc = useCRC ? new CRC8() : nullptr;
	
	unsigned long long randomseed[4];
	
	_rdrand64_step(randomseed+0);
	_rdrand64_step(randomseed+1);
	_rdrand64_step(randomseed+2);
	_rdrand64_step(randomseed+3);
	
	r.seed(_mm256_set_epi64x(randomseed[0], randomseed[1], randomseed[2], randomseed[3]));

	if(!encodeOnly)
	{
		AlignedVector = (float*)_mm_malloc(FLOATSPERVECTOR * sizeof(float), sizeof(vec));
		sgnMask256 = set1_ps(-0.0);
		ZERO = set1_ps(0.0);

		{
			float absmask; unsigned int *absptr=reinterpret_cast<unsigned int*>(&absmask);
			*absptr = 0x7FFFFFFF;
			absMask256 = set1_ps(absmask);
		}

		sgnMask128 = _mm256_extractf128_ps(sgnMask256, 0);
		absMask128 = _mm256_extractf128_ps(absMask256, 0);

		maxCandCount = L<<3;
		Metric.resize(maxCandCount);
		newMetrics.resize(maxCandCount);
		cand.resize(maxCandCount);
		SimpleBits.resize(N);



		absLLR.resize(N);

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
	LLR.clear();
	Bits.clear();
	newLLR.clear();
	newBits.clear();
	FZLookup.clear();
	AcceleratedLookup.clear();
	simplifiedTree.clear();
}

void PolarCode::pcc()
{
	vector<float> z(N, 0.0);
	float designSNRlin = pow(10.0, designSNR/10.0);
	z[0] = exp(-1.0 * designSNRlin);

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
	sorter.stableSort();

	for(int i = 0; i<K; ++i)
	{
		FZLookup[sorter.permuted[i]] = true;//Bit is available for user data
		simplifiedTree[N-1+sorter.permuted[i]] = nodeInfo::RateOne;
	}
	for(int i = K; i<N; ++i)
	{
		FZLookup[sorter.permuted[i]] = false;//Freeze bit
		simplifiedTree[N-1+sorter.permuted[i]] = nodeInfo::RateZero;
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
			else if((Left == RateHalf || Left == SPCnode) && Right == RateOne && lev >= n-2)
			{
				simplifiedTree[idx] = SPCnode;
			}
			else if(Left == RateZero && (Right == RateHalf || Right == RepetitionNode))
			{
				simplifiedTree[idx] = RepetitionNode;
			}
			else if(Left == RateZero && Right == RateOne && lev == n-1)
			{
				simplifiedTree[idx] = RateHalf;
			}
			else if(Left == RepetitionNode && Right == SPCnode && lev == n-3)
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
	subEncodeSystematic(encoded, n, 0, 0);
#endif
}

void PolarCode::subEncodeSystematic(aligned_float_vector &encoded, int stage, int BitLocation, int nodeID)
{
	int leftNode  = (nodeID<<1)+1;
	int rightNode = leftNode+1;
	int subStageLength = 1<<(stage-1);

	unsigned int* iData = reinterpret_cast<unsigned int*>(encoded.data());
	float *fData = encoded.data();

	if(stage == 0)
	{
		if(simplifiedTree[nodeID] == RateZero)
		{
			//Insert frozen bit
			iData[BitLocation] = 0;
		}
		else if(simplifiedTree[nodeID] == RateOne)
		{
			cerr << "Trying to insert rate one value" << endl << flush;
		}
		else
		{
			cerr << "Invalid single-bit node" << endl << flush;
		}
		return;
	}

	//Calculate right node
	if(simplifiedTree[rightNode] != RateOne)
		subEncodeSystematic(encoded, stage-1, BitLocation+subStageLength, rightNode);

	//Calculate left node
	if(simplifiedTree[leftNode] != RateZero)
	{
		//XOR left and right to left, to get the left child node bits
		if(subStageLength>=FLOATSPERVECTOR)
		{
			for(int i=0; i<subStageLength; i+=FLOATSPERVECTOR)
			{
				vec a = load_ps(fData+BitLocation+i);
				vec b = load_ps(fData+BitLocation+subStageLength+i);
				vec c = xor_ps(a, b);
				store_ps(fData+BitLocation+i, c);
			}
		}
		else if(subStageLength==4)
		{
			__m128 a = _mm_load_ps(fData+BitLocation);
			__m128 b = _mm_load_ps(fData+BitLocation+4);
			__m128 c = _mm_xor_ps(a, b);
			_mm_store_ps(fData+BitLocation, c);
		}
		else

		{
			for(int i=0; i<subStageLength; ++i)
			{
				iData[BitLocation+i] ^= iData[BitLocation+subStageLength+i];
			}
		}

		//Calculate left node
		if(simplifiedTree[leftNode] != RateOne)
			subEncodeSystematic(encoded, stage-1, BitLocation, leftNode);
		//XOR left and right to left part of parent node
		if(subStageLength>=FLOATSPERVECTOR)
		{
			for(int i=0; i<subStageLength; i+=FLOATSPERVECTOR)
			{
				vec a = load_ps(fData+BitLocation+i);
				vec b = load_ps(fData+BitLocation+subStageLength+i);
				vec c = xor_ps(a, b);
				store_ps(fData+BitLocation+i, c);
			}
		}
		else if(subStageLength==4)
		{
			__m128 a = _mm_load_ps(fData+BitLocation);
			__m128 b = _mm_load_ps(fData+BitLocation+4);
			__m128 c = _mm_xor_ps(a, b);
			_mm_store_ps(fData+BitLocation, c);
		}
		else
		{
			for(int i=0; i<subStageLength; ++i)
			{
				iData[BitLocation+i] ^= iData[BitLocation+subStageLength+i];
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
		else if(B==4)
		{
			base = 0;
			for(int j=0; j<nB; ++j)
			{
				__m128 Bit_l = _mm_load_ps(BitPtr+base);
				__m128 Bit_r = _mm_load_ps(BitPtr+base+4);
				Bit_l = _mm_xor_ps(Bit_l, Bit_r);
				_mm_store_ps(BitPtr+base, Bit_l);
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
	initialLLR = initLLR;

	if(useCRC)
	{
/*		if(decodeOnePath(decoded))
		{
			return true;
		}
		else if(L > 1)
		{
*/			return decodeMultiPath(decoded);
/*		}
		else
		{
*/			/* For a list size of one, there is no need to try again.
			   Every path pruning would decide for the ML path.
			*/
/*			return false;
		}
*/	} else {
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
	decodeOnePathRecursive(n,SimpleBits.data(),0);

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
	float *LLRptr = stage==n ? initialLLR : LLR[0][stage].data();

	if(simplifiedTree[leftNode] != RateZero)
	{
		F_function(LLRptr, LLR[0][stage-1].data(), subStageLength);
	}

	switch(simplifiedTree[leftNode])
	{
	case RateZero:
//		Rate0(nodeBits, subStageLength);
		break;
	case RateOne:
		Rate1(LLR[0][stage-1].data(), nodeBits, subStageLength);
		break;
	case RepetitionNode:
	case RateHalf:
		Repetition(LLR[0][stage-1].data(), nodeBits, subStageLength);
		break;
	case SPCnode:
		SPC(LLR[0][stage-1].data(), nodeBits, subStageLength);
		break;
	case RepSPCnode:
		RepSPC_8(LLR[0][stage-1].data(), nodeBits);
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
			P_R1(LLRptr, nodeBits, subStageLength);
		}
	}
	else if(simplifiedTree[rightNode] == SPCnode)
	{
		if(simplifiedTree[leftNode] == RateZero)
		{
			P_0SPC(LLRptr, nodeBits, subStageLength);
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
			G_function(LLRptr, LLR[0][stage-1].data(), nodeBits, subStageLength);
		}
		else
		{
			G_function_0R(LLRptr, LLR[0][stage-1].data(), subStageLength);
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
			Repetition(LLR[0][stage-1].data(), rightBits, subStageLength);
			break;
		case SPCnode:
			SPC(LLR[0][stage-1].data(), rightBits, subStageLength);
			break;
		case RepSPCnode:
			RepSPC_8(LLR[0][stage-1].data(), rightBits);
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

