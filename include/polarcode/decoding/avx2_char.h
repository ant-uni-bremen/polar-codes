#include <polarcode/avxconvenience.h>

namespace PolarCode {
namespace Decoding {

/*!
 * \brief Convert block length to minimum AVX-vector count.
 * \param blockLength Bits to store
 * \return The number of AVX-vectors required to store _blockLength_ char bits.
 */
size_t nBit2vecCount(size_t blockLength);

__m256i hardDecode(__m256i x);
char hardDecode(char llr);

void F_function_calc(__m256i &Left, __m256i &Right, __m256i *Out);
void G_function_calc(__m256i &Left, __m256i &Right, __m256i &Bits, __m256i *Out);

void F_function(__m256i *LLRin, __m256i *LLRout, unsigned subBlockLength);
void G_function(__m256i *LLRin, __m256i *LLRout, __m256i *BitsIn, unsigned subBlockLength);

void Combine(__m256i *Bits, const unsigned vecCount);
void CombineShortBits(__m256i *Left, __m256i *Right, __m256i *Out, const unsigned subBlockLength);
void CombineBits(__m256i *Left, __m256i *Right, __m256i *Out, const unsigned subBlockLength);

void CombineSoftBits(__m256i *Left, __m256i *Right, __m256i *Out, const unsigned subBlockLength);
void CombineSoftBitsShort(__m256i *Left, __m256i *Right, __m256i *Out, const unsigned subBlockLength);
void CombineSoftBitsLong(__m256i *Left, __m256i *Right, __m256i *Out, const unsigned subBlockLength);

void RepetitionPrepare(__m256i* x, const size_t codeLength);
void SpcPrepare(__m256i* x, const size_t codeLength);

}// namespace Decoding
}// namespace PolarCode

