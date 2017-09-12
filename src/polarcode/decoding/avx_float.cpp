#include <polarcode/decoding/avx_float.h>
#include <cstring>
#include <cassert>

namespace PolarCode {
namespace Decoding {

__m256 hardDecode(__m256 x) {
	static const __m256 mask = _mm256_set1_ps(-0.0);
	const __m256 result = _mm256_and_ps(x, mask);// Get signs of LLRs
	return result;
}

float hardDecode(float llr) {
	unsigned int* iLlr = reinterpret_cast<unsigned int*>(&llr);
	*iLlr &= 0x80000000;
	return llr;
}

void F_function_calc(__m256 &Left, __m256 &Right, float *Out)
{
	static const __m256 sgnMask = _mm256_set1_ps(-0.0);
	__m256 absL = _mm256_andnot_ps(sgnMask, Left);
	__m256 absR = _mm256_andnot_ps(sgnMask, Right);
	__m256 minV = _mm256_min_ps(absL, absR);

	__m256 sgnV = _mm256_and_ps(sgnMask, _mm256_xor_ps(Left, Right));
	_mm256_store_ps(Out, _mm256_or_ps(sgnV, minV));
}

void G_function_calc(__m256 &Left, __m256 &Right, __m256 &Bits, float *Out)
{
	Bits = hardDecode(Bits);//When using soft-output decoding, hard-decoding is needed.
	Left = _mm256_xor_ps(Left, Bits);//Change to diff-operation if bit is set, else keep add-operation
	_mm256_store_ps(Out, _mm256_add_ps(Left, Right));
}


void F_function(float *LLRin, float *LLRout, unsigned subBlockLength) {
	__m256 Left, Right;
	if(subBlockLength < 8) {
		Left = _mm256_load_ps(LLRin);
		Right = _mm256_subVectorShift_ps(Left, subBlockLength);
		F_function_calc(Left, Right, LLRout);
	} else {
		for(unsigned i=0; i<subBlockLength; i+=8) {
			Left = _mm256_load_ps(LLRin+i);
			Right = _mm256_load_ps(LLRin+subBlockLength+i);
			F_function_calc(Left, Right, LLRout+i);
		}
	}
}

void G_function(float *LLRin, float *LLRout, float *BitsIn, unsigned subBlockLength) {
	__m256 Left, Right, Bits;
	if(subBlockLength < 8) {
		Left = _mm256_load_ps(LLRin);
		Right = _mm256_subVectorShift_ps(Left, subBlockLength);
		Bits = _mm256_load_ps(BitsIn);
		G_function_calc(Left, Right, Bits, LLRout);
	} else {
		for(unsigned i=0; i<subBlockLength; i+=8) {
			Left = _mm256_load_ps(LLRin+i);
			Right = _mm256_load_ps(LLRin+i+subBlockLength);
			Bits = _mm256_load_ps(BitsIn+i);
			G_function_calc(Left, Right, Bits, LLRout+i);
		}
	}
}

void G_function_0R(float *LLRin, float *LLRout, float*, unsigned subBlockLength) {
	__m256 Left, Right, Sum;
	for(unsigned i=0; i<subBlockLength; i+=8) {
		Left = _mm256_load_ps(LLRin+i);
		Right = _mm256_load_ps(LLRin+i+subBlockLength);
		Sum = _mm256_add_ps(Left, Right);
		_mm256_store_ps(LLRout+i, Sum);
	}
}


void PrepareForShortOperation(__m256& Left, const unsigned subBlockLength) {
	memset(reinterpret_cast<float*>(&Left)+subBlockLength, 0, subBlockLength*4);
}

void MoveRightBits(__m256& Right, const unsigned subBlockLength) {
	Right = _mm256_subVectorBackShift_ps(Right, subBlockLength);
}

void CombineShortBits(float *Left, float *Right, float *Out, const unsigned subBlockLength) {
	__m256 vLeft = _mm256_load_ps(Left);
	__m256 vRight = _mm256_load_ps(Right);
	vLeft = _mm256_xor_ps(vLeft, vRight);
	PrepareForShortOperation(vLeft, subBlockLength);
	MoveRightBits(vRight, subBlockLength);
	__m256 result = _mm256_or_ps(vLeft, vRight);
	_mm256_store_ps(Out, result);
}

void Combine(float *Bits, const unsigned bitCount) {
	for(unsigned i=0; i<bitCount; i+=8) {
		__m256 tempL = _mm256_load_ps(Bits+i);
		__m256 tempR = _mm256_load_ps(Bits+bitCount+i);
		tempL = _mm256_xor_ps(tempL, tempR);
		_mm256_store_ps(Bits+i, tempL);
	}
}

void Combine_0R(float *Bits, const unsigned bitCount) {
	memcpy(Bits, Bits+bitCount, bitCount*4);
}

void CombineBits(float *Left, float *Right, float *Out, const unsigned subBlockLength) {
	if(subBlockLength < 8) {
		return CombineShortBits(Left, Right, Out, subBlockLength);
	} else {
		for(unsigned i=0; i<subBlockLength; i+=8) {
			__m256 tempL = _mm256_load_ps(Left+i);
			__m256 tempR = _mm256_load_ps(Right+i);
			tempL = _mm256_xor_ps(tempL, tempR);
			_mm256_store_ps(Out+i, tempL);
			_mm256_store_ps(Out+subBlockLength+i, tempR);
		}
	}
}


void CombineSoftBitsShort(float *Left, float *Right, float *Out, const unsigned subBlockLength) {
	__m256 LeftV = _mm256_loadu_ps(Left);
	__m256 RightV = _mm256_loadu_ps(Right);
	__m256 OutV;

	PrepareForShortOperation(LeftV, subBlockLength);

	//Boxplus operation for upper bits
	F_function_calc(LeftV, RightV, reinterpret_cast<float*>(&OutV));

	// Copy operation for lower bits
	MoveRightBits(RightV, subBlockLength);
	OutV = _mm256_or_ps(RightV, OutV);
	_mm256_store_ps(Out, OutV);
}

void CombineSoftBitsLong(float *Left, float *Right, float *Out, const unsigned subBlockLength) {
	__m256 LeftV;
	__m256 RightV;
	for(unsigned i=0; i<subBlockLength; i+=8) {
		LeftV = _mm256_load_ps(Left+i);
		RightV = _mm256_load_ps(Right+i);

		//Boxplus for upper bits
		F_function_calc(LeftV, RightV, Out+i);

		//Copy lower bits
		_mm256_store_ps(Out+subBlockLength+i, RightV);
	}
}

void CombineSoftBits(float *Left, float *Right, float *Out, const unsigned subBlockLength) {
	if(subBlockLength < 8) {
		return CombineSoftBitsShort(Left, Right, Out, subBlockLength);
	} else {
		return CombineSoftBitsLong(Left, Right, Out, subBlockLength);
	}
}

void RepetitionPrepare(float* x, const size_t codeLength) {
	if(codeLength >= 8) return;
	for(unsigned i=codeLength; i<8; ++i) {
		x[i] = 0.0;
	}
}

void SpcPrepare(float *x, const size_t codeLength) {
	if(codeLength >= 8) return;
	for(unsigned i=codeLength; i<8; ++i) {
		x[i] = 1.0/0;//positive infinity
	}
}

}// namespace Decoding
}// namespace PolarCode
