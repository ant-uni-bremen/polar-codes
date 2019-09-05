#ifndef PC_DEC_TEMPLATIZED_FLOAT_H
#define PC_DEC_TEMPLATIZED_FLOAT_H

#include <polarcode/decoding/decoder.h>
#include <polarcode/avxconvenience.h>

#include <iostream>
#include <array>
#include <cmath>

namespace PolarCode {
namespace Decoding {
namespace TemplatizedFloatCalc {

inline float float_or(float &a, float &b) {
	union {
		unsigned int iRet;
		float fRet;
	};
	unsigned int *iA = reinterpret_cast<unsigned int*>(&a);
	unsigned int *iB = reinterpret_cast<unsigned int*>(&b);
	iRet = *iA | *iB;
	return fRet;
}

inline float float_xor(float &a, float &b) {
	union {
		unsigned int iRet;
		float fRet;
	};
	unsigned int *iA = reinterpret_cast<unsigned int*>(&a);
	unsigned int *iB = reinterpret_cast<unsigned int*>(&b);
	iRet = *iA ^ *iB;
	return fRet;
}

inline __m256 hardDecode(__m256 x) {
	const __m256 mask = _mm256_set1_ps(-0.0);
	return _mm256_and_ps(x, mask);// Get signs of LLRs
}

inline float hardDecode(float x) {
/*	unsigned int *iX = reinterpret_cast<unsigned int*>(&x);
	*iX &= 0x80000000U;
	return x;*/
	return (x<0) ? -0.0f : 0.0f;
}

inline void F_function_calc(__m256 &Left, __m256 &Right, float *Out)
{
	const __m256 sgnMask = _mm256_set1_ps(-0.0);
	__m256 absL = _mm256_andnot_ps(sgnMask, Left);
	__m256 absR = _mm256_andnot_ps(sgnMask, Right);
	__m256 minV = _mm256_min_ps(absL, absR);

	__m256 sgnV = _mm256_and_ps(sgnMask, _mm256_xor_ps(Left, Right));
	_mm256_store_ps(Out, _mm256_or_ps(sgnV, minV));
}

inline float F_function_signXor(float &fa, float &fb) {
	unsigned int *ia = reinterpret_cast<unsigned int*>(&fa);
	unsigned int *ib = reinterpret_cast<unsigned int*>(&fb);
	union {
		float fRet;
		unsigned int iRet;
	};

	iRet = (*ia ^ *ib) & 0x80000000U;
	return fRet;
}

inline float F_function_calc(float Left, float Right)
{
	float min = fmin(fabs(Left), fabs(Right));
	float sgn = F_function_signXor(Left, Right);
	return float_or(min, sgn);
}

inline void G_function_calc(__m256 &Left, __m256 &Right, __m256 &Bits, float *Out)
{
	const __m256 HBits = hardDecode(Bits);
	const __m256 MLeft = _mm256_xor_ps(Left, HBits);
	_mm256_store_ps(Out, _mm256_add_ps(MLeft, Right));
}

inline float G_function_calc(float &Left, float &Right, float &Bit)
{
	float hardBit = hardDecode(Bit);
	return Right + float_xor(Left, hardBit);
}

template<unsigned subBlockLength>
inline void F_function(float LLRin[subBlockLength*2], float LLRout[subBlockLength]) {
	if(subBlockLength < 8) {
		for(unsigned i = 0; i < subBlockLength; ++i) {
			float &Left = LLRin[i];
			float &Right = LLRin[i + subBlockLength];
			LLRout[i] = F_function_calc(Left, Right);
		}
	} else {
		for(unsigned i = 0; i < subBlockLength; i += 8) {
			__m256 Left = _mm256_load_ps(LLRin + i);
			__m256 Right = _mm256_load_ps(LLRin + subBlockLength + i);
			F_function_calc(Left, Right, LLRout + i);
		}
	}
}

template<unsigned subBlockLength>
inline void G_function(float LLRin[subBlockLength * 2], float LLRout[subBlockLength], float BitsIn[subBlockLength * 2]) {
	if(subBlockLength < 8) {
		for(unsigned i = 0; i < subBlockLength; ++i) {
			float &Left = LLRin[i];
			float &Right = LLRin[i + subBlockLength];
			float &Bit = BitsIn[i];
			LLRout[i] = G_function_calc(Left, Right, Bit);
		}
	} else {
		for(unsigned i = 0; i < subBlockLength; i += 8) {
			__m256 Left = _mm256_load_ps(LLRin + i);
			__m256 Right = _mm256_load_ps(LLRin + i + subBlockLength);
			__m256 Bits = _mm256_load_ps(BitsIn + i);
			G_function_calc(Left, Right, Bits, LLRout + i);
		}
	}
}

template<unsigned subBlockLength>
inline void G_function_0R(float input[subBlockLength * 2], float output[subBlockLength]) {
	if(subBlockLength < 8) {
		for(unsigned i = 0; i < subBlockLength; ++i) {
			output[i] = input[i] + input[subBlockLength + i];
		}
	} else {
		__m256 left, right, sum;
		for(unsigned i = 0; i < subBlockLength; i += 8) {
			left = _mm256_load_ps(input + i);
			right = _mm256_load_ps(input + subBlockLength + i);
			sum = _mm256_add_ps(left, right);
			_mm256_store_ps(output + i, sum);
		}
	}
}

template<unsigned subBlockLength>
inline void C_function(float *Bits) {
	if(subBlockLength < 8) {
		HybridFloat tempL, tempR;
		for(unsigned i = 0; i < subBlockLength; i++) {
			tempL.f = Bits[i];
			tempR.f = Bits[i + subBlockLength];
			tempL.u ^= tempR.u;
			Bits[i] = tempL.f;
		}
	} else {
		for(unsigned i = 0; i < subBlockLength; i += 8) {
			__m256 tempL = _mm256_load_ps(Bits + i);
			__m256 tempR = _mm256_load_ps(Bits + subBlockLength + i);
			_mm256_store_ps(Bits + i, _mm256_xor_ps(tempL, tempR));
		}
	}
}

template<unsigned subBlockLength>
inline void C_function_0R(float bits[subBlockLength * 2]) {
	for(unsigned i = 0; i < subBlockLength; ++i) {
		bits[i] = bits[subBlockLength + i];
	}
}


}// namespace TemplatizedFloatCalc





// Calculate amount of frozen bits in subcode of size <size> at start bit <begin>
template<int begin, int size, int N>
constexpr int partialSum(const std::array<int, N> &arr) {
	int ret = 0;
	for(int i = 0; i < size; ++i) {
		ret += arr[begin + i];
	}
	return ret;
}


template<const int size>
inline void decodeRateZero(float *output) {
	if(size >= 8) {
		const __m256 inf = _mm256_setzero_ps();
		for(int i = 0; i < size; i += 8) {
			_mm256_store_ps(output + i, inf);
		}
	} else {
		for(int i = 0; i < size; i++) {
			output[i] = 0.0f;
		}
	}
}

template<const int size>
inline void decodeRateOne(float *input, float *output) {
	if(size >= 8) {
		for(int i = 0; i < size; i += 8) {
			_mm256_store_ps(output + i, _mm256_load_ps(input + i));
		}
	} else {
		for(int i = 0; i < size; i++) {
			output[i] = input[i];
		}
	}
}

template<const int size>
inline void decodeRepetition(float *input, float *output) {
	if(size >= 8) {
		__m256 LlrSum = _mm256_setzero_ps();
		for(int i = 0; i < size; i += 8) {
			LlrSum = _mm256_add_ps(LlrSum, _mm256_load_ps(input + i));
		}
		LlrSum = _mm256_set1_ps(reduce_add_ps(LlrSum));
		for(int i = 0; i < size; i += 8) {
			_mm256_store_ps(output + i, LlrSum);
		}
	} else {
		float LlrSum = 0.0f;
		for(int i = 0; i < size; i++) {
			LlrSum += input[i];
		}
		for(int i = 0; i < size; i++) {
			output[i] = LlrSum;
		}
	}
}

template<const int size>
inline void decodeSpc(float *input, float *output) {
	union {
		float fParity;
		unsigned int iParity;
	};
	unsigned minIdx = 0;
	float testAbs, minAbs = INFINITY;

	if(size >= 8) {
		const __m256 sgnMask = _mm256_set1_ps(-0.0);
		__m256 parVec = _mm256_setzero_ps();

		for(unsigned i = 0; i < size; i += 8) {
			__m256 vecIn = _mm256_load_ps(input + i);
			_mm256_store_ps(output + i, vecIn);

			parVec = _mm256_xor_ps(parVec, vecIn);

			__m256 abs = _mm256_andnot_ps(sgnMask, vecIn);
			unsigned vecMin = _mm256_minidx_ps(abs, &testAbs);
			if(testAbs < minAbs) {
				minIdx = vecMin + i;
				minAbs = testAbs;
			}
		}

		// Flip least reliable bit, if neccessary
		fParity = reduce_xor_ps(parVec);

	} else {
		unsigned int *iInput = reinterpret_cast<unsigned int*>(input);
		iParity = 0;
		minAbs = fabs(input[0]);
		for(unsigned i = 0; i < size; ++i) {
			output[i] = input[i];
			iParity ^= iInput[i];
			testAbs = fabs(input[i]);
			if(testAbs < minAbs) {
				minAbs = testAbs;
				minIdx = i;
			}
		}

	}
	iParity &= 0x80000000;
	reinterpret_cast<unsigned int*>(output)[minIdx] ^= iParity;
}

template<const int size>
inline void decodeROneRight(float input[size], float output[size]) {
	using namespace TemplatizedFloatCalc;
	if(size >= 8) {
		for(unsigned i = 0; i < size; i += 8) {
			__m256 left  = _mm256_load_ps(input + i);
			__m256 right = _mm256_load_ps(input + size + i);
			__m256 bit   = _mm256_load_ps(output + i);
			__m256 hbit  = hardDecode(bit);

			__m256 out = _mm256_xor_ps(left, hbit);// G-function
			out = _mm256_add_ps(out, right);// G-function
			/* nop */ // Rate-1 decoder
			_mm256_store_ps(output + i, _mm256_xor_ps(hbit, out));// Store left bit
			_mm256_store_ps(output + size + i, out);// Store right bit
		}
	} else {
		for(unsigned i = 0; i < size; i++) {
			float &left = input[i];
			float &right = input[size + i];
			float &bit = output[i];
			float hbit = hardDecode(bit);

			float out = float_xor(left, hbit);
			out += right;

			output[i] = float_xor(hbit, out);
			output[size + i] = out;
		}
	}
}

template<const unsigned N, const std::array<int, N> &frozenBitSet>
class TemplatizedFloat : public Decoder {

	template<const int begin, const int size>
	inline void decodeRateR(float input[size], float output[size]) {
		using namespace TemplatizedFloatCalc;
		float llr[size / 2];

		F_function<size / 2>(input, llr);
		decodeNode<begin, size / 2>(llr, output);
		G_function<size / 2>(input, llr, output);
		decodeNode<begin + size / 2, size / 2>(llr, output + size / 2);
		C_function<size / 2>(output);
	}

	template<const int begin, const int size>
	inline void decodeROne(float input[size], float output[size]) {
		using namespace TemplatizedFloatCalc;
		float llr[size / 2];

		F_function<size / 2>(input, llr);
		decodeNode<begin, size / 2>(llr, output);
		decodeROneRight<size / 2>(input, output);
	}

	template<const int begin, const int size>
	inline void decodeZeroR(float input[size], float output[size]) {
		using namespace TemplatizedFloatCalc;
		float llr[size / 2];

		G_function_0R<size / 2>(input, llr);
		decodeNode<begin + size / 2, size / 2>(llr, output + size / 2);
		C_function_0R<size / 2>(output);
	}



	template<const int begin, const int size>
	inline void decodeNode(float input[size], float output[size]) {
		constexpr int frozenBitCount = partialSum<begin, size, N>(frozenBitSet);
/* Simplified decoding */
		if(size > 1) {
			if(frozenBitCount == size - 1) {
				return decodeRepetition<size>(input, output);
			} else if(frozenBitCount == 1) {
				return decodeSpc<size>(input, output);
			} // else: continue below
		}
		if(frozenBitCount == size) {
			return decodeRateZero<size>(output);
		} else if(frozenBitCount == 0) {
			return decodeRateOne<size>(input, output);
		} else {
			constexpr int leftFrozenBitCount = partialSum<begin, size / 2, N>(frozenBitSet);
			constexpr int rightFrozenBitCount = partialSum<begin + size / 2, size / 2, N>(frozenBitSet);

			if(rightFrozenBitCount == 0 && size < 8) {
				return decodeROne<begin, size>(input, output);
			} else if(leftFrozenBitCount == size / 2) {
				return decodeZeroR<begin, size>(input, output);
			} else {
				return decodeRateR<begin, size>(input, output);
			}
		}
/* */

/* Full butterfly decoding
		if(size == 1) {
			if(frozenBitCount == 1) {
				decodeRateZero<1>(output);
			} else {
				decodeRateOne<1>(input, output);
			}
		} else {
			decodeRateR<begin, size>(input, output);
		}
*/
	}

public:
	TemplatizedFloat(std::vector<unsigned> frozenBits) {
		mLlrContainer = new FloatContainer(N);
		mBitContainer = new FloatContainer(N, frozenBits);
		mOutputContainer = new unsigned char[(N-frozenBits.size())/8];
	}

	~TemplatizedFloat() {
	}

	bool decode() {
		decodeNode<0, N>(
			dynamic_cast<FloatContainer*>(mLlrContainer)->data(),
			dynamic_cast<FloatContainer*>(mBitContainer)->data());
		mBitContainer->getPackedInformationBits(mOutputContainer);
		return true;
	}
};

}//namespace Decoding
}//namespace PolarCode

#endif
