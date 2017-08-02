#include <polarcode/encoding/butterfly_avx_float.h>
#include <polarcode/bitcontainer.h>
#include <polarcode/avxconvenience.h>
#include <cmath>


namespace PolarCode {
namespace Encoding {


ButterflyAvxFloat::ButterflyAvxFloat() {
}

ButterflyAvxFloat::ButterflyAvxFloat(
		size_t blockLength,
		const std::set<unsigned> &frozenBits) {
	initialize(blockLength, frozenBits);
}

ButterflyAvxFloat::~ButterflyAvxFloat() {
}

void ButterflyAvxFloat::initialize(
		size_t blockLength,
		const std::set<unsigned> &frozenBits) {
	mBlockLength = blockLength;
	mFrozenBits = frozenBits;

	if(mBitContainer != nullptr) delete mBitContainer;
	mBitContainer = new FloatContainer(mBlockLength, mFrozenBits);
}

void ButterflyAvxFloat::encode() {
	transform();
	if(mSystematic) {
		mBitContainer->resetFrozenBits();
		transform();
	}
}

void ButterflyAvxFloat::transform() {
	float *fBit = dynamic_cast<FloatContainer*>(mBitContainer)->data();
	unsigned int *iBit = reinterpret_cast<unsigned int *>(fBit);

	int B, nB, base, inc;
	int n = __builtin_ctz(mBlockLength);//log2() on powers of 2

	// Copy/paste from old version. Won't be used in future, as it is totally
	// useless to expand bits to 32-bit words only to compress them back lateron.
	// Therefore, see ButterflyAvx2*** encoders. In case AVX2 is not available,
	// a packed SSE version can be constructed from AVX2 with minor changes.
	for(int i=n-1; i>=0; --i)
	{
		B = 1<<(n-i-1);
		nB = 1<<i;
		inc = B<<1;
		if(B>=8)
		{
			base = 0;
			for(int j=0; j<nB; ++j)
			{
				for(int l=0; l<B; l+=8)
				{
					__m256 Bit_l = _mm256_load_ps(fBit+base+l);
					__m256 Bit_r = _mm256_load_ps(fBit+base+l+B);
					Bit_l = _mm256_xor_ps(Bit_l, Bit_r);
					_mm256_store_ps(fBit+base+l, Bit_l);
				}
				base += inc;
			}
		}
		else if(B==4)
		{
			base = 0;
			for(int j=0; j<nB; ++j)
			{
				__m128 Bit_l = _mm_load_ps(fBit+base);
				__m128 Bit_r = _mm_load_ps(fBit+base+4);
				Bit_l = _mm_xor_ps(Bit_l, Bit_r);
				_mm_store_ps(fBit+base, Bit_l);
				base += inc;
			}

		}
		else
		{
			base = 0;
			for(int j=0; j<nB; ++j)
			{
				for(int l=0; l<B; ++l)
				{
					iBit[base+l] ^= iBit[base+l+B];
				}
				base += inc;
			}
		}
	}

}

}//namespace Encoding
}//namespace PolarCode
