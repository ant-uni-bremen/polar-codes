#ifndef PC_DEC_AVX_FLOAT_H
#define PC_DEC_AVX_FLOAT_H

#include <polarcode/avxconvenience.h>
#include <cstring>
#include <cassert>
#include <cmath>

namespace PolarCode {
namespace Decoding {
namespace FastSscAvx {

inline __m256 hardDecode(__m256 x) {
	static const __m256 mask = _mm256_set1_ps(-0.0);
	return _mm256_and_ps(x, mask);// Get signs of LLRs
}

inline float hardDecode(float llr) {
	unsigned int* iLlr = reinterpret_cast<unsigned int*>(&llr);
	*iLlr &= 0x80000000;
	return llr;
}

inline void F_function_calc(__m256 &Left, __m256 &Right, float *Out)
{
	static const __m256 sgnMask = _mm256_set1_ps(-0.0);
	__m256 absL = _mm256_andnot_ps(sgnMask, Left);
	__m256 absR = _mm256_andnot_ps(sgnMask, Right);
	__m256 minV = _mm256_min_ps(absL, absR);

	__m256 sgnV = _mm256_and_ps(sgnMask, _mm256_xor_ps(Left, Right));
	_mm256_store_ps(Out, _mm256_or_ps(sgnV, minV));
}

inline void G_function_calc(__m256 &Left, __m256 &Right, __m256 &Bits, float *Out)
{
	const __m256 HBits = hardDecode(Bits);//When using soft-output decoding, hard-decoding is needed at this point.
	const __m256 MLeft = _mm256_xor_ps(Left, HBits);//Change to diff-operation if bit is set, else keep add-operation
	_mm256_store_ps(Out, _mm256_add_ps(MLeft, Right));
}


inline void F_function(float *LLRin, float *LLRout, unsigned subBlockLength) {
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

inline void G_function(float *LLRin, float *LLRout, float *BitsIn, unsigned subBlockLength) {
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

inline void G_function_0R(float *LLRin, float *LLRout, unsigned subBlockLength) {
	__m256 Left, Right, Sum;
	for(unsigned i=0; i<subBlockLength; i+=8) {
		Left = _mm256_load_ps(LLRin+i);
		Right = _mm256_load_ps(LLRin+i+subBlockLength);
		Sum = _mm256_add_ps(Left, Right);
		_mm256_store_ps(LLRout+i, Sum);
	}
}


inline void PrepareForShortOperation(__m256& Left, const unsigned subBlockLength) {
	//memset(reinterpret_cast<float*>(&Left)+subBlockLength, 0, subBlockLength*4);
	unsigned int *ptr = reinterpret_cast<unsigned int*>(&Left);
	for(unsigned int i=subBlockLength; i<2*subBlockLength; ++i) {
		ptr[i] = 0;
	}
}

inline void MoveRightBits(__m256& Right, const unsigned subBlockLength) {
	Right = _mm256_subVectorBackShift_ps(Right, subBlockLength);
}

inline void CombineSoft(float *Bits, const unsigned bitCount) {
	for(unsigned i=0; i<bitCount; i+=8) {
		__m256 tempL = _mm256_load_ps(Bits+i);
		__m256 tempR = _mm256_load_ps(Bits+bitCount+i);
		F_function_calc(tempL, tempR, Bits+i);
	}
}

inline void Combine_0R(float *Bits, const unsigned bitCount) {
	for(unsigned i=0; i<bitCount; ++i) {
		Bits[i] = Bits[i+bitCount];
	}
}

inline void CombineSoftBitsShort(float *Left, float *Right, float *Out, const unsigned subBlockLength) {
	__m256 LeftV = _mm256_load_ps(Left);
	__m256 RightV = _mm256_load_ps(Right);
	__m256 OutV;

	PrepareForShortOperation(LeftV, subBlockLength);
	PrepareForShortOperation(RightV, subBlockLength);

	//Boxplus operation for upper bits
	F_function_calc(LeftV, RightV, reinterpret_cast<float*>(&OutV));

	// Copy operation for lower bits
	MoveRightBits(RightV, subBlockLength);
	OutV = _mm256_or_ps(RightV, OutV);
	_mm256_store_ps(Out, OutV);
}

inline void CombineSoftBitsLong(float *Left, float *Right, float *Out, const unsigned subBlockLength) {
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

inline void RepetitionPrepare(float* x, const unsigned codeLength) {
	for(unsigned i=codeLength; i<8; ++i) {
		x[i] = 0.0;
	}
}

inline void SpcPrepare(float *x, const unsigned codeLength) {
	for(unsigned i=codeLength; i<8; ++i) {
		x[i] = INFINITY;
	}
}

}//namespace FastSscAvx

/*!
 * \brief Convert block length to minimum AVX-vector count.
 * \param blockLength Bits to store
 * \return The number of AVX-vectors required to store _blockLength_ char bits.
 */
inline size_t nBit2fvecCount(size_t blockLength) {
	return (blockLength+7)/8;
}
/*!
 * \brief Expand the blocklength to AVX-vector boundaries of eight floats.
 *
 * For correct AVX-vector access, groups of eight floats need to be allocated.
 * This function expands the given block length to a multiple of eight.
 *
 * \param blockLength Bits to store
 * \return The expanded block length.
 */
inline size_t nBit2fCount(size_t blockLength) {
	return (blockLength+7)&(~7);
}


}// namespace Decoding
}// namespace PolarCode

#endif
