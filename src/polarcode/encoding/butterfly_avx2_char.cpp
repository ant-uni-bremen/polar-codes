#include <polarcode/encoding/butterfly_avx2_char.h>
#include <polarcode/bitcontainer.h>
#include <polarcode/avxconvenience.h>
#include <cmath>


namespace PolarCode {
namespace Encoding {


ButterflyAvx2Char::ButterflyAvx2Char() {
}

ButterflyAvx2Char::ButterflyAvx2Char(
		size_t blockLength,
		const std::set<unsigned> &frozenBits) {
	initialize(blockLength, frozenBits);
}

ButterflyAvx2Char::~ButterflyAvx2Char() {
}

void ButterflyAvx2Char::initialize(
		size_t blockLength,
		const std::set<unsigned> &frozenBits) {
	mBlockLength = blockLength;
	mFrozenBits = frozenBits;

	if(mBitContainer != nullptr) delete mBitContainer;
	mBitContainer = new CharContainer(mBlockLength);
}

void ButterflyAvx2Char::encode() {
	transform();
	if(mSystematic) {
		mBitContainer->resetFrozenBits(mFrozenBits);
		transform();
	}
}

void ButterflyAvx2Char::transform() {
	__m256i *vBit = reinterpret_cast<__m256i*>(
					dynamic_cast<CharContainer*>(
						mBitContainer
					)->data()
				);

	int blockCount = (mBlockLength+31)/32;

	int n = log2(mBlockLength);

	if(n>=1) {
		/*! First manual stage transform, shift bits by 1 */
		for(int block=0; block<blockCount; ++block) {
			__m256i Left = _mm256_load_si256(vBit+block);
			__m256i Right = _mm256_srli_epi16(Left, 8);
			Left = _mm256_xor_si256(Left, Right);
			_mm256_store_si256(vBit+block, Left);
		}

		if(n>=2) {
			/*! Second manual stage, shift bits by 2 */
			for(int block=0; block<blockCount; ++block) {
				__m256i Left = _mm256_load_si256(vBit+block);
				__m256i Right = _mm256_srli_epi32(Left, 16);
				Left = _mm256_xor_si256(Left, Right);
				_mm256_store_si256(vBit+block, Left);
			}

			if(n>=3) {
				/*! Third manual stage, shift bits by 4 */
				for(int block=0; block<blockCount; ++block) {
					__m256i Left = _mm256_load_si256(vBit+block);
					__m256i Right = _mm256_srli_epi64(Left, 32);
					Left = _mm256_xor_si256(Left, Right);
					_mm256_store_si256(vBit+block, Left);
				}

				if(n>=4) {
					/*! Fourth manual stage, shift bits by 8 */
					for(int block=0; block<blockCount; ++block) {
						__m256i Left = _mm256_load_si256(vBit+block);
						__m256i Right = _mm256_srli_si256(Left, 8);
						Left = _mm256_xor_si256(Left, Right);
						_mm256_store_si256(vBit+block, Left);
					}
					if(n>=5) {
						/*! Fifth manual stage, shift bits by 16 */
						for(int block=0; block<blockCount; ++block) {
							__m256i Left = _mm256_load_si256(vBit+block);
							__m256i Right = _mm256_permute2x128_si256(Left, _mm256_setzero_si256(), 0b00100001);
							Left = _mm256_xor_si256(Left, Right);
							_mm256_store_si256(vBit+block, Left);
						}

						if(n>=6) {
							/*! Cross-vector operands can be selected automatically */
							for(int stage = 6; stage<=n; ++stage) {
								int blockShift = 1<<(stage-6);
								int blockJump = blockShift*2;
								for(int group=0; group<blockCount; group+=blockJump) {
									for(int block=0; block < blockShift; block += 1) {
										__m256i Left = _mm256_load_si256(vBit+group+block);
										__m256i Right = _mm256_load_si256(vBit+group+block+blockShift);
										Left = _mm256_xor_si256(Left, Right);
										_mm256_store_si256(vBit+group+block, Left);
									}
								}
							}
						}
					}
				}
			}
		}
	}
}

}//namespace Encoding
}//namespace PolarCode

