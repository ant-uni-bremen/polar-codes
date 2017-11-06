#include <polarcode/decoding/avx2_char.h>
#include <cstring>
#include <cassert>

namespace PolarCode {
namespace Decoding {

__m256i hardDecode(__m256i x) {
	static const __m256i mask = _mm256_set1_epi8(-128);
	const __m256i result = _mm256_and_si256(x, mask);// Get signs of LLRs
	return result;
}

char hardDecode(char llr) {
	//return llr<0?-128:127;
	assert((-128>>7) == -1);
	assert((-1>>7) == -1);
	assert(((-1>>7)<<7) == -128);
	return llr & (char)-128;
}

void F_function_calc(__m256i &Left, __m256i &Right, __m256i *Out)
{
	static const __m256i absCorrector = _mm256_set1_epi8(-127);
	static const __m256i one = _mm256_set1_epi8(1);
	__m256i absL = _mm256_abs_epi8(_mm256_max_epi8(Left, absCorrector));
	__m256i absR = _mm256_abs_epi8(_mm256_max_epi8(Right, absCorrector));
	__m256i minV = _mm256_min_epi8(absL, absR);//minimum of absolute values
	__m256i xorV = _mm256_xor_si256(Left, Right);//multiply signs
	xorV = _mm256_or_si256(xorV, one);//prevent zero as sign value
	__m256i outV = _mm256_sign_epi8(minV, xorV);//merge sign and value
	//outV = _mm256_max_epi8(outV, absCorrector);
	_mm256_store_si256(Out, outV);//save
}

void G_function_calc(__m256i &Left, __m256i &Right, __m256i &Bits, __m256i *Out)
{
	__m256i sum  = _mm256_adds_epi8(Right, Left);
	__m256i diff = _mm256_subs_epi8(Right, Left);
	__m256i result = _mm256_blendv_epi8(sum, diff, Bits);
	_mm256_store_si256(Out, result);
}


void F_function(__m256i *LLRin, __m256i *LLRout, unsigned subBlockLength) {
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

void G_function(__m256i *LLRin, __m256i *LLRout, __m256i *BitsIn, unsigned subBlockLength) {
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

void G_function_0R(__m256i *LLRin, __m256i *LLRout, unsigned subBlockLength) {
	unsigned vecCount = nBit2cvecCount(subBlockLength);
	__m256i Left, Right, Sum;
	for(unsigned i=0; i<vecCount; i++) {
		Left = _mm256_load_si256(LLRin+i);
		Right = _mm256_load_si256(LLRin+i+vecCount);
		Sum = _mm256_adds_epi8(Left, Right);
		_mm256_store_si256(LLRout+i, Sum);
	}
}

void G_function_0RShort(__m256i *LLRin, __m256i *LLRout, unsigned subBlockLength) {
	__m256i Left, Right, Sum;
	Left = _mm256_load_si256(LLRin);
	Right = _mm256_subVectorShiftBytes_epu8(Left, subBlockLength);
	Sum = _mm256_adds_epi8(Left, Right);
	_mm256_store_si256(LLRout, Sum);
}


void PrepareForShortOperation(__m256i* Left, const unsigned subBlockLength) {
	memset(reinterpret_cast<char*>(Left)+subBlockLength, 0, subBlockLength);
}

void MoveRightBits(__m256i* Right, const unsigned subBlockLength) {
	*Right = _mm256_subVectorBackShiftBytes_epu8(*Right, subBlockLength);
}



void Combine(__m256i *Bits, const unsigned vecCount) {
	for(unsigned i=0; i<vecCount; i++) {
		__m256i tempL = _mm256_load_si256(Bits+i);
		__m256i tempR = _mm256_load_si256(Bits+vecCount+i);
		tempL = _mm256_xor_si256(tempL, tempR);
		_mm256_store_si256(Bits+i, tempL);
	}
}

void Combine_0R(__m256i *Bits, const unsigned blockLength) {
	char* BitPtr = reinterpret_cast<char*>(Bits);
	memcpy(BitPtr, BitPtr+blockLength, blockLength);
}

void Combine_0RShort(__m256i *Bits, __m256i *RightBits, const unsigned blockLength) {
	char* BitPtr = reinterpret_cast<char*>(Bits);
	memcpy(BitPtr,             RightBits, blockLength);
	memcpy(BitPtr+blockLength, RightBits, blockLength);
}

void CombineBits(__m256i *Left, __m256i *Right, __m256i *Out, const unsigned subBlockLength) {
	const unsigned vecCount = nBit2cvecCount(subBlockLength);
	for(unsigned i=0; i<vecCount; ++i) {
		__m256i tempL = _mm256_load_si256(Left+i);
		__m256i tempR = _mm256_load_si256(Right+i);
		tempL = _mm256_xor_si256(tempL, tempR);
		_mm256_store_si256(Out+i, tempL);
		_mm256_store_si256(Out+vecCount+i, tempR);
	}
}

void CombineShortBits(__m256i *Left, __m256i *Right, __m256i *Out, const unsigned subBlockLength) {
	*Left = _mm256_xor_si256(*Left, *Right);
	PrepareForShortOperation(Left, subBlockLength);
	MoveRightBits(Right, subBlockLength);
	__m256i result = _mm256_or_si256(*Left, *Right);
	_mm256_store_si256(Out, result);
}

void CombineSoftBitsShort(__m256i *Left, __m256i *Right, __m256i *Out, const unsigned subBlockLength) {
	PrepareForShortOperation(Left, subBlockLength);

	__m256i LeftV = _mm256_loadu_si256(Left);
	__m256i RightV = _mm256_loadu_si256(Right);
	__m256i OutV;

	//Boxplus operation for upper bits
	F_function_calc(LeftV, RightV, &OutV);

	// Copy operation for lower bits
	MoveRightBits(&RightV, subBlockLength);
	OutV = _mm256_or_si256(RightV, OutV);
	_mm256_store_si256(Out, OutV);
}

void CombineSoftBitsLong(__m256i *Left, __m256i *Right, __m256i *Out, const unsigned subBlockLength) {
	const unsigned vecCount = nBit2cvecCount(subBlockLength);
	__m256i LeftV;
	__m256i RightV;
	for(unsigned i=0; i<vecCount; ++i) {
		LeftV = _mm256_load_si256(Left+i);
		RightV = _mm256_load_si256(Right+i);

		//Boxplus for upper bits
		F_function_calc(LeftV, RightV, Out+i);

		//Copy lower bits
		_mm256_store_si256(Out+vecCount+i, RightV);
	}
}

void CombineSoftBits(__m256i *Left, __m256i *Right, __m256i *Out, const unsigned subBlockLength) {
	if(subBlockLength < 32) {
		return CombineSoftBitsShort(Left, Right, Out, subBlockLength);
	} else {
		return CombineSoftBitsLong(Left, Right, Out, subBlockLength);
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

size_t nBit2cvecCount(size_t blockLength) {
	return (blockLength+31)/32;
}

}// namespace Decoding
}// namespace PolarCode
