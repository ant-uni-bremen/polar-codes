#include <polarcode/decoding/fastssc_avx2_char.h>
#include <polarcode/encoding/butterfly_avx2_packed.h>
#include <polarcode/polarcode.h>

#include <cstring> //for memset
#include <cmath>

namespace PolarCode {
namespace Decoding {

namespace FastSscAvx2 {

__m256i hardDecode(__m256i x) {
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


void RepetitionPrepare(__m256i* x, const size_t codeLength) {
	if(codeLength >= 32) return;
	memset(reinterpret_cast<char*>(x)+codeLength, 0, 32-codeLength);
}

void SpcPrepare(__m256i *x, const size_t codeLength) {
	if(codeLength >= 32) return;
	memset(reinterpret_cast<char*>(x)+codeLength, 127, 32-codeLength);
}

// Constructors of nodes

RateRNode::RateRNode(std::vector<unsigned> &frozenBits, Node *parent)
	: mParent(parent){
	xmDataPool = parent->pool();
	mBlockLength = parent->blockLength() / 2;

	std::vector<unsigned> leftFrozenBits, rightFrozenBits;
	splitFrozenBits(frozenBits, mBlockLength, leftFrozenBits, rightFrozenBits);

	mLeft = createDecoder(leftFrozenBits, this, &leftDecoder);
	mRight = createDecoder(rightFrozenBits, this, &rightDecoder);

//	mBlockLength *= 2;
	mVecCount = nBit2vecCount(mBlockLength);

	ChildLlr = xmDataPool->allocate(mVecCount);
}

ShortRateRNode::ShortRateRNode(std::vector<unsigned> &frozenBits, Node *parent)
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

//const __m256i absCorrector = _mm256_set1_epi8(-127);
void F_function_calc(__m256i &Left, __m256i &Right, __m256i *Out)
{
	__m256i absL = _mm256_abs_epi8(/*_mm256_max_epi8(*/Left/*, absCorrector)*/);
	__m256i absR = _mm256_abs_epi8(/*_mm256_max_epi8(*/Right/*, absCorrector)*/);
	__m256i minV = _mm256_min_epi8(absL, absR);//minimum of absolute values
	__m256i xorV = _mm256_xor_si256(Left, Right);//multiply signs
	xorV = _mm256_or_si256(xorV, _mm256_set1_epi8(1));//prevent zero as sign value
	__m256i outV = _mm256_sign_epi8(minV, xorV);//merge sign and value
//	outV = _mm256_max_epi8(outV, absCorrector);
	_mm256_store_si256(Out, outV);//save
}

void G_function_calc(__m256i &Left, __m256i &Right, __m256i &Bits, __m256i *Out)
{
	__m256i sum  = _mm256_adds_epi8(Right, Left);
	__m256i diff = _mm256_subs_epi8(Right, Left);
	__m256i bitmask = _mm256_slli_epi16(Bits, 7);
	__m256i result = _mm256_blendv_epi8(sum, diff, bitmask);
//	result = _mm256_max_epi8(result, absCorrector);
	_mm256_store_si256(Out, result);
}


void F_function(__m256i *LLRin, __m256i *LLRout, unsigned subBlockLength) {
	__m256i Left, Right;
	if(subBlockLength < 32) {
		Left = _mm256_load_si256(LLRin);
		Right = _mm256_subVectorShift_epu8(Left, subBlockLength*8);
		F_function_calc(Left, Right, LLRout);
	} else {
		unsigned vecCount = nBit2vecCount(subBlockLength);
		for(unsigned i=0; i<vecCount; i++) {
			Left = _mm256_load_si256(LLRin+i);
			Right = _mm256_load_si256(LLRin+i+vecCount);
			F_function_calc(Left, Right, LLRout+i);
		}
	}
}

void G_function(__m256i *LLRin, __m256i *LLRout, __m256i *BitsIn, unsigned subBlockLength) {
	__m256i Left, Right, Bits;
	if(subBlockLength < 32) {
		Left = _mm256_load_si256(LLRin);
		Right = _mm256_subVectorShift_epu8(Left, subBlockLength*8);
		Bits = _mm256_load_si256(BitsIn);
		G_function_calc(Left, Right, Bits, LLRout);
	} else {
		unsigned vecCount = nBit2vecCount(subBlockLength);
		for(unsigned i=0; i<vecCount; i++) {
			Left = _mm256_load_si256(LLRin+i);
			Right = _mm256_load_si256(LLRin+i+vecCount);
			Bits = _mm256_load_si256(BitsIn+i);
			G_function_calc(Left, Right, Bits, LLRout+i);
		}
	}
}

void CombineShortBits(__m256i *Left, __m256i *Right, __m256i *Out, const unsigned subBlockLength) {
	*Left = _mm256_xor_si256(*Left, *Right);
	memset(reinterpret_cast<char*>(Left)+subBlockLength, 0, subBlockLength);
	*Right = _mm256_subVectorBackShift_epu8(*Right, subBlockLength*8);
	__m256i result = _mm256_or_si256(*Left, *Right);
	_mm256_store_si256(Out, result);
}

void Combine(__m256i *Bits, const unsigned vecCount) {
	for(unsigned i=0; i<vecCount; i++) {
		__m256i tempL = _mm256_load_si256(Bits+i);
		__m256i tempR = _mm256_load_si256(Bits+vecCount+i);
		tempL = _mm256_xor_si256(tempL, tempR);
		_mm256_store_si256(Bits+i, tempL);
//		_mm256_store_si256(Bits+vecCount+i, tempR);
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

// End of mass defining

Node* createDecoder(std::vector<unsigned> frozenBits, Node* parent, void (**specialDecoder)(__m256i *, __m256i *, size_t)) {
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

size_t nBit2vecCount(size_t blockLength) {
	return (blockLength+31)/32;
}
}// namespace FastSscAvx2

FastSscAvx2Char::FastSscAvx2Char(size_t blockLength, const std::vector<unsigned> &frozenBits) {
	mBlockLength = 0;// Hint in order to not delete objects that don't exist yet.
	initialize(blockLength, frozenBits);
}

FastSscAvx2Char::~FastSscAvx2Char() {
	clear();
}

void FastSscAvx2Char::clear() {
	delete mLlrContainer;
	delete mBitContainer;
	delete [] mOutputContainer;
	if(mRootNode) delete mRootNode;
	delete mNodeBase;
	delete mDataPool;
}

void FastSscAvx2Char::initialize(size_t blockLength, const std::vector<unsigned> &frozenBits) {
	if(blockLength == mBlockLength && frozenBits == mFrozenBits) {
		return;
	} else {
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
