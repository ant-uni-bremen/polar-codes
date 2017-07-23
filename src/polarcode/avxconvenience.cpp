#include <polarcode/avxconvenience.h>

unsigned _mm256_minpos_epu8(__m256i x)
{
    //Get the two 128-bit lanes
    __m128i l0 = _mm256_extracti128_si256(x, 0);//low lane
    __m128i l1 = _mm256_extracti128_si256(x, 1);//high lane

    //Convert them to epu16
    __m256i ext0 = _mm256_cvtepu8_epi16(l0);
    __m256i ext1 = _mm256_cvtepu8_epi16(l1);

    //Get the four sublanes
    __m128i sl0 = _mm256_extracti128_si256(ext0, 0);
    __m128i sl1 = _mm256_extracti128_si256(ext0, 1);
    __m128i sl2 = _mm256_extracti128_si256(ext1, 0);
    __m128i sl3 = _mm256_extracti128_si256(ext1, 1);

    //Find their minpos
    __m128i mp0 = _mm_minpos_epu16(sl0);
    __m128i mp1 = _mm_minpos_epu16(sl1);
    __m128i mp2 = _mm_minpos_epu16(sl2);
    __m128i mp3 = _mm_minpos_epu16(sl3);

    //Get u16 access
    unsigned short *p[4] = {reinterpret_cast<unsigned short*>(&mp0),
                            reinterpret_cast<unsigned short*>(&mp1),
                            reinterpret_cast<unsigned short*>(&mp2),
                            reinterpret_cast<unsigned short*>(&mp3)};

    //Get four local minima and fill unused entries with dummy value 127
    __m128i collection = _mm_setr_epi16(p[0][0], p[1][0], p[2][0], p[3][0], 127, 127, 127, 127);

    //Get index of total minimum
    __m128i minIdx = _mm_minpos_epu16(collection);
    unsigned short selectedLane = reinterpret_cast<unsigned short*>(&minIdx)[1];
    unsigned ret = p[selectedLane][1]+selectedLane*8;

    return ret;
}

__m256i _mm256_subVectorShift_epu8(__m256i x, int shift) {
	switch(shift) {
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
