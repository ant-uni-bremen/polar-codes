#include <polarcode/avxconvenience.h>

namespace PolarCode {
namespace Decoding {

/*!
 * \brief Convert block length to minimum AVX-vector count.
 * \param blockLength Bits to store
 * \return The number of AVX-vectors required to store _blockLength_ char bits.
 */
size_t nBit2fvecCount(size_t blockLength);

/*!
 * \brief Expand the blocklength to AVX-vector boundaries of eight floats.
 *
 * For correct AVX-vector access, groups of eight floats need to be allocated.
 * This function expands the given block length to a multiple of eight.
 *
 * \param blockLength Bits to store
 * \return The expanded block length.
 */
size_t nBit2fCount(size_t blockLength);

__m256 hardDecode(__m256 x);
float hardDecode(float llr);

void F_function_calc(__m256 &Left, __m256 &Right, float *Out);
void G_function_calc(__m256 &Left, __m256 &Right, __m256 &Bits, float *Out);

void F_function(float *LLRin, float *LLRout, unsigned subBlockLength);
void G_function(float *LLRin, float *LLRout, float *BitsIn, unsigned subBlockLength);
void G_function_0R(float *LLRin, float *LLRout, float *, unsigned subBlockLength);

void Combine(float *Bits, const unsigned bitCount);
void CombineSoft(float *Bits, const unsigned bitCount);
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

