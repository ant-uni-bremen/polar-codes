#include <polarcode/decoding/fastssc_avx2_char.h>
#include <polarcode/encoding/butterfly_avx2_packed.h>
#include <polarcode/polarcode.h>

#include <string>
#include <iostream>

#include <cstring> //for memset
#include <cmath>

namespace PolarCode {
namespace Decoding {

namespace FastSscAvx2 {


Node::Node()
	: mLlr(nullptr),
	  mBit(nullptr),
	  xmDataPool(nullptr),
	  mBlockLength(0),
	  mVecCount(0) {
}

Node::Node(size_t blockLength, datapool_t *pool)
	: mLlr(pool->allocate(nBit2cvecCount(blockLength))),
	  mBit(pool->allocate(nBit2cvecCount(blockLength))),
	  xmDataPool(pool),
	  mBlockLength(blockLength),
	  mVecCount(nBit2cvecCount(blockLength)){
}

Node::~Node() {
	clearBlocks();
}

void Node::decode(__m256i *LlrIn, __m256i *BitsOut) {
}

void Node::clearBlocks() {
	clearLlrBlock();
	clearBitBlock();
}

void Node::clearLlrBlock() {
	if(mLlr != nullptr) {
		xmDataPool->release(mLlr);
		mLlr = nullptr;
	}
}

void Node::clearBitBlock() {
	if(mBit != nullptr) {
		xmDataPool->release(mBit);
		mBit = nullptr;
	}
}

size_t Node::blockLength() {
	return mBlockLength;
}

Node::datapool_t* Node::pool() {
	return xmDataPool;
}

__m256i* Node::input() {
	return mLlr->data;
}

__m256i* Node::output() {
	return mBit->data;
}


// Constructors of nodes

std::string decoderName(void (*specialDecoder)(__m256i*, __m256i*, size_t)) {
	if(specialDecoder == &RateZeroDecode)
		return std::string("Zero");
	if(specialDecoder == &RateOneDecode)
		return std::string("One");
	if(specialDecoder == &RepetitionDecode)
		return std::string("Rep");
	if(specialDecoder == &SpcDecode)
		return std::string("SPC");
	return std::string("R");
}

RateRNode::RateRNode(const std::vector<unsigned> &frozenBits, Node *parent)
	: mParent(parent){
	xmDataPool = parent->pool();
	mBlockLength = parent->blockLength() / 2;

	std::vector<unsigned> leftFrozenBits, rightFrozenBits;
	splitFrozenBits(frozenBits, mBlockLength, leftFrozenBits, rightFrozenBits);

	mLeft = createDecoder(leftFrozenBits, this, &leftDecoder);
	mRight = createDecoder(rightFrozenBits, this, &rightDecoder);

	if((!mLeft || !mRight) && mBlockLength >= 32) {
//		std::cout << decoderName(leftDecoder) << " " << decoderName(rightDecoder) << ": " << mBlockLength << std::endl;
	}

	mVecCount = nBit2cvecCount(mBlockLength);

	ChildLlr = xmDataPool->allocate(mVecCount);
}

ShortRateRNode::ShortRateRNode(const std::vector<unsigned> &frozenBits, Node *parent)
	: RateRNode(frozenBits, parent),
	  LeftBits(xmDataPool->allocate(mVecCount)),
	  RightBits(xmDataPool->allocate(mVecCount)) {
}

ROneNode::ROneNode(const std::vector<unsigned> &frozenBits, Node *parent)
	: RateRNode(frozenBits, parent) {
}

ZeroRNode::ZeroRNode(const std::vector<unsigned> &frozenBits, Node *parent)
	: RateRNode(frozenBits, parent) {
}

// Destructors of nodes

RateRNode::~RateRNode() {
	delete mLeft;
	delete mRight;
	xmDataPool->release(ChildLlr);
}

ShortRateRNode::~ShortRateRNode() {
	xmDataPool->release(LeftBits);
	xmDataPool->release(RightBits);
}

ROneNode::~ROneNode() {
}

ZeroRNode::~ZeroRNode() {
}

// Decoders


void RateZeroDecode(__m256i *LlrIn, __m256i *BitsOut, const size_t blockLength) {
	memset(BitsOut, 127, blockLength);
}

void RateOneDecode(__m256i *LlrIn, __m256i *BitsOut, const size_t blockLength) {
	memcpy(BitsOut, LlrIn, blockLength);
}


/* WARNING: Saturation can lead to wrong results!

	127 + 127 + 127 + 127 + -128 results in -1 and thus wrong sign
	after step-by-step saturated addition.

	Conversion to epi16 will reduce throughput but circumvents that problem.
*/
void RepetitionDecode(__m256i *LlrIn, __m256i *BitsOut, const size_t blockLength) {
	unsigned vecCount = (blockLength+31)/32;
	__m256i LlrSum = _mm256_setzero_si256();

	RepetitionPrepare(LlrIn, blockLength);

	// Accumulate vectors
	for(unsigned i=0; i<vecCount; i++) {
		LlrSum = _mm256_adds_epi8(LlrSum, _mm256_load_si256(LlrIn+i));
	}

	// Get final sum and save decoding result
	char Bits = reduce_adds_epi8(LlrSum);
	memset(BitsOut, Bits, blockLength);
}

void SpcDecode(__m256i *LlrIn, __m256i *BitsOut, const size_t blockLength) {
	unsigned vecCount = (blockLength+31)/32;
	__m256i parVec = _mm256_setzero_si256();
	unsigned minIdx = 0;
	char testAbs, minAbs = 127;

	SpcPrepare(LlrIn, blockLength);

	memcpy(BitsOut, LlrIn, blockLength);

	for(unsigned i=0; i<vecCount; i++) {
		__m256i vecIn = _mm256_load_si256(LlrIn+i);

		parVec = _mm256_xor_si256(parVec, vecIn);

		// Only search for minimum if there is a chance for smaller absolute value
		if(minAbs/* > 0*/) {
			__m256i abs = _mm256_abs_epi8(vecIn);
			unsigned vecMin = _mm256_minpos_epu8(abs, &testAbs);
			if(testAbs < minAbs) {
				minIdx = vecMin+i;
				minAbs = testAbs;
			}
		}
	}

	// Flip least reliable bit, if neccessary
	reinterpret_cast<unsigned char*>(BitsOut)[minIdx] ^= reduce_xor_si256(parVec);
}

void ZeroSpcDecode(__m256i *LlrIn, __m256i *BitsOut, const size_t blockLength) {
	std::cout << "ZeroSpc";
	const size_t subBlockLength = blockLength/2;
	unsigned vecCount = (subBlockLength+31)/32;
	__m256i parVec = _mm256_setzero_si256();
	unsigned minIdx = 0;
	char testAbs, minAbs = 127;

	//Generate output
	memcpy(BitsOut,                LlrIn+subBlockLength, subBlockLength);
	memcpy(BitsOut+subBlockLength, LlrIn+subBlockLength, subBlockLength);

	//Check parity equation
	for(unsigned i=0; i<vecCount; i++) {
		//G-function with only frozen bits
		__m256i left = _mm256_load_si256(LlrIn+i);
		__m256i right = _mm256_load_si256(LlrIn+vecCount+i);
		__m256i llr = _mm256_adds_epi8(left, right);

		//Update parity counter
		parVec = _mm256_xor_si256(parVec, llr);

		// Only search for minimum if there is a chance for smaller absolute value
		if(minAbs/* > 0*/) {
			__m256i abs = _mm256_abs_epi8(llr);
			unsigned vecMin = _mm256_minpos_epu8(abs, &testAbs);
			if(testAbs < minAbs) {
				minIdx = vecMin+i;
				minAbs = testAbs;
			}
		}
	}

	// Flip least reliable bit, if neccessary
	reinterpret_cast<unsigned char*>(BitsOut)[minIdx] ^= reduce_xor_si256(parVec);
	reinterpret_cast<unsigned char*>(BitsOut+subBlockLength)[minIdx] ^= reduce_xor_si256(parVec);
}

void simplifiedRightRateOneDecode(__m256i *LlrIn, __m256i *BitsOut, const size_t blockLength) {
	const size_t vecCount = nBit2cvecCount(blockLength);
	for(unsigned i = 0; i < vecCount; ++i) {
		__m256i Llr_l = _mm256_load_si256(LlrIn+i);
		__m256i Llr_r = _mm256_load_si256(LlrIn+i+vecCount);
		__m256i Bits = _mm256_load_si256(BitsOut+i);

		__m256i Llr_o = _mm256_xor_si256(Llr_l, Bits);//G-function
		Llr_o = _mm256_and_si256(Llr_o, Llr_r);//G-function
		/*nop*/ //Rate 1 decoder
		_mm256_store_si256(BitsOut+i+vecCount, Llr_o);//Right bit
		Llr_o = _mm256_xor_si256(Llr_o, Bits);//Combine
		_mm256_store_si256(BitsOut+i, Llr_o);//Left bit

	}
}


void RateRNode::decode(__m256i *LlrIn, __m256i *BitsOut) {
	F_function(LlrIn, ChildLlr->data, mBlockLength);

	if(mLeft) {
		mLeft->decode(ChildLlr->data, BitsOut);
	} else {
		leftDecoder(ChildLlr->data, BitsOut, mBlockLength);
	}

	G_function(LlrIn, ChildLlr->data, BitsOut, mBlockLength);

	if(mRight) {
		mRight->decode(ChildLlr->data, BitsOut+mVecCount);
	} else {
		rightDecoder(ChildLlr->data, BitsOut+mVecCount, mBlockLength);
	}

	Combine(BitsOut, mVecCount);
}

void ShortRateRNode::decode(__m256i *LlrIn, __m256i *BitsOut) {
	F_function(LlrIn, ChildLlr->data, mBlockLength);

	if(mLeft) {
		mLeft->decode(ChildLlr->data, LeftBits->data);
	} else {
		leftDecoder(ChildLlr->data, LeftBits->data, mBlockLength);
	}

	G_function(LlrIn, ChildLlr->data, LeftBits->data, mBlockLength);

	if(mRight) {
		mRight->decode(ChildLlr->data, RightBits->data);
	} else {
		rightDecoder(ChildLlr->data, RightBits->data, mBlockLength);
	}

	CombineShortBits(LeftBits->data, RightBits->data, BitsOut, mBlockLength);
}

void ROneNode::decode(__m256i *LlrIn, __m256i *BitsOut) {
	F_function(LlrIn, ChildLlr->data, mBlockLength);

	if(mLeft) {
		mLeft->decode(ChildLlr->data, BitsOut);
	} else {
		leftDecoder(ChildLlr->data, BitsOut, mBlockLength);
	}

	simplifiedRightRateOneDecode(LlrIn, BitsOut, mBlockLength);
}

void ZeroRNode::decode(__m256i *LlrIn, __m256i *BitsOut) {
	G_function_0R(LlrIn, ChildLlr->data, BitsOut, mBlockLength);

	if(mRight) {
		mRight->decode(ChildLlr->data, BitsOut+mVecCount);
	} else {
		rightDecoder(ChildLlr->data, BitsOut+mVecCount, mBlockLength);
	}

	Combine_0R(BitsOut, mVecCount);
}

// End of mass defining

Node* createDecoder(const std::vector<unsigned> &frozenBits, Node* parent, void (**specialDecoder)(__m256i *, __m256i *, size_t)) {
	size_t blockLength = parent->blockLength();
	size_t frozenBitCount = frozenBits.size();

	// Begin with the two most simple codes:
	if(frozenBitCount == blockLength) {
		*specialDecoder = &RateZeroDecode;
		return nullptr;
	}
	if(frozenBitCount == 0) {
		*specialDecoder = &RateOneDecode;
		return nullptr;
	}

	// Following are "one bit unlike the others" codes:
	if(frozenBitCount == (blockLength-1)) {
		*specialDecoder = &RepetitionDecode;
		return nullptr;
	}
	if(frozenBitCount == 1) {
		*specialDecoder = &SpcDecode;
		return nullptr;
	}

	// Fallback: No special code available, split into smaller subcodes
	if(blockLength <= 32) {
		*specialDecoder = nullptr;
		return new ShortRateRNode(frozenBits, parent);
	} else {
		std::vector<unsigned> leftFrozenBits, rightFrozenBits;
		splitFrozenBits(frozenBits, blockLength/2, leftFrozenBits, rightFrozenBits);

//		std::cout << leftFrozenBits.size() << "," << rightFrozenBits.size() << "=>";
		//Last case of optimization:
		//Common child node combinations
		if(leftFrozenBits.size() == blockLength/2 && rightFrozenBits.size() == 1) {
			*specialDecoder = &ZeroSpcDecode;
//			std::cout << "ZeroSPC" << std::endl;
			return nullptr;
		}


		//Minor optimization:
		*specialDecoder = nullptr;
		//Right rate-1
		if(rightFrozenBits.size() == 0) {
			return new ROneNode(frozenBits, parent);
		}
		//Left rate-0
		if(leftFrozenBits.size() == blockLength/2) {
			return new ZeroRNode(frozenBits, parent);
		}

		return new RateRNode(frozenBits, parent);
	}
}

}// namespace FastSscAvx2

FastSscAvx2Char::FastSscAvx2Char(size_t blockLength, const std::vector<unsigned> &frozenBits) {
	initialize(blockLength, frozenBits);
}

FastSscAvx2Char::~FastSscAvx2Char() {
	clear();
}

void FastSscAvx2Char::clear() {
	if(mRootNode) delete mRootNode;
	delete mNodeBase;
	delete mDataPool;
}

void FastSscAvx2Char::initialize(size_t blockLength, const std::vector<unsigned> &frozenBits) {
	if(blockLength == mBlockLength && frozenBits == mFrozenBits) {
		return;
	}
	if(mBlockLength != 0) {
		clear();
	}
	mBlockLength = blockLength;
	//mFrozenBits = frozenBits;
	mFrozenBits.assign(frozenBits.begin(), frozenBits.end());
	mDataPool = new DataPool<__m256i, 32>();
	mNodeBase = new FastSscAvx2::Node(blockLength, mDataPool);
	//std::cout << "Create decoder of length " << mBlockLength << std::endl;
	mRootNode = FastSscAvx2::createDecoder(frozenBits, mNodeBase, &mSpecialDecoder);
	mLlrContainer = new CharContainer(reinterpret_cast<char*>(mNodeBase->input()),  mBlockLength);
	mBitContainer = new CharContainer(reinterpret_cast<char*>(mNodeBase->output()), mBlockLength);
	mLlrContainer->setFrozenBits(mFrozenBits);
	mBitContainer->setFrozenBits(mFrozenBits);
	mOutputContainer = new unsigned char[(mBlockLength-frozenBits.size()+7)/8];
}

bool FastSscAvx2Char::decode() {
	if(mRootNode) {
		mRootNode->decode(mNodeBase->input(), mNodeBase->output());
	} else {
		mSpecialDecoder(mNodeBase->input(), mNodeBase->output(), mBlockLength);
	}
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
