#include <polarcode/decoding/avx2_char.h>
#include <cstring>

namespace PolarCode {
namespace Decoding {

__m256i hardDecode(__m256i x) {
	static const __m256i mask = _mm256_set1_epi8(-128);
	const __m256i sign = _mm256_and_si256(x, mask);// Get signs of LLRs
	const __m256i result = _mm256_srli_epi16(sign, 7);//Move them to LSB
	return result;
}

char hardDecode(char llr) {
	return llr<0?1:0;
}

const __m256i absCorrector = _mm256_set1_epi8(-127);
void F_function_calc(__m256i &Left, __m256i &Right, __m256i *Out)
{
	__m256i absL = _mm256_abs_epi8(_mm256_max_epi8(Left, absCorrector));
	__m256i absR = _mm256_abs_epi8(_mm256_max_epi8(Right, absCorrector));
	__m256i minV = _mm256_min_epi8(absL, absR);//minimum of absolute values
	__m256i xorV = _mm256_xor_si256(Left, Right);//multiply signs
	xorV = _mm256_or_si256(xorV, _mm256_set1_epi8(1));//prevent zero as sign value
	__m256i outV = _mm256_sign_epi8(minV, xorV);//merge sign and value
	outV = _mm256_max_epi8(outV, absCorrector);
	_mm256_store_si256(Out, outV);//save
}

void G_function_calc(__m256i &Left, __m256i &Right, __m256i &Bits, __m256i *Out)
{
	__m256i sum  = _mm256_adds_epi8(Right, Left);
	__m256i diff = _mm256_subs_epi8(Right, Left);
	__m256i bitmask = _mm256_slli_epi16(Bits, 7);
	__m256i result = _mm256_blendv_epi8(sum, diff, bitmask);
//	result = _mm256_max_epi8(result, absCorrector);
	_mm256_store_si256(Out, result);
}


void F_function(__m256i *LLRin, __m256i *LLRout, unsigned subBlockLength) {
	__m256i Left, Right;
	if(subBlockLength < 32) {
		Left = _mm256_load_si256(LLRin);
		Right = _mm256_subVectorShift_epu8(Left, subBlockLength*8);
		F_function_calc(Left, Right, LLRout);
	} else {
		unsigned vecCount = nBit2vecCount(subBlockLength);
		for(unsigned i=0; i<vecCount; i++) {
			Left = _mm256_load_si256(LLRin+i);
			Right = _mm256_load_si256(LLRin+i+vecCount);
			F_function_calc(Left, Right, LLRout+i);
		}
	}
}

void G_function(__m256i *LLRin, __m256i *LLRout, __m256i *BitsIn, unsigned subBlockLength) {
	__m256i Left, Right, Bits;
	if(subBlockLength < 32) {
		Left = _mm256_load_si256(LLRin);
		Right = _mm256_subVectorShift_epu8(Left, subBlockLength*8);
		Bits = _mm256_load_si256(BitsIn);
		G_function_calc(Left, Right, Bits, LLRout);
	} else {
		unsigned vecCount = nBit2vecCount(subBlockLength);
		for(unsigned i=0; i<vecCount; i++) {
			Left = _mm256_load_si256(LLRin+i);
			Right = _mm256_load_si256(LLRin+i+vecCount);
			Bits = _mm256_load_si256(BitsIn+i);
			G_function_calc(Left, Right, Bits, LLRout+i);
		}
	}
}

void CombineShortBits(__m256i *Left, __m256i *Right, __m256i *Out, const unsigned subBlockLength) {
	*Left = _mm256_xor_si256(*Left, *Right);
	memset(reinterpret_cast<char*>(Left)+subBlockLength, 0, subBlockLength);
	*Right = _mm256_subVectorBackShift_epu8(*Right, subBlockLength*8);
	__m256i result = _mm256_or_si256(*Left, *Right);
	_mm256_store_si256(Out, result);
}

void Combine(__m256i *Bits, const unsigned vecCount) {
	for(unsigned i=0; i<vecCount; i++) {
		__m256i tempL = _mm256_load_si256(Bits+i);
		__m256i tempR = _mm256_load_si256(Bits+vecCount+i);
		tempL = _mm256_xor_si256(tempL, tempR);
		_mm256_store_si256(Bits+i, tempL);
	}
}

void CombineBits(__m256i *Left, __m256i *Right, __m256i *Out, const unsigned subBlockLength) {
	if(subBlockLength < 32) {
		return CombineShortBits(Left, Right, Out, subBlockLength);
	} else {
		const unsigned vecCount = nBit2vecCount(subBlockLength);
		for(unsigned i=0; i<vecCount; ++i) {
			__m256i tempL = _mm256_load_si256(Left+i);
			__m256i tempR = _mm256_load_si256(Right+i);
			tempL = _mm256_xor_si256(tempL, tempR);
			_mm256_store_si256(Out+i, tempL);
			_mm256_store_si256(Out+vecCount+i, tempR);
		}
	}
}

void RepetitionPrepare(__m256i* x, const size_t codeLength) {
	if(codeLength >= 32) return;
	memset(reinterpret_cast<char*>(x)+codeLength, 0, 32-codeLength);
}

void SpcPrepare(__m256i *x, const size_t codeLength) {
	if(codeLength >= 32) return;
	memset(reinterpret_cast<char*>(x)+codeLength, 127, 32-codeLength);
}

size_t nBit2vecCount(size_t blockLength) {
	return (blockLength+31)/32;
}

}// namespace Decoding
}// namespace PolarCode
