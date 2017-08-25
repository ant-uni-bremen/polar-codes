#include <polarcode/decoding/fastssc_avx2_char.h>
#include <polarcode/encoding/butterfly_avx2_packed.h>
#include <polarcode/polarcode.h>

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
	: mLlr(pool->allocate(nBit2vecCount(blockLength))),
	  mBit(pool->allocate(nBit2vecCount(blockLength))),
	  xmDataPool(pool),
	  mBlockLength(blockLength),
	  mVecCount(nBit2vecCount(blockLength)){
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

RateRNode::RateRNode(const std::vector<unsigned> &frozenBits, Node *parent)
	: mParent(parent){
	xmDataPool = parent->pool();
	mBlockLength = parent->blockLength() / 2;

	std::vector<unsigned> leftFrozenBits, rightFrozenBits;
	splitFrozenBits(frozenBits, mBlockLength, leftFrozenBits, rightFrozenBits);

	mLeft = createDecoder(leftFrozenBits, this, &leftDecoder);
	mRight = createDecoder(rightFrozenBits, this, &rightDecoder);

	mVecCount = nBit2vecCount(mBlockLength);

	ChildLlr = xmDataPool->allocate(mVecCount);
}

ShortRateRNode::ShortRateRNode(const std::vector<unsigned> &frozenBits, Node *parent)
	: RateRNode(frozenBits, parent),
	  LeftBits(xmDataPool->allocate(mVecCount)),
	  RightBits(xmDataPool->allocate(mVecCount)) {
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

// Decoders


void RateZeroDecode(__m256i *LlrIn, __m256i *BitsOut, const size_t blockLength) {
	memset(BitsOut, 0, blockLength);
}

void RateOneDecode(__m256i *LlrIn, __m256i *BitsOut, const size_t blockLength) {
	unsigned vecCount = (blockLength+31)/32;
	for(unsigned i=0; i<vecCount; i++) {
		__m256i vec = _mm256_load_si256(LlrIn+i);
		vec = hardDecode(vec);
		_mm256_store_si256(BitsOut+i, vec);//Save hard decision
	}
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

	// Get final sum and perform hard decision
	__m256i Bits = _mm256_set1_epi8(reduce_adds_epi8(LlrSum));
	Bits = hardDecode(Bits);

	// Prepare result
	RepetitionPrepare(&Bits, blockLength);

	// Save bits
	for(unsigned i=0; i<vecCount; i++) {
		_mm256_store_si256(BitsOut+i, Bits);
	}
}

void SpcDecode(__m256i *LlrIn, __m256i *BitsOut, const size_t blockLength) {
	unsigned vecCount = (blockLength+31)/32;
	__m256i parVec = _mm256_setzero_si256();
	unsigned minIdx = 0;
	char testAbs, minAbs = 127;

	SpcPrepare(LlrIn, blockLength);

	for(unsigned i=0; i<vecCount; i++) {
		__m256i vecIn = _mm256_load_si256(LlrIn+i);

		__m256i sign = hardDecode(vecIn);
		_mm256_store_si256(BitsOut+i, sign);

		parVec = _mm256_xor_si256(parVec, sign);

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
/*	if(frozenBitCount == (blockLength-1)) {
		*specialDecoder = &RepetitionDecode;
		return nullptr;
	}
	if(frozenBitCount == 1) {
		*specialDecoder = &SpcDecode;
		return nullptr;
	}*/

	// Fallback: No special code available, split into smaller subcodes
	if(blockLength <= 32) {
		*specialDecoder = nullptr;
		return new ShortRateRNode(frozenBits, parent);
	} else {
		*specialDecoder = nullptr;
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
		encoder->setCodeword(mBitContainer->data());
		encoder->encode();
		encoder->getEncodedData(mBitContainer->data());
		delete encoder;
	}
	mBitContainer->getPackedInformationBits(mOutputContainer);
	bool result = mErrorDetector->check(mOutputContainer, (mBlockLength-mFrozenBits.size()+7)/8);
	return result;
}


}// namespace Decoding
}// namespace PolarCode
