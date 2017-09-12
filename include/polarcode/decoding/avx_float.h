#include <polarcode/avxconvenience.h>

namespace PolarCode {
namespace Decoding {

__m256 hardDecode(__m256 x);
float hardDecode(float llr);

void F_function_calc(__m256 &Left, __m256 &Right, float *Out);
void G_function_calc(__m256 &Left, __m256 &Right, __m256 &Bits, float *Out);

void F_function(float *LLRin, float *LLRout, unsigned subBlockLength);
void G_function(float *LLRin, float *LLRout, float *BitsIn, unsigned subBlockLength);
void G_function_0R(float *LLRin, float *LLRout, float *, unsigned subBlockLength);

void Combine(float *Bits, const unsigned bitCount);
void Combine_0R(float *Bits, const unsigned bitCount);
void CombineShortBits(float *Left, float *Right, float *Out, const unsigned subBlockLength);
void CombineBits(float *Left, float *Right, float *Out, const unsigned subBlockLength);

void CombineSoftBits(float *Left, float *Right, float *Out, const unsigned subBlockLength);
void CombineSoftBitsShort(float *Left, float *Right, float *Out, const unsigned subBlockLength);
void CombineSoftBitsLong(float *Left, float *Right, float *Out, const unsigned subBlockLength);

void RepetitionPrepare(float* x, const size_t codeLength);
void SpcPrepare(float* x, const size_t codeLength);

}// namespace Decoding
}// namespace PolarCode

