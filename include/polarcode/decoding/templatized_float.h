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

template <typename T> inline int sgn(T val) {
	return (T(0) < val) - (val < T(0));
}

inline __m256 hardDecode(__m256 x) {
	static const __m256 mask = _mm256_set1_ps(-0.0);
	return _mm256_and_ps(x, mask);// Get signs of LLRs
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

inline float F_function_calc(float &Left, float &Right)
{
	float absL = fabs(Left);
	float absR = fabs(Right);
	float minV = fmin(absL, absR);

	return minV * sgn(Left) * sgn(Right);
}

inline void G_function_calc(__m256 &Left, __m256 &Right, __m256 &Bits, float *Out)
{
	const __m256 HBits = hardDecode(Bits);
	const __m256 MLeft = _mm256_xor_ps(Left, HBits);
	_mm256_store_ps(Out, _mm256_add_ps(MLeft, Right));
}

inline float G_function_calc(float &Left, float &Right, float &Bit)
{
	const float hardBit = sgn(Bit);
	return Right + Left * hardBit;
}

template<unsigned subBlockLength>
inline void F_function(float *LLRin, float LLRout[subBlockLength]) {
	if(subBlockLength < 8) {
		for(unsigned i = 0; i < subBlockLength; ++i) {
			float &Left = LLRin[i];
			float &Right = LLRin[i + subBlockLength];
			LLRout[i] = TemplatizedFloatCalc::F_function_calc(Left, Right);
		}
	} else {
		for(unsigned i = 0; i < subBlockLength; i += 8) {
			__m256 Left = _mm256_load_ps(LLRin + i);
			__m256 Right = _mm256_load_ps(LLRin + subBlockLength + i);
			TemplatizedFloatCalc::F_function_calc(Left, Right, LLRout + i);
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
			LLRout[i] = TemplatizedFloatCalc::G_function_calc(Left, Right, Bit);
		}
	} else {
		for(unsigned i = 0; i < subBlockLength; i += 8) {
			__m256 Left = _mm256_load_ps(LLRin + i);
			__m256 Right = _mm256_load_ps(LLRin + i + subBlockLength);
			__m256 Bits = _mm256_load_ps(BitsIn + i);
			TemplatizedFloatCalc::G_function_calc(Left, Right, Bits, LLRout + i);
		}
	}
}

template<unsigned subBlockLength>
inline void C_function(float *Bits) {
	if(subBlockLength < 8) {
		for(unsigned i = 0; i < subBlockLength; i++) {
			float tempL = Bits[i];
			float tempR = Bits[i + subBlockLength];
			Bits[i] = TemplatizedFloatCalc::F_function_calc(tempL, tempR);
		}
	} else {
		for(unsigned i = 0; i < subBlockLength; i += 8) {
			__m256 tempL = _mm256_load_ps(Bits + i);
			__m256 tempR = _mm256_load_ps(Bits + subBlockLength + i);
			TemplatizedFloatCalc::F_function_calc(tempL, tempR, Bits + i);
		}
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
		const __m256 inf = _mm256_set1_ps(INFINITY);
		for(int i = 0; i < size; i += 8) {
			_mm256_store_ps(output + i, inf);
		}
	} else {
		for(int i = 0; i < size; i++) {
			output[i] = INFINITY;
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
			_mm256_storeu_ps(output + i, LlrSum);
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
    std::cout << "SPC: ";
    if(iParity) {
        std::cout << "Corrected bit " << minIdx << " from " << input[minIdx] << " to " << output[minIdx] << "." << std::endl;
    } else {
        std::cout << "No error detected." << std::endl;
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
	inline void decodeNode(float input[size], float output[size]) {
		constexpr int frozenBitCount = partialSum<begin, size, N>(frozenBitSet);
/* Simplified decoding */
		if(size > 1) {
			if(frozenBitCount == size - 1) {
				return decodeRepetition<size>(input, output);
			} else if(frozenBitCount == 1) {
				return decodeSpc<size>(input, output);
			}
		}
		if(frozenBitCount == size) {
			return decodeRateZero<size>(output);
		} else if(frozenBitCount == 0) {
			return decodeRateOne<size>(input, output);
		} else {
			return decodeRateR<begin, size>(input, output);
		}
/* */

/* Precise soft-output decoding
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
	TemplatizedFloat() {
		mLlrContainer = new FloatContainer(N);
		mBitContainer = new FloatContainer(N);
	}

	~TemplatizedFloat() {
	}

	bool decode() {
		decodeNode<0, N>(
			dynamic_cast<FloatContainer*>(mLlrContainer)->data(),
			dynamic_cast<FloatContainer*>(mBitContainer)->data());
		return true;
	}
};

}//namespace Decoding
}//namespace PolarCode

#endif
