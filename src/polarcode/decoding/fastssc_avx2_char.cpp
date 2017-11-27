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
	: mLlr(nullptr)
	, mBit(nullptr)
	, xmDataPool(nullptr)
	, mBlockLength(0)
	, mVecCount(0)
{
}

Node::Node(Node *parent)
	: mLlr(nullptr)
	, mBit(nullptr)
	, xmDataPool(parent->pool())
	, mBlockLength(parent->blockLength())
	, mVecCount(nBit2cvecCount(mBlockLength))
{
}

Node::Node(size_t blockLength, datapool_t *pool)
	: mLlr(pool->allocate(nBit2cvecCount(blockLength)))
	, mBit(pool->allocate(nBit2cvecCount(blockLength)))
	, xmDataPool(pool)
	, mBlockLength(blockLength)
	, mVecCount(nBit2cvecCount(blockLength))
{
}

Node::~Node() {
	if(mLlr != nullptr) xmDataPool->release(mLlr);
	if(mBit != nullptr) xmDataPool->release(mBit);
}

void Node::decode(__m256i*, __m256i*) {
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

ShortNode::ShortNode(Node *parent)
	: Node(parent)
	, mLeftBits(xmDataPool->allocate(mVecCount))
	, mRightBits(xmDataPool->allocate(mVecCount))
{
}

ShortNode::~ShortNode() {
	xmDataPool->release(mLeftBits);
	xmDataPool->release(mRightBits);
}


// Constructors of nodes

RateRNode::RateRNode(const std::vector<unsigned> &frozenBits, Node *parent)
	: Node(parent){
	mBlockLength /= 2;
	mVecCount = nBit2cvecCount(mBlockLength);

	std::vector<unsigned> leftFrozenBits, rightFrozenBits;
	splitFrozenBits(frozenBits, mBlockLength, leftFrozenBits, rightFrozenBits);

	mLeft = createDecoder(leftFrozenBits, this);
	mRight = createDecoder(rightFrozenBits, this);

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

ShortROneNode::ShortROneNode(const std::vector<unsigned> &frozenBits, Node *parent)
	: ShortRateRNode(frozenBits, parent) {
}

ZeroRNode::ZeroRNode(const std::vector<unsigned> &frozenBits, Node *parent)
	: RateRNode(frozenBits, parent) {
}

ShortZeroRNode::ShortZeroRNode(const std::vector<unsigned> &frozenBits, Node *parent)
	: ShortRateRNode(frozenBits, parent) {
}

RateZeroDecoder::RateZeroDecoder(Node *parent)
	: Node(parent) {
}

RateOneDecoder::RateOneDecoder(Node *parent)
	: Node(parent) {
}

RepetitionDecoder::RepetitionDecoder(Node *parent)
	: Node(parent) {
}

ShortRepetitionDecoder::ShortRepetitionDecoder(Node *parent)
	: ShortNode(parent) {
}

SpcDecoder::SpcDecoder(Node *parent)
	: Node(parent) {
}

ShortSpcDecoder::ShortSpcDecoder(Node *parent)
	: ShortNode(parent) {
}

ZeroSpcDecoder::ZeroSpcDecoder(Node *parent)
	: Node(parent)
	, mSubBlockLength(mBlockLength/2)
	, mSubVecCount(nBit2cvecCount(mSubBlockLength))
{
}

ShortZeroSpcDecoder::ShortZeroSpcDecoder(Node *parent)
	: ShortNode(parent)
	, mSubBlockLength(mBlockLength/2) {
}

ShortZeroOneDecoder::ShortZeroOneDecoder(Node *parent)
	: ShortNode(parent)
	, mSubBlockLength(mBlockLength/2) {
}

// Destructors of nodes

RateRNode::~RateRNode() {
	if(mLeft)  delete mLeft;
	if(mRight) delete mRight;
	xmDataPool->release(ChildLlr);
}

ShortRateRNode::~ShortRateRNode() {
	xmDataPool->release(LeftBits);
	xmDataPool->release(RightBits);
}

ROneNode::~ROneNode() {
}

ShortROneNode::~ShortROneNode() {
}

ZeroRNode::~ZeroRNode() {
}

ShortZeroRNode::~ShortZeroRNode() {
}

RateZeroDecoder::~RateZeroDecoder() {
}

RateOneDecoder::~RateOneDecoder() {
}

RepetitionDecoder::~RepetitionDecoder() {
}

ShortRepetitionDecoder::~ShortRepetitionDecoder() {
}

SpcDecoder::~SpcDecoder() {
}

ShortSpcDecoder::~ShortSpcDecoder() {
}

ZeroSpcDecoder::~ZeroSpcDecoder() {
}

ShortZeroSpcDecoder::~ShortZeroSpcDecoder() {
}

ShortZeroOneDecoder::~ShortZeroOneDecoder() {
}


// Decoders

void RateZeroDecoder::decode(__m256i *, __m256i *BitsOut) {
	memset(BitsOut, 127, mBlockLength);
}

void RateOneDecoder::decode(__m256i *LlrIn, __m256i *BitsOut) {
	memcpy(BitsOut, LlrIn, mBlockLength);
}


/* WARNING: Saturation can lead to wrong results!

	127 + 127 + 127 + 127 + -128 results in -1 and thus wrong sign
	after step-by-step saturated addition.

	Conversion to epi16 will reduce throughput but circumvents that problem.
*/
void RepetitionDecoder::decode(__m256i *LlrIn, __m256i *BitsOut) {
	__m256i LlrSum = _mm256_setzero_si256();

	// Accumulate vectors
	for(unsigned i=0; i<mVecCount; i++) {
		LlrSum = _mm256_adds_epi8(LlrSum, _mm256_load_si256(LlrIn+i));
	}

	// Get final sum and save decoding result
	char Bits = reduce_adds_epi8(LlrSum);
	memset(BitsOut, Bits, mBlockLength);
}

void ShortRepetitionDecoder::decode(__m256i *LlrIn, __m256i *BitsOut) {
	RepetitionPrepare(LlrIn, mBlockLength);

	// Get sum and save decoding result
	char Bits = reduce_adds_epi8(_mm256_load_si256(LlrIn));
	memset(BitsOut, Bits, mBlockLength);
}

void SpcDecoder::decode(__m256i *LlrIn, __m256i *BitsOut) {
	__m256i parVec = _mm256_setzero_si256();
	unsigned minIdx = 0;
	char testAbs, minAbs = 127;

	memcpy(BitsOut, LlrIn, mBlockLength);

	for(unsigned i=0; i<mVecCount; i++) {
		__m256i vecIn = _mm256_load_si256(LlrIn+i);

		parVec = _mm256_xor_si256(parVec, vecIn);

		// Only search for minimum if there is a chance for smaller absolute value
		if(minAbs > 0) {
			__m256i abs = _mm256_abs_epi8(vecIn);
			unsigned vecMin = _mm256_minpos_epu8(abs, &testAbs);
			if(testAbs < minAbs) {
				minIdx = vecMin+i*32;
				minAbs = testAbs;
			}
		}
	}

	// Flip least reliable bit, if neccessary
	unsigned char parity = reduce_xor_si256(parVec) & 0x80;
	if(parity) {
		char* BitPtr = reinterpret_cast<char*>(BitsOut);
		BitPtr[minIdx] = -BitPtr[minIdx];
	}
}

void ShortSpcDecoder::decode(__m256i *LlrIn, __m256i *BitsOut) {
	SpcPrepare(LlrIn, mBlockLength);

	__m256i vecIn = _mm256_load_si256(LlrIn);
	_mm256_store_si256(BitsOut, vecIn);

	// Flip least reliable bit, if neccessary
	if(reduce_xor_si256(vecIn) & 0x80) {
		__m256i abs = _mm256_abs_epi8(vecIn);
		unsigned vecMin = _mm256_minpos_epu8(abs);
		unsigned char *BitPtr = reinterpret_cast<unsigned char*>(BitsOut);
		BitPtr[vecMin] = -BitPtr[vecMin];
	}
}

void ZeroSpcDecoder::decode(__m256i *LlrIn, __m256i *BitsOut) {
	unsigned char* BitPtr = reinterpret_cast<unsigned char*>(BitsOut);
	__m256i parVec = _mm256_setzero_si256();
	unsigned minIdx = 0;
	char testAbs, minAbs = 127;

	//Check parity equation
	for(unsigned i=0; i<mSubVecCount; i++) {
		//G-function with only frozen bits
		__m256i left = _mm256_load_si256(LlrIn+i);
		__m256i right = _mm256_load_si256(LlrIn+mSubVecCount+i);
		__m256i llr = _mm256_adds_epi8(left, right);

		//Store output
		_mm256_store_si256(BitsOut+i, llr);
		_mm256_store_si256(BitsOut+mSubVecCount+i, llr);

		//Update parity counter
		parVec = _mm256_xor_si256(parVec, llr);

		// Only search for minimum if there is a chance for smaller absolute value
		if(minAbs > 0) {
			__m256i abs = _mm256_abs_epi8(llr);
			unsigned vecMin = _mm256_minpos_epu8(abs, &testAbs);
			if(testAbs < minAbs) {
				minIdx = vecMin+i*32;
				minAbs = testAbs;
			}
		}
	}

	// Flip least reliable bit, if neccessary
	unsigned char parity = reduce_xor_si256(parVec) & 0x80;
	if(parity) {
		BitPtr[minIdx] = -BitPtr[minIdx];
		BitPtr[minIdx+mSubBlockLength] = -BitPtr[minIdx+mSubBlockLength];
	}
}

void ShortZeroSpcDecoder::decode(__m256i *LlrIn, __m256i *BitsOut) {
	unsigned char* BitPtr = reinterpret_cast<unsigned char*>(BitsOut);

	//G-function with only frozen bits
	__m256i left = _mm256_load_si256(LlrIn);
	__m256i right = _mm256_subVectorShiftBytes_epu8(left, mSubBlockLength);
	union {
		__m256i llr;
		char llr_c[32];
	};
	llr = _mm256_adds_epi8(left, right);

	//Set unused bits to SPC-neutral value of 127
	memset(llr_c+mSubBlockLength, 127, 32-mSubBlockLength);

	// Flip least reliable bit, if neccessary
	unsigned char parity = reduce_xor_si256(llr) & 0x80;
	if(parity) {
		__m256i abs = _mm256_abs_epi8(llr);
		unsigned vecMin = _mm256_minpos_epu8(abs);
		llr_c[vecMin] = -llr_c[vecMin];
	}

	//Generate output
	memcpy(BitPtr,                 llr_c, mSubBlockLength);
	memcpy(BitPtr+mSubBlockLength, llr_c, mSubBlockLength);
}

void ShortZeroOneDecoder::decode(__m256i *LlrIn, __m256i *BitsOut) {
	__m256i subLlrLeft, subLlrRight;

	G_function_0RShort(LlrIn, &subLlrLeft, mSubBlockLength);

	subLlrRight = _mm256_subVectorBackShiftBytes_epu8(subLlrLeft, mSubBlockLength);
	PrepareForShortOperation(&subLlrLeft, mSubBlockLength);
	_mm256_store_si256(BitsOut, _mm256_or_si256(subLlrLeft, subLlrRight));
}

void RateRNode::decode(__m256i *LlrIn, __m256i *BitsOut) {
	F_function(LlrIn, ChildLlr->data, mBlockLength);

	mLeft->decode(ChildLlr->data, BitsOut);

	G_function(LlrIn, ChildLlr->data, BitsOut, mBlockLength);

	mRight->decode(ChildLlr->data, BitsOut+mVecCount);

	CombineSoftInPlace(BitsOut, mVecCount);
}

void ShortRateRNode::decode(__m256i *LlrIn, __m256i *BitsOut) {
	F_function(LlrIn, ChildLlr->data, mBlockLength);

	mLeft->decode(ChildLlr->data, LeftBits->data);

	G_function(LlrIn, ChildLlr->data, LeftBits->data, mBlockLength);

	mRight->decode(ChildLlr->data, RightBits->data);

	CombineSoftBitsShort(LeftBits->data, RightBits->data, BitsOut, mBlockLength);
}

void ROneNode::decode(__m256i *LlrIn, __m256i *BitsOut) {
	F_function(LlrIn, ChildLlr->data, mBlockLength);

	mLeft->decode(ChildLlr->data, BitsOut);

	simplifiedRightRateOneDecode(LlrIn, BitsOut);
}

void ROneNode::simplifiedRightRateOneDecode(__m256i *LlrIn, __m256i *BitsOut) {
	for(unsigned i = 0; i < mVecCount; ++i) {
		__m256i Llr_l = _mm256_load_si256(LlrIn+i);
		__m256i Llr_r = _mm256_load_si256(LlrIn+i+mVecCount);
		__m256i Bits = _mm256_load_si256(BitsOut+i);
		__m256i Llr_o;

		G_function_calc(Llr_l, Llr_r, Bits, &Llr_o);
		/*nop*/ //Rate 1 decoder
		F_function_calc(Bits, Llr_o, BitsOut+i);//Combine left bit
		_mm256_store_si256(BitsOut+i+mVecCount, Llr_o);//Copy right bit
	}
}

void ShortROneNode::decode(__m256i *LlrIn, __m256i *BitsOut) {
	F_function(LlrIn, ChildLlr->data, mBlockLength);

	mLeft->decode(ChildLlr->data, BitsOut);

	simplifiedRightRateOneDecodeShort(LlrIn, BitsOut);
}

void ShortROneNode::simplifiedRightRateOneDecodeShort(__m256i *LlrIn, __m256i *BitsOut) {
	__m256i Bits = _mm256_load_si256(BitsOut);//Load left bits
	__m256i Llr_r_subcode = _mm256_setzero_si256();//Destination for right subcode

	G_function(LlrIn, &Llr_r_subcode, BitsOut, mBlockLength);//Get right child LLRs
	/*nop*/ //Rate 1 decoder
	__m256i Bits_r = _mm256_subVectorBackShiftBytes_epu8(Llr_r_subcode, mBlockLength);
	__m256i Bits_o;
	F_function_calc(Bits, Llr_r_subcode, &Bits_o);//Combine left bits
	memset(reinterpret_cast<char*>(&Bits_o)+mBlockLength, 0, mBlockLength);//Clear right bits
	Bits = _mm256_or_si256(Bits_o, Bits_r);//Merge bits into single vector
	_mm256_store_si256(BitsOut, Bits);//Save
}

void ZeroRNode::decode(__m256i *LlrIn, __m256i *BitsOut) {
	G_function_0R(LlrIn, ChildLlr->data, mBlockLength);

	mRight->decode(ChildLlr->data, BitsOut+mVecCount);

	Combine_0R(BitsOut, mBlockLength);
}

void ShortZeroRNode::decode(__m256i *LlrIn, __m256i *BitsOut) {
	G_function_0RShort(LlrIn, ChildLlr->data, mBlockLength);

	mRight->decode(ChildLlr->data, RightBits->data);

	Combine_0RShort(BitsOut, RightBits->data, mBlockLength);
}

// End of mass defining

Node* createDecoder(const std::vector<unsigned> &frozenBits, Node* parent) {
	size_t blockLength = parent->blockLength();
	size_t frozenBitCount = frozenBits.size();

	// Begin with the two most simple codes:
	if(frozenBitCount == blockLength) {
		return new RateZeroDecoder(parent);
	}
	if(frozenBitCount == 0) {
		return new RateOneDecoder(parent);
	}

	// Following are "one bit unlike the others" codes:
	if(frozenBitCount == (blockLength-1)) {
		if(blockLength <= 32) {
			return new ShortRepetitionDecoder(parent);
		} else {
			return new RepetitionDecoder(parent);
		}
	}
	if(frozenBitCount == 1) {
		if(blockLength <= 32) {
			return new ShortSpcDecoder(parent);
		} else {
			return new SpcDecoder(parent);
		}
	}

	//Precalculate subcodes to find special child node combinations
	std::vector<unsigned> leftFrozenBits, rightFrozenBits;
	splitFrozenBits(frozenBits, blockLength/2, leftFrozenBits, rightFrozenBits);

	if(blockLength <= 32) {
		if(leftFrozenBits.size() == blockLength/2 && rightFrozenBits.size() == 0) {
			return new ShortZeroOneDecoder(parent);
		}

		if(leftFrozenBits.size() == blockLength/2 && rightFrozenBits.size() == 1) {
			return new ShortZeroSpcDecoder(parent);
		}

		// Fallback: No special decoder available
		if(rightFrozenBits.size() == 0) {
			return new ShortROneNode(frozenBits, parent);
		}

		if(leftFrozenBits.size() == blockLength/2) {
			return new ShortZeroRNode(frozenBits, parent);
		}

		return new ShortRateRNode(frozenBits, parent);
	} else {
		if(leftFrozenBits.size() == blockLength/2 && rightFrozenBits.size() == 1) {
			return new ZeroSpcDecoder(parent);
		}
		//Minor optimization:

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
	mSoftOutput = true;
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
	mRootNode = FastSscAvx2::createDecoder(frozenBits, mNodeBase);
	mLlrContainer = new CharContainer(reinterpret_cast<char*>(mNodeBase->input()),  mBlockLength);
	mBitContainer = new CharContainer(reinterpret_cast<char*>(mNodeBase->output()), mBlockLength);
	mLlrContainer->setFrozenBits(mFrozenBits);
	mBitContainer->setFrozenBits(mFrozenBits);
	mOutputContainer = new unsigned char[(mBlockLength-frozenBits.size()+7)/8];
}

bool FastSscAvx2Char::decode() {
	mRootNode->decode(mNodeBase->input(), mNodeBase->output());
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
