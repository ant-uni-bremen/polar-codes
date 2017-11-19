#include <polarcode/encoding/recursive_avx2_packed.h>
#include <polarcode/encoding/butterfly_avx2.h>
#include <polarcode/polarcode.h>

namespace PolarCode {
namespace Encoding {

namespace RecursiveAvx2 {

Node::Node()
	: mBit(nullptr),
	  mBlockLength(0),
	  mVecCount(0) {
}

Node::Node(size_t blockLength)
	: mBlockLength(blockLength),
	  mVecCount(nBit2vecCount(blockLength)) {
	mBit = reinterpret_cast<__m256i*>(_mm_malloc(mVecCount*32, 32));
}

Node::~Node() {
	clearBlock();
}

void Node::encode(__m256i *Bits) {
	throw "This should not be called.";
}

void Node::clearBlock() {
	if(mBit != nullptr) {
		_mm_free(mBit);
		mBit = nullptr;
		mBlockLength = 0;
		mVecCount = 0;
	}
}

void Node::unsetBlockPointer() {
	mBit = nullptr;
}

size_t Node::blockLength() {
	return mBlockLength;
}

__m256i* Node::block() {
	return mBit;
}

// Constructors

RateOneNode::RateOneNode(Node *parent)
	: mParent(parent) {
}

RateZeroNode::RateZeroNode(Node *parent)
	: mParent(parent) {
	mBlockLength = parent->blockLength();
	mVecCount = nBit2vecCount(mBlockLength);
}

RepetitionNode::RepetitionNode(Node *parent)
	: mParent(parent) {
	mBlockLength = parent->blockLength();
	mVecCount = nBit2vecCount(mBlockLength);
}

SpcNode::SpcNode(Node *parent)
	: mParent(parent) {
	mBlockLength = parent->blockLength();
	mVecCount = nBit2vecCount(mBlockLength);
}

ShortButterflyNode::ShortButterflyNode(std::vector<unsigned> &frozenBits, Node *parent)
	: mParent(parent) {
	mBlockLength = parent->blockLength();
	mVecCount = nBit2vecCount(mBlockLength);
	mButterflyEncoder = new ButterflyAvx2Packed(mBlockLength, frozenBits);
}

RateRNode::RateRNode(std::vector<unsigned> &frozenBits, Node *parent)
	: Node(), mParent(parent) {
	mBlockLength = parent->blockLength()/2;
	mVecCount = nBit2vecCount(mBlockLength);
	mStage = __builtin_ctz(mBlockLength);

	std::vector<unsigned> leftFrozenBits, rightFrozenBits;
	splitFrozenBits(frozenBits, mBlockLength, leftFrozenBits, rightFrozenBits);

	mLeft = createEncoder(leftFrozenBits, this);
	mRight = createEncoder(rightFrozenBits, this);
}

// Destructors

RateOneNode::~RateOneNode() {
}

RateZeroNode::~RateZeroNode() {
}

RepetitionNode::~RepetitionNode() {
}

SpcNode::~SpcNode() {
}

ShortButterflyNode::~ShortButterflyNode() {
	delete mButterflyEncoder;
}

RateRNode::~RateRNode() {
	delete mLeft;
	delete mRight;
}

// Encoders

void RateOneNode::encode(__m256i *Bits) {
}

void RateZeroNode::encode(__m256i *Bits) {
	const __m256i zero = _mm256_setzero_si256();
	for(unsigned i=0; i<mVecCount; ++i) {
		_mm256_store_si256(Bits+i, zero);
	}
}

void RepetitionNode::encode(__m256i *Bits) {
	const char bit = reinterpret_cast<char*>(Bits+mVecCount-1)[31]&1;
	const __m256i vector = _mm256_set1_epi8(0-bit);
	for(unsigned i=0; i<mVecCount; ++i) {
		_mm256_store_si256(Bits+i, vector);
	}
}

void SpcNode::encode(__m256i *Bits) {
	//clear the parity bit
	unsigned char *firstByte = reinterpret_cast<unsigned char*>(Bits);
	*firstByte &= 0x7F;

	__m256i parVec = _mm256_setzero_si256();
	for(unsigned i=0; i<mVecCount; ++i) {
		parVec = _mm256_xor_si256(parVec, _mm256_load_si256(Bits+i));
	}
	unsigned char parity = reduce_xor_si256(parVec);
	//Above reduce operation stops at eight bit width
	//We need to reduce further to single bit
	parity ^= parity<<4;
	parity ^= parity<<2;
	parity ^= parity<<1;
	parity &= 0x80;//Clear unused bits
	*firstByte |= parity;//Insert parity information
}

void ShortButterflyNode::encode(__m256i *Bits) {
	mButterflyEncoder->setCodeword(Bits);
	mButterflyEncoder->clearFrozenBits();
	mButterflyEncoder->encode();
	mButterflyEncoder->getEncodedData(Bits);
}

void RateRNode::encode(__m256i *Bits) {
	__m256i *rightBits = Bits+mVecCount;

	mRight->encode(rightBits);

	ButterflyAvx2PackedTransform(Bits, mBlockLength*2, mStage);
	mLeft->encode(Bits);
	ButterflyAvx2PackedTransform(Bits, mBlockLength*2, mStage);
}

// End of nodes

Node* createEncoder(std::vector<unsigned> &frozenBits, Node *parent) {
	size_t blockLength = parent->blockLength();
	size_t frozenBitCount = frozenBits.size();

	//Straightforward codes
	if(frozenBitCount == 0) {
		return new RateOneNode(parent);
	}
	if(frozenBitCount == blockLength) {
		return new RateZeroNode(parent);
	}

	// "One bit unlike the others"
	if(frozenBitCount == (blockLength-1)) {
		return new RepetitionNode(parent);
	}
	if(frozenBitCount == 1) {
		return new SpcNode(parent);
	}

	//General codes
	if(blockLength == 256) {
		//No specializations for 128-bit codes
		return new ShortButterflyNode(frozenBits, parent);
	} else {
		//Divide code into half-length subcodes
		return new RateRNode(frozenBits, parent);
	}
}

size_t nBit2vecCount(size_t blockLength) {
	return (blockLength+255)/256;
}

}// namespace RecursiveAvx2

RecursiveAvx2Packed::RecursiveAvx2Packed(size_t blockLength, const std::vector<unsigned> &frozenBits) {
	if(blockLength < 256) {
		throw "AVX-2 recursive encoder does not exist for block length < 256 bits!";
		return;
	}
	mBlockLength = 0;
	initialize(blockLength, frozenBits);
}

RecursiveAvx2Packed::~RecursiveAvx2Packed() {
	clear();
}

void RecursiveAvx2Packed::clear() {
	delete mBitContainer; mBitContainer = nullptr;
	delete mRootNode; mRootNode = nullptr;
	delete mNodeBase; mNodeBase = nullptr;
}

void RecursiveAvx2Packed::initialize(size_t blockLength, const std::vector<unsigned> &frozenBits) {
	if(blockLength == mBlockLength && frozenBits == mFrozenBits) {
		return;
	} else {
		if(mBlockLength != 0) {
			clear();
		}
		mBlockLength = blockLength;
		mFrozenBits = frozenBits;
		mNodeBase = new RecursiveAvx2::Node(blockLength);
		mRootNode = RecursiveAvx2::createEncoder(mFrozenBits, mNodeBase);
		mBitContainer = new PackedContainer(reinterpret_cast<char*>(mNodeBase->block()), mBlockLength, mFrozenBits);
	}
}

void RecursiveAvx2Packed::encode() {
	mErrorDetector->generate(xmInputData, (mBlockLength - mFrozenBits.size()) / 8);
	mBitContainer->insertPackedInformationBits(xmInputData);

	mRootNode->encode(mNodeBase->block());
}
}// namespace Encoding
}// namespace PolarCode
