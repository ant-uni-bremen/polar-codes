#ifndef PC_DEC_AVX2_CHAR_H
#define PC_DEC_AVX2_CHAR_H

#include <polarcode/avxconvenience.h>
#include <cstring>
#include <cassert>

namespace PolarCode {
namespace Decoding {

/*!
 * \brief Convert block length to minimum AVX-vector count.
 * \param blockLength Bits to store
 * \return The number of AVX-vectors required to store _blockLength_ char bits.
 */
size_t nBit2cvecCount(size_t blockLength);


namespace FastSscAvx2 {

inline __m256i hardDecode(__m256i x) {
	static const __m256i mask = _mm256_set1_epi8(-128);
	const __m256i result = _mm256_and_si256(x, mask);// Get signs of LLRs
	return result;
}

inline char hardDecode(char llr) {
	//return llr<0?-128:127;
	assert((-128>>7) == -1);
	assert((-1>>7) == -1);
	assert(((-1>>7)<<7) == -128);
	return llr & (char)-128;
}

inline void F_function_calc(__m256i Left, __m256i Right, __m256i *Out)
{
	const __m256i absCorrector = _mm256_set1_epi8(-127);
	const __m256i one = _mm256_set1_epi8(1);

	__m256i xorV = _mm256_xor_si256(Left, Right);//multiply signs
	xorV = _mm256_or_si256(xorV, one);//prevent zero as sign value

	Left = _mm256_max_epi8(Left, absCorrector);
	Right = _mm256_max_epi8(Right, absCorrector);

	Left = _mm256_abs_epi8(Left);
	Right = _mm256_abs_epi8(Right);
    
    Left = _mm256_max_epi8(Left, one);
    Right = _mm256_max_epi8(Right, one);

	__m256i minV = _mm256_min_epi8(Left, Right);//minimum of absolute values
	__m256i outV = _mm256_sign_epi8(minV, xorV);//merge sign and value

	_mm256_store_si256(Out, outV);//save
}

inline void G_function_calc(__m256i &Left, __m256i &Right, __m256i &Bits, __m256i *Out)
{
	__m256i sum  = _mm256_adds_epi8(Right, Left);
	__m256i diff = _mm256_subs_epi8(Right, Left);
	__m256i result = _mm256_blendv_epi8(sum, diff, Bits);
	_mm256_store_si256(Out, result);
}


inline void F_function(__m256i *LLRin, __m256i *LLRout, unsigned subBlockLength) {
	__m256i Left, Right;
	if(subBlockLength < 32) {
		Left = _mm256_load_si256(LLRin);
		Right = _mm256_subVectorShiftBytes_epu8(Left, subBlockLength);
		F_function_calc(Left, Right, LLRout);
	} else {
		unsigned vecCount = nBit2cvecCount(subBlockLength);
		for(unsigned i=0; i<vecCount; i++) {
			Left = _mm256_load_si256(LLRin+i);
			Right = _mm256_load_si256(LLRin+i+vecCount);
			F_function_calc(Left, Right, LLRout+i);
		}
	}
}

inline void G_function(__m256i *LLRin, __m256i *LLRout, __m256i *BitsIn, unsigned subBlockLength) {
	__m256i Left, Right, Bits;
	if(subBlockLength < 32) {
		Left = _mm256_load_si256(LLRin);
		Right = _mm256_subVectorShiftBytes_epu8(Left, subBlockLength);
		Bits = _mm256_load_si256(BitsIn);
		G_function_calc(Left, Right, Bits, LLRout);
	} else {
		unsigned vecCount = nBit2cvecCount(subBlockLength);
		for(unsigned i=0; i<vecCount; i++) {
			Left = _mm256_load_si256(LLRin+i);
			Right = _mm256_load_si256(LLRin+i+vecCount);
			Bits = _mm256_load_si256(BitsIn+i);
			G_function_calc(Left, Right, Bits, LLRout+i);
		}
	}
}

inline void G_function_0R(__m256i *LLRin, __m256i *LLRout, unsigned subBlockLength) {
	unsigned vecCount = nBit2cvecCount(subBlockLength);
	__m256i Left, Right, Sum;
	for(unsigned i=0; i<vecCount; i++) {
		Left = _mm256_load_si256(LLRin+i);
		Right = _mm256_load_si256(LLRin+i+vecCount);
		Sum = _mm256_adds_epi8(Left, Right);
		_mm256_store_si256(LLRout+i, Sum);
	}
}

inline void G_function_0RShort(__m256i *LLRin, __m256i *LLRout, unsigned subBlockLength) {
	__m256i Left, Right, Sum;
	Left = _mm256_load_si256(LLRin);
	Right = _mm256_subVectorShiftBytes_epu8(Left, subBlockLength);
	Sum = _mm256_adds_epi8(Left, Right);
	_mm256_store_si256(LLRout, Sum);
}


inline void PrepareForShortOperation(__m256i* Left, const unsigned subBlockLength) {
	memset(reinterpret_cast<char*>(Left)+subBlockLength, 0, 32-subBlockLength);
}

inline void MoveRightBits(__m256i* Right, const unsigned subBlockLength) {
	*Right = _mm256_subVectorBackShiftBytes_epu8(*Right, subBlockLength);
}

/* Following combine functions are named by this scheme:
 *
 *
 * 'Combine' + 'Soft' +  option 1 + option 2
 *
 * 1. 'InPlace' for in-place combination
 *    or 'Bits', when left and right bits are in different sources
 *                    and output will be written toa third location
 *
 * 2. 'Short', when block length is shorter than vector length (32)
 *    or nothing, for fully vectorized operations
 *    or 'Flexible', for adaptive selection of Short or Long
 *
 * 'InPlace' and 'Short' is impossible.
 */

inline void CombineSoftInPlace(__m256i *Bits, const unsigned vecCount) {
	for(unsigned i=0; i<vecCount; i++) {
		__m256i tempL = _mm256_load_si256(Bits+i);
		__m256i tempR = _mm256_load_si256(Bits+vecCount+i);
		F_function_calc(tempL, tempR, Bits+i);
	}
}

inline void CombineSoftBits(__m256i *Left, __m256i *Right, __m256i *Out, const unsigned subBlockLength) {
	const unsigned vecCount = nBit2cvecCount(subBlockLength);
	__m256i LeftV;
	__m256i RightV;
	for(unsigned i=0; i<vecCount; ++i) {
		LeftV = _mm256_load_si256(Left+i);
		RightV = _mm256_load_si256(Right+i);

		//Copy lower bits
		_mm256_store_si256(Out+vecCount+i, RightV);

		//Boxplus for upper bits
		F_function_calc(LeftV, RightV, Out+i);
	}
}

inline void CombineSoftBitsShort(__m256i *Left, __m256i *Right, __m256i *Out, const unsigned subBlockLength) {
	const __m256i absCorrector = _mm256_set1_epi8(-127);
	PrepareForShortOperation(Left, subBlockLength);
	PrepareForShortOperation(Right, subBlockLength);

	__m256i LeftV = _mm256_load_si256(Left);
	__m256i RightV = _mm256_load_si256(Right);
	__m256i OutV;

	LeftV = _mm256_max_epi8(LeftV, absCorrector);
	RightV = _mm256_max_epi8(RightV, absCorrector);

	//Boxplus operation for upper bits
	F_function_calc(LeftV, RightV, &OutV);

	// Copy operation for lower bits
	MoveRightBits(&RightV, subBlockLength);
	OutV = _mm256_or_si256(RightV, OutV);
	_mm256_store_si256(Out, OutV);
}

inline void Combine_0R(__m256i *Bits, const unsigned blockLength) {
	char* BitPtr = reinterpret_cast<char*>(Bits);
	memcpy(BitPtr, BitPtr+blockLength, blockLength);
}

inline void Combine_0RShort(__m256i *Bits, __m256i *RightBits, const unsigned blockLength) {
	char* BitPtr = reinterpret_cast<char*>(Bits);
	memcpy(BitPtr,             RightBits, blockLength);
	memcpy(BitPtr+blockLength, RightBits, blockLength);
}


inline void RepetitionPrepare(__m256i* x, const size_t codeLength) {
	if(codeLength < 32) {
		memset(reinterpret_cast<char*>(x)+codeLength, 0, 32-codeLength);
	}
}

inline void SpcPrepare(__m256i *x, const size_t codeLength) {
	if(codeLength < 32) {
		memset(reinterpret_cast<char*>(x)+codeLength, 127, 32-codeLength);
	}
}


}// namespace FastSscAvx2

inline size_t nBit2cvecCount(size_t blockLength) {
	return (blockLength+31)/32;
}

}// namespace Decoding
}// namespace PolarCode

#endif
