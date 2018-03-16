#ifndef PC_FIP_TEMPLATES
#define PC_FIP_TEMPLATES

#include <polarcode/avxconvenience.h>
#include <cstring>
#include <iostream>

namespace PolarCode {
namespace Decoding {
namespace FixedDecoding {

/*****************
 * Genral memory management
 *****************/


inline constexpr unsigned nBit2cvecCount(const unsigned blockLength) {
	return (blockLength + (BYTESPERVECTOR - 1)) / BYTESPERVECTOR;
}


template<unsigned vecCount>
inline void WriteFixedValue(char value, fipv *BitsOut) {
	fipv Output = fi_set1_epi8(value);
	for(unsigned int i = 0; i < vecCount; ++i) {
		fi_store(BitsOut + i, Output);
	}
}

#ifdef __AVX2__

template<unsigned blockLength>
inline void ZeroPrepareShortVector(__m256i &vec) {
	if(blockLength >= 32) return;

	union {
		__m256i mask;
		char cMask[32];
		short sMask[16];
		int iMask[8];
		long long lMask[4];
		__m128i vMask[2];
	};
	__m256i neutral = _mm256_set1_epi8(0);
	mask = _mm256_setzero_si256();

	if (blockLength == 1) {
		cMask[0] = -1;
	} else if (blockLength == 2) {
		sMask[0] = -1;
	} else if (blockLength == 4) {
		iMask[0] = -1;
	} else if (blockLength == 8) {
		lMask[0] = -1;
	} else {
		vMask[0] = _mm_set1_epi64x(-1);
	}

	vec = _mm256_blendv_epi8(neutral, vec, mask);
}

template<unsigned blockLength>
inline void SpcPrepareShortVector(__m256i &vec) {
	if(blockLength >= 32) return;

	union {
		__m256i mask;
		char cMask[32];
		short sMask[16];
		int iMask[8];
		long long lMask[4];
		__m128i vMask[2];
	};
	__m256i neutral = _mm256_set1_epi8(127);
	mask = _mm256_setzero_si256();

	if (blockLength == 1) {
		cMask[0] = -1;
	} else if (blockLength == 2) {
		sMask[0] = -1;
	} else if (blockLength == 4) {
		iMask[0] = -1;
	} else if (blockLength == 8) {
		lMask[0] = -1;
	} else {
		vMask[0] = _mm_set1_epi64x(-1);
	}

	vec = _mm256_blendv_epi8(neutral, vec, mask);
}

template<unsigned shift>
inline __m256i subVectorBackShiftBytes(__m256i x) {
	switch (shift) {
	case 1:
		return _mm256_slli_epi16(x, 8);
	case 2:
		return _mm256_slli_epi32(x, 16);
	case 4:
		return _mm256_slli_epi64(x, 32);
	case 8:
		return _mm256_slli_si256(x, 8);
	case 16:
		return _mm256_permute2x128_si256(x, _mm256_setzero_si256(), 0b01001110);
	default:
		std::cerr << "Subvector backshift of undefined size.";
	}
	return _mm256_setzero_si256();
}

template<unsigned shift>
inline __m256i subVectorShiftBytes_epu8(__m256i x) {
	switch(shift) {
	case 1:
		return _mm256_srli_epi16(x, 8);
	case 2:
		return _mm256_srli_epi32(x, 16);
	case 4:
		return _mm256_srli_epi64(x, 32);
	case 8:
		return _mm256_srli_si256(x, 8);
	case 16:
		return _mm256_permute2x128_si256(x, _mm256_setzero_si256(), 0b00100001);
	default:
		std::cerr << "Subvector shift of undefined size.";
		return _mm256_setzero_si256();
	}
}

#else
template<unsigned blockLength>
inline void ZeroPrepareShortVector(__m128i &vec) {
	if(blockLength >= 16) return;

	union {
		__m128i mask;
		char cMask[16];
		short sMask[8];
		int iMask[4];
		long long lMask[2];
	};
	__m128i neutral = _mm_set1_epi8(0);
	mask = _mm_setzero_si128();

	if (blockLength == 1) {
		cMask[0] = -1;
	} else if (blockLength == 2) {
		sMask[0] = -1;
	} else if (blockLength == 4) {
		iMask[0] = -1;
	} else {
		lMask[0] = -1;
	}

	vec = _mm_blendv_epi8(neutral, vec, mask);
}

template<unsigned blockLength>
inline void SpcPrepareShortVector(__m128i &vec) {
	if(blockLength >= 16) return;

	union {
		__m128i mask;
		char cMask[32];
		short sMask[16];
		int iMask[8];
		long long lMask[4];
	};
	__m128i neutral = _mm_set1_epi8(127);
	mask = _mm_setzero_si128();

	if (blockLength == 1) {
		cMask[0] = -1;
	} else if (blockLength == 2) {
		sMask[0] = -1;
	} else if (blockLength == 4) {
		iMask[0] = -1;
	} else {
		lMask[0] = -1;
	}

	vec = _mm_blendv_epi8(neutral, vec, mask);
}

template<unsigned shift>
inline __m128i subVectorBackShiftBytes(__m128i x) {
	switch (shift) {
	case 1:
		return _mm_slli_epi16(x, 8);
	case 2:
		return _mm_slli_epi32(x, 16);
	case 4:
		return _mm_slli_epi64(x, 32);
	case 8:
		return _mm_slli_si128(x, 8);
	default:
		std::cerr << "Subvector backshift of undefined size.";
	}
	return _mm_setzero_si128();
}

template<unsigned shift>
inline __m128i subVectorShiftBytes_epu8(__m128i x) {
	switch(shift) {
	case 1:
		return _mm_srli_epi16(x, 8);
	case 2:
		return _mm_srli_epi32(x, 16);
	case 4:
		return _mm_srli_epi64(x, 32);
	case 8:
		return _mm_srli_si128(x, 8);
	default:
		std::cerr << "Subvector shift of undefined size.";
	}
}

#endif


/*****************
 * Polar Transformations
 *****************/



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

template<unsigned blockLength>
inline void F_function(fipv *LLRin, fipv *LLRout) {
	fipv Left, Right;
	if (blockLength < BYTESPERVECTOR) {
		Left = fi_load(LLRin);
		Right = subVectorShiftBytes_epu8<blockLength>(Left);
		F_function_calc(Left, Right, LLRout);
	} else {
		constexpr unsigned vecCount = nBit2cvecCount(blockLength);
		for(unsigned i = 0; i < vecCount; i++) {
			Left = fi_load(LLRin + i);
			Right = fi_load(LLRin + i + vecCount);
			F_function_calc(Left, Right, LLRout + i);
		}
	}
}

template<unsigned blockLength>
inline void G_function(fipv *LLRin, fipv *LLRout, fipv *BitsIn) {
	fipv Left, Right, Bits;
	if (blockLength < BYTESPERVECTOR) {
		Left = fi_load(LLRin);
		Right = subVectorShiftBytes_epu8<blockLength>(Left);
		Bits = fi_load(BitsIn);
		G_function_calc(Left, Right, Bits, LLRout);
	} else {
		constexpr unsigned vecCount = nBit2cvecCount(blockLength);
		for(unsigned i = 0; i < vecCount; i++) {
			Left = fi_load(LLRin + i);
			Right = fi_load(LLRin + i + vecCount);
			Bits = fi_load(BitsIn + i);
			G_function_calc(Left, Right, Bits, LLRout + i);
		}
	}
}

template<unsigned blockLength>
inline void G_function_0R(fipv *LLRin, fipv *LLRout) {
	if (blockLength < BYTESPERVECTOR) {
		fipv Left, Right, Sum;
		Left = fi_load(LLRin);
		Right = subVectorShiftBytes_epu8<blockLength>(Left);
		Sum = fi_adds_epi8(Left, Right);
		fi_store(LLRout, Sum);
	} else {
		constexpr unsigned vecCount = nBit2cvecCount(blockLength);
		fipv Left, Right, Sum;
		for(unsigned i = 0; i < vecCount; i++) {
			Left = fi_load(LLRin + i);
			Right = fi_load(LLRin + i + vecCount);
			Sum = fi_adds_epi8(Left, Right);
			fi_store(LLRout + i, Sum);
		}
	}
}

template<unsigned blockLength>
inline void CombineSoftInPlace(fipv *Bits) {
	constexpr unsigned vecCount = nBit2cvecCount(blockLength);
	for(unsigned i = 0; i < vecCount; i++) {
		fipv tempL = fi_load(Bits + i);
		fipv tempR = fi_load(Bits + vecCount + i);
		F_function_calc(tempL, tempR, Bits + i);
	}
}

template<unsigned blockLength>
inline void CombineSoftBits(fipv *Left, fipv *Right, fipv *Out) {
	 if (blockLength < BYTESPERVECTOR) {
		const fipv absCorrector = fi_set1_epi8(-127);

		fipv LeftV = fi_load(Left);
		fipv RightV = fi_load(Right);
		fipv OutV;

		ZeroPrepareShortVector<blockLength>(LeftV);
		ZeroPrepareShortVector<blockLength>(RightV);

		LeftV = fi_max_epi8(LeftV, absCorrector);
		RightV = fi_max_epi8(RightV, absCorrector);

		//Boxplus operation for upper bits
		F_function_calc(LeftV, RightV, &OutV);

		// Copy operation for lower bits
		OutV = fi_or(OutV, subVectorBackShiftBytes<blockLength>(RightV));
		fi_store(Out, OutV);
	 } else {
		constexpr unsigned vecCount = nBit2cvecCount(blockLength);
		for(unsigned i = 0; i < vecCount; ++i) {
			fipv LeftV = fi_load(Left + i);
			fipv RightV = fi_load(Right + i);

			//Copy lower bits
			fi_store(Out + vecCount + i, RightV);

			//Boxplus for upper bits
			F_function_calc(LeftV, RightV, Out + i);
		}
	}
}

template<unsigned blockLength>
inline void Combine_0R(fipv *Bits) {
	const unsigned vecLength = blockLength / 8;
	for(unsigned i = 0; i < vecLength; ++i) {
		fi_store(Bits + i, fi_load(Bits + vecLength + i));
	}
}

template<unsigned blockLength>
inline void Combine_0RShort(fipv *Bits, fipv *RightBits) {
	char* BitPtr = reinterpret_cast<char*>(Bits);
	memcpy(BitPtr,               RightBits, blockLength);
	memcpy(BitPtr + blockLength, RightBits, blockLength);
}

template<>
inline void Combine_0RShort<1>(fipv *Bits, fipv *RightBits) {
	char* BitPtr = reinterpret_cast<char*>(Bits);
	char* RightBitPtr = reinterpret_cast<char*>(RightBits);
	BitPtr[0] = BitPtr[1] = RightBitPtr[0];
}

template<>
inline void Combine_0RShort<2>(fipv *Bits, fipv *RightBits) {
	short* BitPtr = reinterpret_cast<short*>(Bits);
	short* RightBitPtr = reinterpret_cast<short*>(RightBits);
	BitPtr[0] = BitPtr[1] = RightBitPtr[0];
}

template<>
inline void Combine_0RShort<4>(fipv *Bits, fipv *RightBits) {
	int* BitPtr = reinterpret_cast<int*>(Bits);
	int* RightBitPtr = reinterpret_cast<int*>(RightBits);
	BitPtr[0] = BitPtr[1] = RightBitPtr[0];
}

template<>
inline void Combine_0RShort<8>(fipv *Bits, fipv *RightBits) {
	long long* BitPtr = reinterpret_cast<long long*>(Bits);
	long long* RightBitPtr = reinterpret_cast<long long*>(RightBits);
	BitPtr[0] = BitPtr[1] = RightBitPtr[0];
}

#ifdef __AVX2__
template<>
inline void Combine_0RShort<16>(__m256i *Bits, __m256i *RightBits) {
	__m256i vec = _mm256_loadu_si256(RightBits);
	_mm256_store_si256(Bits, vec);
	_mm256_store_si256(Bits+1, vec);
}
#endif

/*****************
 * Decoders
 *****************/


template<unsigned blockLength>
inline void RateZeroDecode(fipv *BitsOut) {
	WriteFixedValue<(blockLength + (BYTESPERVECTOR - 1)) / BYTESPERVECTOR>(127, BitsOut);
}

template<unsigned blockLength>
inline void RateOneDecode(fipv *LlrIn, fipv *BitsOut) {
	constexpr unsigned vecCount = (blockLength + (BYTESPERVECTOR - 1)) / BYTESPERVECTOR;

	for(unsigned i = 0; i < vecCount; ++i) {
		fi_store(BitsOut + i, fi_load(LlrIn + i));
	}
}

template<unsigned blockLength>
inline void RepetitionDecode(fipv *LlrIn, fipv *BitsOut) {
	constexpr unsigned vecCount = (blockLength + (BYTESPERVECTOR - 1)) / BYTESPERVECTOR;
	char Bits;
	fipv LlrSum;

	if (blockLength < BYTESPERVECTOR) {
		ZeroPrepareShortVector<blockLength>(*LlrIn);

		// Get simple sum
		LlrSum = fi_load(LlrIn);
	} else {
		LlrSum = fi_setzero();

		// Accumulate vectors
		for(unsigned i = 0; i < vecCount; i++) {
			LlrSum = fi_adds_epi8(LlrSum, fi_load(LlrIn + i));
		}
	}

	// Get final sum and save decoding result
	Bits = reduce_adds_epi8(LlrSum);
	WriteFixedValue<vecCount>(Bits, BitsOut);
}

template<unsigned blockLength>
inline void SpcDecode(fipv *LlrIn, fipv *BitsOut) {
	constexpr unsigned vecCount = (blockLength + (BYTESPERVECTOR - 1)) / BYTESPERVECTOR;

	if (blockLength >= BYTESPERVECTOR) {
		fipv parVec = fi_setzero();
		unsigned minIdx = 0;
		char testAbs, minAbs = 127;

		// Compute parity and save uncorrected output data
		for(unsigned i = 0; i < vecCount; i++) {
			fipv vecIn = fi_load(LlrIn + i);
			parVec = fi_xor(parVec, vecIn);
			fi_store(BitsOut + i, vecIn);
		}

		unsigned char parity = reduce_xor(parVec) & 0x80;

		// If there was an error, try to correct it
		if(parity) {
			for(unsigned i = 0; i < vecCount; i++) {
				fipv vecIn = fi_load(LlrIn + i);

				fipv abs = fi_abs_epi8(vecIn);
				unsigned vecMin = minpos_epu8(abs, &testAbs);
				if(testAbs < minAbs) {
					minIdx = vecMin + i * BYTESPERVECTOR;
					minAbs = testAbs;
					if(minAbs == 0) break;
				}
			}

			// Flip least reliable bit
			unsigned char *BitPtr = reinterpret_cast<unsigned char*>(BitsOut);
			BitPtr[minIdx] = ~BitPtr[minIdx];
		}
	} else {
		SpcPrepareShortVector<blockLength>(*LlrIn);

		fipv vecIn = fi_load(LlrIn);
		fi_store(BitsOut, vecIn);

		// Flip least reliable bit, if neccessary
		if(reduce_xor(vecIn) & 0x80) {
			fipv abs = fi_abs_epi8(vecIn);
			unsigned vecMin = minpos_epu8(abs);
			unsigned char *BitPtr = reinterpret_cast<unsigned char*>(BitsOut);
			BitPtr[vecMin] = ~BitPtr[vecMin];
		}
	}
}

template<unsigned blockLength>
inline void ZeroSpcDecode(fipv *LlrIn, fipv *BitsOut) {
	unsigned char* BitPtr = reinterpret_cast<unsigned char*>(BitsOut);
	constexpr size_t subBlockLength = blockLength / 2;
	constexpr unsigned vecCount = (subBlockLength + (BYTESPERVECTOR - 1)) / BYTESPERVECTOR;
	unsigned minIdx = 0;
	unsigned char parity;
	char testAbs;

	if (blockLength >= BYTESPERVECTOR) {
		fipv parVec = fi_setzero();
		char minAbs = 127;

		//Check parity equation
		for(unsigned i = 0; i < vecCount; i++) {
			//G-function with only frozen bits
			fipv left = fi_load(LlrIn + i);
			fipv right = fi_load(LlrIn + vecCount + i);
			fipv llr = fi_adds_epi8(left, right);

			//Store output
			fi_store(BitsOut + i, llr);
			fi_store(BitsOut + vecCount + i, llr);


			//Update parity counter
			parVec = fi_xor(parVec, llr);

			// Only search for minimum if there is a chance for smaller absolute value
			if(minAbs > 0) {
				fipv abs = fi_abs_epi8(llr);
				unsigned vecMin = minpos_epu8(abs, &testAbs);
				if(testAbs < minAbs) {
					minIdx = vecMin + i * BYTESPERVECTOR;
					minAbs = testAbs;
				}
			}
		}
		parity = reduce_xor(parVec) & 0x80;
	} else {
		fipv left = fi_load(LlrIn);
		fipv right = subVectorShiftBytes_epu8<subBlockLength>(left);
		fipv llr = fi_adds_epi8(left, right);

		//Set unused bits to SPC-neutral value of 127
		SpcPrepareShortVector<subBlockLength>(llr);

		//Store output
		right = subVectorBackShiftBytes<subBlockLength>(llr);
		fi_store(BitsOut, fi_or(llr, right));

		// Flip least reliable bit, if neccessary
		fipv abs = fi_abs_epi8(llr);
		minIdx = minpos_epu8(abs, &testAbs);
		parity = reduce_xor(llr) & 0x80;
	}

	// Flip least reliable bit, if neccessary
	if(parity) {
		BitPtr[minIdx] = ~BitPtr[minIdx];
		BitPtr[minIdx + subBlockLength] = ~BitPtr[minIdx + subBlockLength];
	}
}

template<unsigned blockLength>
inline void ZeroOneDecodeShort(fipv *LlrIn, fipv *BitsOut) {
	constexpr unsigned subBlockLength = blockLength / 2;
	fipv subLlrLeft, subLlrRight;

	G_function_0R<subBlockLength>(LlrIn, &subLlrLeft);

	ZeroPrepareShortVector<subBlockLength>(subLlrLeft);
	subLlrRight = subVectorBackShiftBytes<subBlockLength>(subLlrLeft);
	fi_store(BitsOut, fi_or(subLlrLeft, subLlrRight));
}

template<unsigned blockLength>
inline void simplifiedRightRateOneDecode(fipv *LlrIn, fipv *BitsOut) {
	constexpr unsigned vecCount = nBit2cvecCount(blockLength);
	for(unsigned i = 0; i < vecCount; ++i) {
		fipv Llr_l = fi_load(LlrIn + i);
		fipv Llr_r = fi_load(LlrIn + i + vecCount);
		fipv Bits = fi_load(BitsOut + i);
		fipv Llr_o;

		G_function_calc(Llr_l, Llr_r, Bits, &Llr_o);
		/*nop*/ //Rate 1 decoder
		F_function_calc(Bits, Llr_o, BitsOut + i);//Combine left bit
		fi_store(BitsOut + i + vecCount, Llr_o);//Copy right bit
	}
}

template<unsigned blockLength>
inline void simplifiedRightRateOneDecodeShort(fipv *LlrIn, fipv *BitsIn, fipv *BitsOut) {
	fipv Bits = fi_load(BitsIn);//Load left bits
	fipv Llr_r_subcode = fi_setzero();//Destination for right subcode

	G_function<blockLength>(LlrIn, &Llr_r_subcode, BitsIn);//Get right child LLRs
	/*nop*/ //Rate 1 decoder
	fipv Bits_r = subVectorBackShiftBytes<blockLength>(Llr_r_subcode);
	fipv Bits_o;
	F_function_calc(Bits, Llr_r_subcode, &Bits_o);//Combine left bits
	ZeroPrepareShortVector<blockLength>(Bits_o);//Clear right bits
	Bits = fi_or(Bits_o, Bits_r);//Merge bits into single vector
	fi_store(BitsOut, Bits);//Save
}

}// namespace FixedDecoding
}// namespace Decoding
}// namespace PolarCode

#endif
