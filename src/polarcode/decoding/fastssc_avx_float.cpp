#include <polarcode/decoding/fastssc_avx_float.h>
#include <polarcode/encoding/butterfly_avx2_packed.h>
#include <polarcode/polarcode.h>

#include <string>
#include <iostream>

#include <cstring> //for memset
#include <cmath>

namespace PolarCode {
namespace Decoding {

namespace FastSscAvx {


Node::Node()
	: mLlr(nullptr),
	  mBit(nullptr),
	  xmDataPool(nullptr),
	  mBlockLength(0) {
}

Node::Node(size_t blockLength, datapool_t *pool)
	: mLlr(pool->allocate(blockLength)),
	  mBit(pool->allocate(blockLength)),
	  xmDataPool(pool),
	  mBlockLength(blockLength) {
}

Node::~Node() {
	clearBlocks();
}

void Node::decode(float *LlrIn, float *BitsOut) {
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

float* Node::input() {
	return mLlr->data;
}

float* Node::output() {
	return mBit->data;
}


// Constructors of nodes

RateRNode::RateRNode(const std::vector<unsigned> &frozenBits, Node *parent, ChildCreationFlags flags)
	: mParent(parent){
	xmDataPool = parent->pool();
	mBlockLength = parent->blockLength() / 2;

	std::vector<unsigned> leftFrozenBits, rightFrozenBits;
	splitFrozenBits(frozenBits, mBlockLength, leftFrozenBits, rightFrozenBits);

	if(flags & NO_LEFT) {
		mLeft = nullptr;
	} else {
		mLeft = createDecoder(leftFrozenBits, this, &leftDecoder);
	}

	if(flags & NO_RIGHT) {
		mRight = nullptr;
	} else {
		mRight = createDecoder(rightFrozenBits, this, &rightDecoder);
	}

	ChildLlr = xmDataPool->allocate(mBlockLength);
}

ShortRateRNode::ShortRateRNode(const std::vector<unsigned> &frozenBits, Node *parent)
	: RateRNode(frozenBits, parent),
	  LeftBits(xmDataPool->allocate(8)),
	  RightBits(xmDataPool->allocate(8)) {
}

ROneNode::ROneNode(const std::vector<unsigned> &frozenBits, Node *parent)
	: RateRNode(frozenBits, parent, NO_RIGHT) {
}

ZeroRNode::ZeroRNode(const std::vector<unsigned> &frozenBits, Node *parent)
	: RateRNode(frozenBits, parent, NO_LEFT) {
}

// Destructors of nodes

RateRNode::~RateRNode() {
	if(mLeft) delete mLeft;
	if(mRight) delete mRight;
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

void memFloatFill(float *dst, float value, const size_t blockLength) {
	if(blockLength>=8) {
		const __m256 vec = _mm256_set1_ps(value);
		for(unsigned i=0; i<blockLength; i+=8) {
			_mm256_store_ps(dst+i, vec);
		}
	} else {
		for(unsigned i=0; i<blockLength; i++) {
			dst[i] = value;
		}
	}
}

void RateZeroDecode(float*, float *BitsOut, const size_t blockLength) {
	memFloatFill(BitsOut, INFINITY, blockLength);
}

void RateOneDecode(float *LlrIn, float *BitsOut, const size_t blockLength) {
	memcpy(BitsOut, LlrIn, blockLength*4);
}


void RepetitionDecode(float *LlrIn, float *BitsOut, const size_t blockLength) {
	__m256 LlrSum = _mm256_setzero_ps();

	RepetitionPrepare(LlrIn, blockLength);

	// Accumulate vectors
	for(unsigned i=0; i<blockLength; i+=8) {
		LlrSum = _mm256_add_ps(LlrSum, _mm256_load_ps(LlrIn+i));
	}

	// Get final sum and save decoding result
	float Bits = reduce_add_ps(LlrSum);
	memFloatFill(BitsOut, Bits, blockLength);
}

void SpcDecode(float *LlrIn, float *BitsOut, const size_t blockLength) {
	static const __m256 sgnMask = _mm256_set1_ps(-0.0);
	__m256 parVec = _mm256_setzero_ps();
	unsigned minIdx = 0;
	float testAbs, minAbs = INFINITY;

	SpcPrepare(LlrIn, blockLength);

	for(unsigned i=0; i<blockLength; i+=8) {
		__m256 vecIn = _mm256_load_ps(LlrIn+i);
		_mm256_store_ps(BitsOut+i, vecIn);

		parVec = _mm256_xor_ps(parVec, vecIn);

		__m256 abs = _mm256_andnot_ps(sgnMask, vecIn);
		unsigned vecMin = _mm256_minidx_ps(abs, &testAbs);
		if(testAbs < minAbs) {
			minIdx = vecMin+i;
			minAbs = testAbs;
		}
	}

	// Flip least reliable bit, if neccessary
	union {
		float fParity;
		unsigned int iParity;
	};
	fParity = reduce_xor_ps(parVec);
	reinterpret_cast<unsigned int*>(BitsOut)[minIdx] ^= (iParity & 0x80000000);
}

void ZeroSpcDecode(float *LlrIn, float *BitsOut, const size_t blockLength) {
	static const __m256 sgnMask = _mm256_set1_ps(-0.0);
	const size_t subBlockLength = blockLength/2;
	__m256 parVec = _mm256_setzero_ps();
	unsigned minIdx = 0;
	float testAbs, minAbs = INFINITY;

	//Check parity equation
	for(unsigned i=0; i<subBlockLength; i+=8) {
		//G-function with only frozen bits
		__m256 left = _mm256_load_ps(LlrIn+i);
		__m256 right = _mm256_load_ps(LlrIn+subBlockLength+i);
		__m256 llr = _mm256_add_ps(left, right);

		//Save output
		_mm256_store_ps(BitsOut+i, right);
		_mm256_store_ps(BitsOut+subBlockLength+i, right);

		//Update parity counter
		parVec = _mm256_xor_ps(parVec, llr);

		// Only search for minimum if there is a chance for smaller absolute value
		if(minAbs > 0) {
			__m256 abs = _mm256_andnot_ps(sgnMask, llr);
			unsigned vecMin = _mm256_minidx_ps(abs, &testAbs);
			if(testAbs < minAbs) {
				minIdx = vecMin+i;
				minAbs = testAbs;
			}
		}
	}

	// Flip least reliable bit, if neccessary
	union {
		float fParity;
		unsigned int iParity;
	};
	fParity = reduce_xor_ps(parVec);
	iParity &= 0x80000000;
	reinterpret_cast<unsigned int*>(BitsOut)[minIdx] ^= iParity;
	reinterpret_cast<unsigned int*>(BitsOut+subBlockLength)[minIdx] ^= iParity;
}

void simplifiedRightRateOneDecode(float *LlrIn, float *BitsOut, const size_t blockLength) {
	for(unsigned i = 0; i < blockLength; i+=8) {
		__m256 Llr_l = _mm256_load_ps(LlrIn+i);
		__m256 Llr_r = _mm256_load_ps(LlrIn+blockLength+i);
		__m256 Bits = _mm256_load_ps(BitsOut+i);
		__m256 HBits = hardDecode(Bits);

		__m256 Llr_o = _mm256_xor_ps(Llr_l, HBits);//G-function
		Llr_o = _mm256_add_ps(Llr_o, Llr_r);//G-function
		/*nop*/ //Rate 1 decoder
		_mm256_store_ps(BitsOut+blockLength+i, Llr_o);//Right bit
		F_function_calc(Bits, Llr_o, BitsOut+i);//Combine left bit
	}
}


void RateRNode::decode(float *LlrIn, float *BitsOut) {
	F_function(LlrIn, ChildLlr->data, mBlockLength);

	if(mLeft) {
		mLeft->decode(ChildLlr->data, BitsOut);
	} else {
		leftDecoder(ChildLlr->data, BitsOut, mBlockLength);
	}

	G_function(LlrIn, ChildLlr->data, BitsOut, mBlockLength);

	if(mRight) {
		mRight->decode(ChildLlr->data, BitsOut+mBlockLength);
	} else {
		rightDecoder(ChildLlr->data, BitsOut+mBlockLength, mBlockLength);
	}

	CombineSoft(BitsOut, mBlockLength);
}

void ShortRateRNode::decode(float *LlrIn, float *BitsOut) {
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

	CombineSoftBitsShort(LeftBits->data, RightBits->data, BitsOut, mBlockLength);
}

void ROneNode::decode(float *LlrIn, float *BitsOut) {
	F_function(LlrIn, ChildLlr->data, mBlockLength);

	if(mLeft) {
		mLeft->decode(ChildLlr->data, BitsOut);
	} else {
		leftDecoder(ChildLlr->data, BitsOut, mBlockLength);
	}

	simplifiedRightRateOneDecode(LlrIn, BitsOut, mBlockLength);
}

void ZeroRNode::decode(float *LlrIn, float *BitsOut) {
	G_function_0R(LlrIn, ChildLlr->data, mBlockLength);

	if(mRight) {
		mRight->decode(ChildLlr->data, BitsOut+mBlockLength);
	} else {
		rightDecoder(ChildLlr->data, BitsOut+mBlockLength, mBlockLength);
	}

	Combine_0R(BitsOut, mBlockLength);
}

// End of mass defining

Node* createDecoder(const std::vector<unsigned> &frozenBits, Node* parent, void (**specialDecoder)(float *, float *, size_t)) {
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
	if(blockLength <= 8) {
		*specialDecoder = nullptr;
		return new ShortRateRNode(frozenBits, parent);
	} else {
		std::vector<unsigned> leftFrozenBits, rightFrozenBits;
		splitFrozenBits(frozenBits, blockLength/2, leftFrozenBits, rightFrozenBits);

		//Last case of optimization:
		//Common child node combinations
		if(leftFrozenBits.size() == blockLength/2 && rightFrozenBits.size() == 1) {
			*specialDecoder = &ZeroSpcDecode;
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

}// namespace FastSscAvx

FastSscAvxFloat::FastSscAvxFloat(size_t blockLength, const std::vector<unsigned> &frozenBits) {
	initialize(blockLength, frozenBits);
}

FastSscAvxFloat::~FastSscAvxFloat() {
	clear();
}

void FastSscAvxFloat::clear() {
	if(mRootNode) delete mRootNode;
	delete mNodeBase;
	delete mDataPool;
}

void FastSscAvxFloat::initialize(size_t blockLength, const std::vector<unsigned> &frozenBits) {
	if(blockLength == mBlockLength && frozenBits == mFrozenBits) {
		return;
	}
	if(mBlockLength != 0) {
		clear();
	}
	mBlockLength = blockLength;
	//mFrozenBits = frozenBits;
	mFrozenBits.assign(frozenBits.begin(), frozenBits.end());
	mDataPool = new DataPool<float, 32>();
	mNodeBase = new FastSscAvx::Node(mBlockLength, mDataPool);
//	std::cout << "Create decoder of length " << mBlockLength << std::endl;
	mRootNode = FastSscAvx::createDecoder(mFrozenBits, mNodeBase, &mSpecialDecoder);
	mLlrContainer = new FloatContainer(mNodeBase->input(),  mBlockLength);
	mBitContainer = new FloatContainer(mNodeBase->output(), mBlockLength);
	mLlrContainer->setFrozenBits(mFrozenBits);
	mBitContainer->setFrozenBits(mFrozenBits);
	mOutputContainer = new unsigned char[(mBlockLength-mFrozenBits.size()+7)/8];
}

bool FastSscAvxFloat::decode() {
	if(mRootNode) {
		mRootNode->decode(mNodeBase->input(), mNodeBase->output());
	} else {
		mSpecialDecoder(mNodeBase->input(), mNodeBase->output(), mBlockLength);
	}
	if(!mSystematic) {
		Encoding::Encoder* encoder = new Encoding::ButterflyAvx2Packed(mBlockLength);
		encoder->setSystematic(false);
		encoder->setCodeword(dynamic_cast<FloatContainer*>(mBitContainer)->data());
		encoder->encode();
		encoder->getEncodedData(dynamic_cast<FloatContainer*>(mBitContainer)->data());
		delete encoder;
	}
	mBitContainer->getPackedInformationBits(mOutputContainer);
	bool result = mErrorDetector->check(mOutputContainer, (mBlockLength-mFrozenBits.size()+7)/8);
	return result;
}


}// namespace Decoding
}// namespace PolarCode
