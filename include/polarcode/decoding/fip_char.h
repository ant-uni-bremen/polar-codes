/* -*- c++ -*- */
/*
 * Copyright 2018 Florian Lotze
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#ifndef PC_DEC_FIP_CHAR_H
#define PC_DEC_FIP_CHAR_H

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


namespace FastSscFip {

inline fipv hardDecode(fipv x) {
	static const fipv mask = fi_set1_epi8(-128);
	return fi_and(x, mask);// Get signs of LLRs
}

inline char hardDecode(char llr) {
	//return llr<0?-128:127;
	assert((-128>>7) == -1);
	assert((-1>>7) == -1);
	assert(((-1>>7)<<7) == -128);
	return llr & (char)-128;
}

inline void F_function_calc(fipv Left, fipv Right, fipv *Out)
{
	const fipv absCorrector = fi_set1_epi8(-127);
	const fipv one = fi_set1_epi8(1);

	fipv xorV = fi_xor(Left, Right);//multiply signs
	xorV = fi_or(xorV, one);//prevent zero as sign value

	Left = fi_max_epi8(Left, absCorrector);
	Right = fi_max_epi8(Right, absCorrector);

	Left = fi_abs_epi8(Left);
	Right = fi_abs_epi8(Right);

	Left = fi_max_epi8(Left, one);
	Right = fi_max_epi8(Right, one);

	fipv minV = fi_min_epi8(Left, Right);//minimum of absolute values
	fipv outV = fi_sign_epi8(minV, xorV);//merge sign and value

	fi_store(Out, outV);//save
}

inline void G_function_calc(fipv &Left, fipv &Right, fipv &Bits, fipv *Out)
{
	fipv sum  = fi_adds_epi8(Right, Left);
	fipv diff = fi_subs_epi8(Right, Left);
	fipv result = fi_blendv_epi8(sum, diff, Bits);
	fi_store(Out, result);
}


inline void F_function(fipv *LLRin, fipv *LLRout, unsigned subBlockLength) {
	fipv Left, Right;
	if(subBlockLength < BYTESPERVECTOR) {
		Left = fi_load(LLRin);
		Right = subVectorShiftBytes_epu8(Left, subBlockLength);
		F_function_calc(Left, Right, LLRout);
	} else {
		unsigned vecCount = nBit2cvecCount(subBlockLength);
		for(unsigned i = 0; i < vecCount; i++) {
			Left = fi_load(LLRin + i);
			Right = fi_load(LLRin + i + vecCount);
			F_function_calc(Left, Right, LLRout + i);
		}
	}
}

inline void G_function(fipv *LLRin, fipv *LLRout, fipv *BitsIn, unsigned subBlockLength) {
	fipv Left, Right, Bits;
	if(subBlockLength < BYTESPERVECTOR) {
		Left = fi_load(LLRin);
		Right = subVectorShiftBytes_epu8(Left, subBlockLength);
		Bits = fi_load(BitsIn);
		G_function_calc(Left, Right, Bits, LLRout);
	} else {
		unsigned vecCount = nBit2cvecCount(subBlockLength);
		for(unsigned i = 0; i < vecCount; i++) {
			Left = fi_load(LLRin + i);
			Right = fi_load(LLRin + i + vecCount);
			Bits = fi_load(BitsIn + i);
			G_function_calc(Left, Right, Bits, LLRout + i);
		}
	}
}

inline void G_function_0R(fipv *LLRin, fipv *LLRout, unsigned subBlockLength) {
	unsigned vecCount = nBit2cvecCount(subBlockLength);
	fipv Left, Right, Sum;
	for(unsigned i = 0; i < vecCount; i++) {
		Left = fi_load(LLRin + i);
		Right = fi_load(LLRin + i + vecCount);
		Sum = fi_adds_epi8(Left, Right);
		fi_store(LLRout + i, Sum);
	}
}

inline void G_function_0RShort(fipv *LLRin, fipv *LLRout, unsigned subBlockLength) {
	fipv Left, Right, Sum;
	Left = fi_load(LLRin);
	Right = subVectorShiftBytes_epu8(Left, subBlockLength);
	Sum = fi_adds_epi8(Left, Right);
	fi_store(LLRout, Sum);
}


inline void PrepareForShortOperation(fipv* Left, const unsigned subBlockLength) {
	memset(reinterpret_cast<char*>(Left) + subBlockLength // location
		   , 0 // value
		   , BYTESPERVECTOR - subBlockLength); // length
}

inline void MoveRightBits(fipv* Right, const unsigned subBlockLength) {
	*Right = subVectorBackShiftBytes_epu8(*Right, subBlockLength);
}

/* Following combine functions are named by this scheme:
 *
 *
 * 'Combine' + option 1 + option 2
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

inline void CombineInPlace(fipv *Bits, const unsigned vecCount) {
	for(unsigned i = 0; i < vecCount; i++) {
		fipv tempL = fi_load(Bits + i);
		fipv tempR = fi_load(Bits + vecCount + i);
		fi_store(Bits + i, fi_xor(tempL, tempR));
	}
}

inline void CombineBits(fipv *Left, fipv *Right, fipv *Out, const unsigned subBlockLength) {
	const unsigned vecCount = nBit2cvecCount(subBlockLength);
	fipv LeftV;
	fipv RightV;
	for(unsigned i = 0; i < vecCount; ++i) {
		LeftV = fi_load(Left + i);
		RightV = fi_load(Right + i);

		fi_store(Out + i, fi_xor(LeftV, RightV));
		fi_store(Out + i + vecCount, RightV);
	}
}

inline void CombineBitsShort(fipv *Left, fipv *Right, fipv *Out, const unsigned subBlockLength) {
	const fipv absCorrector = fi_set1_epi8(-127);
	PrepareForShortOperation(Left, subBlockLength);
	PrepareForShortOperation(Right, subBlockLength);

	fipv LeftV = fi_load(Left);
	fipv RightV = fi_load(Right);
	fipv OutV;

	LeftV = fi_max_epi8(LeftV, absCorrector);
	RightV = fi_max_epi8(RightV, absCorrector);

	OutV = fi_xor(LeftV, RightV);

	// Copy operation for lower bits
	MoveRightBits(&RightV, subBlockLength);
	OutV = fi_or(RightV, OutV);
	fi_store(Out, OutV);
}

inline void Combine_0R(fipv *Bits, const unsigned blockLength) {
	char* BitPtr = reinterpret_cast<char*>(Bits);
	memcpy(BitPtr, BitPtr + blockLength, blockLength);
}

inline void Combine_0RShort(fipv *Bits, fipv *RightBits, const unsigned blockLength) {
	char* BitPtr = reinterpret_cast<char*>(Bits);
	memcpy(BitPtr,               RightBits, blockLength);
	memcpy(BitPtr + blockLength, RightBits, blockLength);
}


inline void RepetitionPrepare(fipv* x, const size_t codeLength) {
	if(codeLength < BYTESPERVECTOR) {
		memset(reinterpret_cast<char*>(x) + codeLength, 0, BYTESPERVECTOR - codeLength);
	}
}

inline void SpcPrepare(fipv *x, const size_t codeLength) {
	if(codeLength < BYTESPERVECTOR) {
		memset(reinterpret_cast<char*>(x) + codeLength, 127, BYTESPERVECTOR - codeLength);
	}
}


}// namespace FastSscFip

inline size_t nBit2cvecCount(size_t blockLength) {
	return (blockLength + (BYTESPERVECTOR - 1)) / BYTESPERVECTOR;
}

}// namespace Decoding
}// namespace PolarCode

#endif
