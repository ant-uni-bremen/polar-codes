#include <polarcode/decoding/fixed_avx2_char.h>
#include <polarcode/errordetection/dummy.h>
#include <polarcode/encoding/butterfly_avx2_packed.h>

namespace PolarCode {
namespace Decoding {

namespace FixedDecoding {

FixedDecoder::FixedDecoder() {
}

FixedDecoder::~FixedDecoder() {
}

}// namespace FixedDecoding


FixedChar::FixedChar(unsigned int scheme) {
	mDecoder = FixedDecoding::createFixedDecoder(scheme);

	mBlockLength = codeRegistry[scheme].blockLength;
	mSystematic = codeRegistry[scheme].systematic;
	mFrozenBits.assign(codeRegistry[scheme].frozenBits.begin(),
					   codeRegistry[scheme].frozenBits.end());
	mLlrContainer = new CharContainer(mBlockLength, mFrozenBits);
	mBitContainer = new CharContainer(mBlockLength, mFrozenBits);
	mOutputContainer = new unsigned char[(codeRegistry[scheme].infoLength+7)/8];
}

FixedChar::~FixedChar() {
	delete mDecoder;
}

bool FixedChar::decode() {
	mDecoder->decode(reinterpret_cast<CharContainer*>(mLlrContainer)->data(),
					 reinterpret_cast<CharContainer*>(mBitContainer)->data());

	if(!mSystematic) {
		Encoding::Encoder* encoder = new Encoding::ButterflyAvx2Packed(mBlockLength);
		encoder->setSystematic(false);
		encoder->setCodeword(dynamic_cast<CharContainer*>(mBitContainer)->data());
		encoder->encode();
		encoder->getEncodedData(dynamic_cast<CharContainer*>(mBitContainer)->data());
		delete encoder;
	}

	mBitContainer->getPackedInformationBits(mOutputContainer);
	bool result = mErrorDetector->check(mOutputContainer, (mBlockLength-mFrozenBits.size()+7)/8);
	return result;
}

}// namespace Decoding
}// namespace PolarCode

