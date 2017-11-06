#include <polarcode/avxconvenience.h>

unsigned _mm256_minpos_epu8(__m256i x, char *val)
{
	//Get the two 128-bit lanes
	const __m128i l0 = _mm256_extracti128_si256(x, 0);//low lane
	const __m128i l1 = _mm256_extracti128_si256(x, 1);//high lane

    //Convert them to epu16
	const __m256i ext0 = _mm256_cvtepu8_epi16(l0);
	const __m256i ext1 = _mm256_cvtepu8_epi16(l1);

    //Get the four sublanes
	const __m128i sl0 = _mm256_extracti128_si256(ext0, 0);
	const __m128i sl1 = _mm256_extracti128_si256(ext0, 1);
	const __m128i sl2 = _mm256_extracti128_si256(ext1, 0);
	const __m128i sl3 = _mm256_extracti128_si256(ext1, 1);

	//Find their minpos using the SSE4.1 intrinsic
	const __m128i mp0 = _mm_minpos_epu16(sl0);
	const __m128i mp1 = _mm_minpos_epu16(sl1);
	const __m128i mp2 = _mm_minpos_epu16(sl2);
	const __m128i mp3 = _mm_minpos_epu16(sl3);

    //Get u16 access
	const unsigned short *p[4] = {reinterpret_cast<const unsigned short*>(&mp0),
							reinterpret_cast<const unsigned short*>(&mp1),
							reinterpret_cast<const unsigned short*>(&mp2),
							reinterpret_cast<const unsigned short*>(&mp3)};

    //Get four local minima and fill unused entries with dummy value 127
	const __m128i collection = _mm_setr_epi16(p[0][0], p[1][0], p[2][0], p[3][0], 127, 127, 127, 127);

    //Get index of total minimum
	const __m128i minIdx = _mm_minpos_epu16(collection);
	const unsigned short selectedLane = reinterpret_cast<const unsigned short*>(&minIdx)[1];
	const unsigned ret = p[selectedLane][1]+selectedLane*8;

	if(val!=nullptr) {
		*val = ((char*)&x)[ret];
	}

    return ret;
}

__m256i _mm256_subVectorShift_epu8(__m256i x, int shift) {
	static const __m256i mask[4] = {
		_mm256_set1_epi8(0b0101010-128),
		_mm256_set1_epi8(0b1001100-128),
		_mm256_setzero_si256(),
		_mm256_set1_epi8(0b1110000-128)};
	__m256i y;
	switch(shift) {
	case 1:
	case 2:
	case 4:
		y = _mm256_slli_epi16(x, shift);
		return _mm256_and_si256(y, mask[shift-1]);
	case 8:
		return _mm256_srli_epi16(x, 8);
	case 16:
		return _mm256_srli_epi32(x, 16);
	case 32:
		return _mm256_srli_epi64(x, 32);
	case 64:
		return _mm256_srli_si256(x, 8);
	case 128:
		return _mm256_permute2x128_si256(x, _mm256_setzero_si256(), 0b00100001);
	default:
		throw "Subvector shift of undefined size.";
	}
}

__m256i _mm256_subVectorShiftBytes_epu8(__m256i x, int shift) {
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
		throw "Subvector shift of undefined size.";
	}
}

__m256i _mm256_subVectorBackShift_epu8(__m256i x, int shift) {
	static const __m256i mask[4] = {
		_mm256_set1_epi8(0b01010101),
		_mm256_set1_epi8(0b00110011),
		_mm256_setzero_si256(),
		_mm256_set1_epi8(0b00001111)};
	__m256i y;
	switch(shift) {
	case 1:
	case 2:
	case 4:
		y = _mm256_srli_epi16(x, shift);
		return _mm256_and_si256(y, mask[shift-1]);
	case 8:
		return _mm256_slli_epi16(x, 8);
	case 16:
		return _mm256_slli_epi32(x, 16);
	case 32:
		return _mm256_slli_epi64(x, 32);
	case 64:
		return _mm256_slli_si256(x, 8);
	case 128:
		return _mm256_permute2x128_si256(x, _mm256_setzero_si256(), 0b01001110);
	default:
		throw "Subvector shift of undefined size.";
	}
}

__m256i _mm256_subVectorBackShiftBytes_epu8(__m256i x, int shift) {
	switch(shift) {
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
		throw "Subvector shift of undefined size.";
	}
}

__m256 _mm256_subVectorShift_ps(__m256 x, int shift) {
	__m256 y;
	switch (shift) {
	case 1:
		y = _mm256_shuffle_ps(x, x, 0b00110001);
		return _mm256_blend_ps(y, _mm256_setzero_ps(), 0b10101010);
	case 2:
		return _mm256_shuffle_ps(x, _mm256_setzero_ps(), 0b00001110);
	case 4:
		return _mm256_permute2f128_ps(x, _mm256_setzero_ps(), 0b00110001);
	default:
		throw "Subvector shift of undefined size.";
	}
}

__m256 _mm256_subVectorBackShift_ps(__m256 x, int shift) {
	__m256 y;
	switch (shift) {
	case 1:
		y = _mm256_shuffle_ps(x, x, 0b10000000);
		return _mm256_blend_ps(y, _mm256_setzero_ps(), 0b01010101);
	case 2:
		return _mm256_shuffle_ps(_mm256_setzero_ps(), x, 0b01000000);
	case 4:
		return _mm256_permute2f128_ps(x, _mm256_setzero_ps(), 0b00000010);
	default:
		throw "Subvector shift of undefined size.";
	}
}

bool featureCheckAvx2() {
	return __builtin_cpu_supports("avx2");
}

bool featureCheckAvx() {
	return __builtin_cpu_supports("avx");
}
