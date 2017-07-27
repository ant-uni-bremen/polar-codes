#include <polarcode/decoding/fastssc_avx2_char.h>
#include <polarcode/encoding/butterfly_avx2_packed.h>
#include <polarcode/polarcode.h>

#include <cstring> //for memset
#include <cmath>

namespace PolarCode {
namespace Decoding {

namespace FastSscAvx2 {

static inline __m256i hardDecode(__m256i x) {
	static const __m256i mask = _mm256_set1_epi8(-128);
	const __m256i sign = _mm256_and_si256(x, mask);// Get signs of LLRs
	const __m256i result = _mm256_srli_epi16(sign, 7);//Move them to LSB
	return result;
}

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

void Node::decode() {
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

void Node::setInput(Block<__m256i> *newInputBlock) {
	clearLlrBlock();
	mLlr = newInputBlock;
}

void Node::setOutput(Block<__m256i> *newOutputBlock) {
	clearBitBlock();
	mBit = newOutputBlock;
}

void Node::unsetBlockPointers() {
	mLlr = nullptr;
	mBit = nullptr;
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

// Preparers

Preparer::Preparer() {
}

Preparer::~Preparer() {
}

RepetitionPrep::RepetitionPrep(size_t codeLength)
	: mCodeLength(codeLength) {
}

RepetitionPrep::~RepetitionPrep() {
}

SpcPrep::SpcPrep(size_t codeLength)
	: mCodeLength(codeLength) {
}

SpcPrep::~SpcPrep() {
}

void Preparer::prepare(__m256i *x) {
}

void RepetitionPrep::prepare(__m256i *x) {
	memset(reinterpret_cast<char*>(x)+mCodeLength, 0, 32-mCodeLength);
}

void SpcPrep::prepare(__m256i *x) {
	memset(reinterpret_cast<char*>(x)+mCodeLength, 127, 32-mCodeLength);
}

// Constructors of nodes

RateRNode::RateRNode(std::set<unsigned> &frozenBits, Node *parent)
	: mParent(parent){
	xmDataPool = parent->pool();
	mBlockLength = parent->blockLength() / 2;

	std::set<unsigned> leftFrozenBits, rightFrozenBits;
	splitFrozenBits(frozenBits, mBlockLength, leftFrozenBits, rightFrozenBits);

	mLeft = createDecoder(leftFrozenBits, this);
	mRight = createDecoder(rightFrozenBits, this);

	mBlockLength *= 2;
	mVecCount = nBit2vecCount(mBlockLength);
}

RateZeroNode::RateZeroNode(Node *parent)
	: mParent(parent) {
	xmDataPool = parent->pool();
	mBlockLength = parent->blockLength();
	mVecCount = nBit2vecCount(mBlockLength);
}

RateOneNode::RateOneNode(Node *parent)
	: mParent(parent) {
	xmDataPool = parent->pool();
	mBlockLength = parent->blockLength();
	mVecCount = nBit2vecCount(mBlockLength);
}

RepetitionNode::RepetitionNode(Node *parent)
	: mParent(parent) {
	xmDataPool = parent->pool();
	mBlockLength = parent->blockLength();
	mVecCount = nBit2vecCount(mBlockLength);

	if(mBlockLength<32) {
		mPreparer = new RepetitionPrep(mBlockLength);
	} else {
		mPreparer = new Preparer();
	}
}

SpcNode::SpcNode(Node *parent)
	: mParent(parent) {
	xmDataPool = parent->pool();
	mBlockLength = parent->blockLength();
	mVecCount = nBit2vecCount(mBlockLength);

	if(mBlockLength<32) {
		mPreparer = new SpcPrep(mBlockLength);
	} else {
		mPreparer = new Preparer();
	}
}

// Destructors of nodes

RateRNode::~RateRNode() {
	delete mLeft;
	delete mRight;
}

RateZeroNode::~RateZeroNode() {
}

RateOneNode::~RateOneNode() {
}

RepetitionNode::~RepetitionNode() {
	delete mPreparer;
}

SpcNode::~SpcNode() {
	delete mPreparer;
}

// Decoders

void RateZeroNode::decode() {
	memset(mParent->output(), 0, mBlockLength);
}

void RateOneNode::decode() {
	__m256i* LlrIn = mParent->input();
	__m256i* BitsOut = mParent->output();

	for(unsigned i=0; i<mVecCount; i++) {
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
void RepetitionNode::decode() {
	__m256i* LlrIn = mParent->input();
	__m256i* BitsOut = mParent->output();

	__m256i LlrSum = _mm256_setzero_si256();

	mPreparer->prepare(LlrIn);

	// Accumulate vectors
	for(unsigned i=0; i<mVecCount; i++) {
		LlrSum = _mm256_adds_epi8(LlrSum, _mm256_load_si256(LlrIn+i));
	}

	// Get final sum and perform hard decision
	__m256i Bits = _mm256_set1_epi8(reduce_adds_epi8(LlrSum));
	Bits = hardDecode(Bits);

	// Prepare result
	mPreparer->prepare(&Bits);

	// Save bits
	for(unsigned i=0; i<mVecCount; i++) {
		_mm256_store_si256(BitsOut+i, Bits);
	}
}

void SpcNode::decode() {
	__m256i* LlrIn = mParent->input();
	__m256i* BitsOut = mParent->output();

	__m256i parVec = _mm256_setzero_si256();
	unsigned minIdx = 0;
	char testAbs, minAbs = 127;

	mPreparer->prepare(LlrIn);

	for(unsigned i=0; i<mVecCount; i++) {
		__m256i vecIn = _mm256_load_si256(LlrIn+i);

		__m256i sign = hardDecode(vecIn);
		_mm256_store_si256(BitsOut+i, sign);

		parVec = _mm256_xor_si256(parVec, sign);

		// Only search for minimum if there is a chance for smaller absolute value
		if(minAbs > 0) {
			__m256i abs = _mm256_abs_epi8(vecIn);
			unsigned vecMin = _mm256_minpos_epu8(abs, &testAbs);
			if(testAbs < minAbs) {
				minIdx = vecMin+i;
				minAbs = testAbs;
			}
		}
	}

	// Flip least reliable bit, if neccessary
	char parity = reduce_xor_si256(parVec);
	if(parity) {
		reinterpret_cast<char*>(BitsOut)[minIdx] ^= 1;
	}

}

void RateRNode::F_function_calc(__m256i &Left, __m256i &Right, __m256i *Out)
{
	__m256i absL = _mm256_abs_epi8(Left);
	__m256i absR = _mm256_abs_epi8(Right);
	__m256i minV = _mm256_min_epi8(absL, absR);//minimum of absolute values
	__m256i xorV = _mm256_xor_si256(Left, Right);//multiply signs
	__m256i outV = _mm256_sign_epi8(minV, xorV);//merge sign and value
	_mm256_store_si256(Out, outV);//save
}

void RateRNode::G_function_calc(__m256i &Left, __m256i &Right, __m256i &Bits, __m256i *Out)
{
	__m256i sum  = _mm256_adds_epi8(Right, Left);
	__m256i diff = _mm256_subs_epi8(Right, Left);
	__m256i bitmask = _mm256_slli_epi16(Bits, 7);
	__m256i result = _mm256_blendv_epi8(sum, diff, bitmask);
	_mm256_store_si256(Out, result);
}


void RateRNode::F_function(__m256i *LLRin, __m256i *LLRout) {
	__m256i Left, Right;
	if(mBlockLength < 32) {
		Left = _mm256_load_si256(LLRin);
		Right = _mm256_subVectorShift_epu8(Left, mBlockLength*8/2);
		F_function_calc(Left, Right, LLRout);
	} else {
		for(unsigned i=0; i<mVecCount; i++) {
			Left = _mm256_load_si256(LLRin+i);
			Right = _mm256_load_si256(LLRin+i+mVecCount);
			F_function_calc(Left, Right, LLRout+i);
		}
	}
}

void RateRNode::G_function(__m256i *LLRin, __m256i *LLRout, __m256i *BitsIn) {
	__m256i Left, Right, Bits;
	if(mBlockLength < 32) {
		Left = _mm256_load_si256(LLRin);
		Right = _mm256_subVectorShift_epu8(Left, mBlockLength*8/2);
		Bits = _mm256_load_si256(BitsIn);
		G_function_calc(Left, Right, Bits, LLRout);
	} else {
		for(unsigned i=0; i<mVecCount; i++) {
			Left = _mm256_load_si256(LLRin+i);
			Right = _mm256_load_si256(LLRin+i+mVecCount);
			Bits = _mm256_load_si256(BitsIn+i+mVecCount);
			G_function_calc(Left, Right, Bits, LLRout+i);
		}
	}

}

void RateRNode::Combine(__m256i *Left, __m256i *Right, __m256i *Out) {
	if(mBlockLength/2 < 32) {
		*Right = _mm256_subVectorBackShift_epu8(*Right, mBlockLength*8/2);
		_mm256_store_si256(Out, _mm256_or_si256(*Left, *Right));
	} else {
		// Copy left bits
		for(unsigned i=0; i<mVecCount; i++) {
			__m256i temp = _mm256_load_si256(Left+i);
			_mm256_store_si256(Out+i, temp);
		}
		// Copy right bits
		for(unsigned i=0; i<mVecCount; i++) {
			__m256i temp = _mm256_load_si256(Right+i);
			_mm256_store_si256(Out+mVecCount+i, temp);
		}
	}
	// Combine
	Encoding::ButterflyAvx2CharTransform(Out, mBlockLength, log2(mBlockLength)-1);
}

void RateRNode::decode() {
	__m256i *LlrIn = mParent->input();
	__m256i* BitsOut = mParent->output();

	Block<__m256i> *ChildLlr, *LeftBits, *RightBits;
	unsigned vecCount = nBit2vecCount(mBlockLength/2);
	ChildLlr = xmDataPool->allocate(vecCount);
	LeftBits = xmDataPool->allocate(vecCount);
	RightBits = xmDataPool->allocate(vecCount);

	F_function(LlrIn, ChildLlr->data);
	setInput(ChildLlr);
	setOutput(LeftBits);
	mLeft->decode();

	G_function(LlrIn, ChildLlr->data, LeftBits->data);
	setOutput(RightBits);
	mRight->decode();

	Combine(LeftBits->data, RightBits->data, BitsOut);

	xmDataPool->release(ChildLlr);
	xmDataPool->release(LeftBits);
	xmDataPool->release(RightBits);

	unsetBlockPointers();
}

// End of mass defining

Node* createDecoder(std::set<unsigned> frozenBits, Node* parent) {
	size_t blockLength = parent->blockLength();

	// Begin with the two most simple codes:
	if(frozenBits.size() == blockLength) {
		return new RateZeroNode(parent);
	}
	if(frozenBits.empty()) {
		return new RateOneNode(parent);
	}

	// Following are "one bit unlike the others" codes:
	if(frozenBits.size() == (blockLength-1)) {
		return new RepetitionNode(parent);
	}
	if(frozenBits.size() == 1) {
		return new SpcNode(parent);
	}

	// Fallback: No special code available, split into smaller subcodes
	return new RateRNode(frozenBits, parent);
}

size_t nBit2vecCount(size_t blockLength) {
	return (blockLength+31)/32;
}
}// namespace FastSscAvx2

FastSscAvx2Char::FastSscAvx2Char(size_t blockLength, const std::set<unsigned> &frozenBits) {
	mBlockLength = 0;// Hint in order to not delete objects that don't exist yet.
	initialize(blockLength, frozenBits);
}

FastSscAvx2Char::~FastSscAvx2Char() {
	clear();
}

void FastSscAvx2Char::clear() {
	delete mLlrContainer;
	delete mBitContainer;
	delete mOutputContainer;
	delete mRootNode;
	delete mNodeBase;
	delete mDataPool;
}

void FastSscAvx2Char::initialize(size_t blockLength, const std::set<unsigned> &frozenBits) {
	if(blockLength == mBlockLength && frozenBits == mFrozenBits) {
		return;
	} else {
		if(mBlockLength != 0) {
			clear();
		}
		mBlockLength = blockLength;
		mFrozenBits = frozenBits;
		mDataPool = new DataPool<__m256i, 32>();
		mNodeBase = new FastSscAvx2::Node(blockLength, mDataPool);
		mRootNode = FastSscAvx2::createDecoder(frozenBits, mNodeBase);
		mLlrContainer = new CharContainer(reinterpret_cast<char*>(mNodeBase->input()),  mBlockLength);
		mBitContainer = new CharContainer(reinterpret_cast<char*>(mNodeBase->output()), mBlockLength);
		mOutputContainer = new PackedContainer(mBlockLength);
	}
}

bool FastSscAvx2Char::decode() {
	mRootNode->decode();
	mOutputContainer->insertCharBits(reinterpret_cast<char*>(mNodeBase->output()));
	if(!mSystematic) {
		Encoding::Encoder* encoder = new Encoding::ButterflyAvx2Packed(mBlockLength);
		encoder->setCodeword(mOutputContainer->data());
		encoder->encode();
		encoder->getEncodedData(mOutputContainer->data());
		delete encoder;
	}
	return mErrorDetector->check(mOutputContainer->data(), mBlockLength/8);
}


}// namespace Decoding
}// namespace PolarCode
