#include <polarcode/decoding/fastssc_avx2_char.h>
#include <polarcode/polarcode.h>

#include <cstring> //for memset

namespace PolarCode {
namespace Decoding {

namespace FastSscAvx2 {

Node::Node() {
}

Node::Node(size_t blockLength, datapool_t *pool)
	: mLlr(pool->allocate(nBit2vecCount(blockLength))),
	  mBit(pool->allocate(nBit2vecCount(blockLength))),
	  xmDataPool(pool),
	  mBlockLength(blockLength) {
}

Node::~Node() {
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

RateRNode::RateRNode(std::set<unsigned> &frozenBits, Node *parent) {
	mBlockLength = parent->blockLength() / 2;

	std::set<unsigned> leftFrozenBits, rightFrozenBits;
	splitFrozenBits(frozenBits, mBlockLength, leftFrozenBits, rightFrozenBits);

	mLeft = createDecoder(leftFrozenBits, this);
	mRight = createDecoder(rightFrozenBits, this);

	mBlockLength *= 2;
}

RateZeroNode::RateZeroNode(Node *parent)
	: mParent(parent) {
	xmDataPool = parent->pool();
	mBlockLength = parent->blockLength();
}

RateOneNode::RateOneNode(Node *parent)
	: mParent(parent) {
	xmDataPool = parent->pool();
	mBlockLength = parent->blockLength();
}

RepetitionNode::RepetitionNode(Node *parent)
	: mParent(parent) {
	xmDataPool = parent->pool();
	mBlockLength = parent->blockLength();
}

SpcNode::SpcNode(Node *parent)
	: mParent(parent) {
	xmDataPool = parent->pool();
	mBlockLength = parent->blockLength();
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
}

SpcNode::~SpcNode() {
}

// Decoders

static inline __m256i hardDecode(__m256i x) {
	static const __m256i mask = _mm256_set1_epi8(-128);
	const __m256i sign = _mm256_and_si256(x, mask);// Get signs of LLRs
	const __m256i result = _mm256_srli_epi16(sign, 7);//Move them to LSB
	return result;
}

void RateZeroNode::decode() {
	memset(mParent->output(), 0, mBlockLength);
}

void RateOneNode::decode() {
	__m256i* LlrIn = mParent->input();
	__m256i* BitsOut = mParent->output();

	for(unsigned i=0; i<mBlockLength; i+=32) {
		__m256i vec = _mm256_load_si256(LlrIn+i);
		vec = hardDecode(vec);
		_mm256_store_si256(BitsOut+i, vec);//Save hard decision
	}

}


/* WARNING: Saturation can lead to wrong results!

	127 + 127 + 127 + 127 - 128 results in -1
	after step-by-step saturated addition.

	Conversion to epi16 will reduce throughput but circumvents that problem.
*/
void RepetitionNode::decode() {
	__m256i* LlrIn = mParent->input();
	__m256i* BitsOut = mParent->output();

	__m256i LlrSum = _mm256_setzero_si256();

	// Accumulate vectors
	for(unsigned i=0; i<mBlockLength; i+=32) {
		LlrSum = _mm256_adds_epi8(LlrSum, _mm256_load_si256(LlrIn+i));
	}

	// Get final sum and perform hard decision
	__m256i Bits = _mm256_set1_epi8(reduce_adds_epi8(LlrSum));
	Bits = hardDecode(Bits);

	// Save bits
	for(unsigned i=0; i<mBlockLength; i+=32) {
		_mm256_store_si256(BitsOut+i, Bits);
	}
}

void SpcNode::decode() {

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
	mBitContainer = new CharContainer(mBlockLength);
}

}// namespace Decoding
}// namespace PolarCode
