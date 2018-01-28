#include <polarcode/decoding/scl_avx_float.h>
#include <polarcode/polarcode.h>
#include <polarcode/arrayfuncs.h>
#include <polarcode/encoding/butterfly_avx2_packed.h>
#include <cmath>

namespace PolarCode {
namespace Decoding {

namespace SclAvx {

PathList::PathList(){}

PathList::PathList(size_t listSize, size_t stageCount, datapool_t *dataPool)
	: mPathLimit(listSize),
	  mPathCount(0),
	  mNextPathCount(0),
	  mStageCount(stageCount),
	  xmDataPool(dataPool) {
	mLlrTree.resize(listSize);
	mBitTree.resize(listSize);
	mLeftBitTree.resize(listSize);
	mMetric.assign(listSize, 0);
//	mCorrectedNodeIds.resize(listSize);
	mNextLlrTree.resize(listSize);
	mNextBitTree.resize(listSize);
	mNextLeftBitTree.resize(listSize);
	mNextMetric.assign(listSize, 0);
//	mNextCorrectedNodeIds.resize(listSize);
	for(unsigned i=0; i<mPathLimit; ++i) {
		mLlrTree[i].resize(stageCount);
		mBitTree[i].resize(stageCount);
		mLeftBitTree[i].resize(stageCount);
		mNextLlrTree[i].resize(stageCount);
		mNextBitTree[i].resize(stageCount);
		mNextLeftBitTree[i].resize(stageCount);
	}
}

PathList::~PathList() {
	clear();
}

void PathList::clear() {
	for(unsigned path=0; path < mPathCount; ++path) {
		xmDataPool->release(mLlrTree[path][mStageCount-1]);
		xmDataPool->release(mBitTree[path][mStageCount-1]);
		xmDataPool->release(mLeftBitTree[path][mStageCount-1]);
	}
	mPathCount = 0;
}

void PathList::duplicatePath(unsigned destination, unsigned source, unsigned stage) {
	for(unsigned i = stage; i<mStageCount; ++i) {
		mNextLlrTree[destination][i] = xmDataPool->lazyDuplicate(mLlrTree[source][i]);
		mNextBitTree[destination][i] = xmDataPool->lazyDuplicate(mBitTree[source][i]);
		mNextLeftBitTree[destination][i] = xmDataPool->lazyDuplicate(mLeftBitTree[source][i]);
	}
}

void PathList::getWriteAccessToLlr(unsigned path, unsigned stage) {
	xmDataPool->prepareForWrite(mLlrTree[path][stage]);
}

void PathList::getWriteAccessToBit(unsigned path, unsigned stage) {
	xmDataPool->prepareForWrite(mBitTree[path][stage]);
}

void PathList::getWriteAccessToNextBit(unsigned path, unsigned stage) {
	xmDataPool->prepareForWrite(mNextBitTree[path][stage]);
}

void PathList::clearOldPaths(unsigned stage) {
	for(unsigned path = 0; path < mPathCount; ++path) {
		for(unsigned i = stage; i < mStageCount; ++i) {
			xmDataPool->release(mLlrTree[path][i]);
			xmDataPool->release(mBitTree[path][i]);
			xmDataPool->release(mLeftBitTree[path][i]);
		}
	}
}

void PathList::switchToNext() {
	std::swap(mLlrTree, mNextLlrTree);
	std::swap(mBitTree, mNextBitTree);
	std::swap(mLeftBitTree, mNextLeftBitTree);
	std::swap(mMetric, mNextMetric);
//	std::swap(mCorrectedNodeIds, mNextCorrectedNodeIds);
	mPathCount = mNextPathCount;
}

void PathList::setFirstPath(void *pLlr) {
	mPathCount = 1;
	allocateStage(mStageCount-1);

	memcpy(Llr(0, mStageCount-1), pLlr, 2<<mStageCount /* 4*bitCount = 4*(1<<stage) =  4*(1<<(stageCount-1)) = 2*(1<<stageCount) = 2<<stageCount */);
}

void PathList::allocateStage(unsigned stage) {
	unsigned expandedBitCount = nBit2fCount(1<<stage);
	for(unsigned path = 0; path < mPathCount; ++path) {
		mLlrTree[path][stage] = xmDataPool->allocate(expandedBitCount);
		mBitTree[path][stage] = xmDataPool->allocate(expandedBitCount);
		mLeftBitTree[path][stage] = xmDataPool->allocate(expandedBitCount);
	}
}

void PathList::clearStage(unsigned stage) {
	for(unsigned path = 0; path < mPathCount; ++path) {
		xmDataPool->release(mLlrTree[path][stage]);
		xmDataPool->release(mBitTree[path][stage]);
		xmDataPool->release(mLeftBitTree[path][stage]);
	}
}

float* PathList::Llr(unsigned path, unsigned stage) {
	return mLlrTree[path][stage]->data;
}

float* PathList::Bit(unsigned path, unsigned stage) {
	return mBitTree[path][stage]->data;
}

float* PathList::LeftBit(unsigned path, unsigned stage) {
	return mLeftBitTree[path][stage]->data;
}

void PathList::prepareRightDecoding(unsigned stage) {
	for(unsigned path = 0; path < mPathCount; ++path) {
		std::swap(mBitTree[path][stage], mLeftBitTree[path][stage]);
		xmDataPool->prepareForWrite(mLlrTree[path][stage]);
	}
}

float* PathList::NextLlr(unsigned path, unsigned stage) {
	return mNextLlrTree[path][stage]->data;
}

float* PathList::NextBit(unsigned path, unsigned stage) {
	return mNextBitTree[path][stage]->data;
}

float& PathList::Metric(unsigned path) {
	return mMetric[path];
}

float& PathList::NextMetric(unsigned path) {
	return mNextMetric[path];
}

unsigned PathList::PathCount() {
	return mPathCount;
}

unsigned PathList::PathLimit() {
	return mPathLimit;
}

void PathList::setNextPathCount(unsigned pc) {
	mNextPathCount = pc;
}

Node::Node() {
}

Node::Node(Node *other)
	: xmDataPool(other->xmDataPool),
	  mBlockLength(other->mBlockLength),
	  mBitCount(other->mBitCount),
	  mStage(other->mStage),
	  mListSize(other->mListSize),
	  xmPathList(other->xmPathList) {
}

Node::Node(size_t blockLength, size_t listSize, datapool_t *pool, PathList *pathList)
	: xmDataPool(pool),
	  mBlockLength(blockLength),
	  mBitCount(nBit2fCount(blockLength)),
	  mStage(__builtin_ctz(blockLength)),
	  mListSize(listSize),
	  xmPathList(pathList) {
}

Node::~Node() {
}

void Node::decode() {
}

datapool_t* Node::pool() {
	return xmDataPool;
}

size_t Node::blockLength() {
	return mBlockLength;
}

SclAvx::PathList* Node::pathList() {
	return xmPathList;
}

/*unsigned Node::id() {
	return mId;
}

void Node::setId(unsigned newId) {
	mId = newId;
}

unsigned Node::lastId() {
	return mLastId;
}*/

/*************
 * (Short)RateRNode
 * ***********/

RateRNode::RateRNode() {
}

RateRNode::RateRNode(const std::vector<unsigned> &frozenBits, Node *parent)
	: Node(parent) {
	mBlockLength /= 2;
	mStage -= 1;

	std::vector<unsigned> leftFrozenBits, rightFrozenBits;
	splitFrozenBits(frozenBits, mBlockLength, leftFrozenBits, rightFrozenBits);

	mLeft = createDecoder(leftFrozenBits, this);
	mRight = createDecoder(rightFrozenBits, this);
}

RateRNode::~RateRNode() {
	delete mLeft;
	delete mRight;
}

void RateRNode::decode() {
	xmPathList->allocateStage(mStage);

	unsigned pathCount = xmPathList->PathCount();
	for(unsigned path=0; path < pathCount; ++path) {
		FastSscAvx::F_function(xmPathList->Llr(path, mStage+1), xmPathList->Llr(path, mStage), mBlockLength);
	}

	mLeft->decode();

	xmPathList->prepareRightDecoding(mStage);
	pathCount = xmPathList->PathCount();
	for(unsigned path=0; path < pathCount; ++path) {
		FastSscAvx::G_function(xmPathList->Llr(path, mStage+1), xmPathList->Llr(path, mStage), xmPathList->LeftBit(path, mStage), mBlockLength);
	}

	mRight->decode();

	pathCount = xmPathList->PathCount();
	for(unsigned path=0; path < pathCount; ++path) {
		xmPathList->getWriteAccessToBit(path, mStage+1);
		FastSscAvx::CombineSoftBitsLong(xmPathList->LeftBit(path, mStage), xmPathList->Bit(path, mStage), xmPathList->Bit(path, mStage+1), mBlockLength);
	}

	xmPathList->clearStage(mStage);
}
/*
void RateRNode::setId(unsigned newId) {
	mId = newId;
	mLeft->setId(newId+1);
	mRight->setId(mLeft->lastId() + 1);
}*/

ShortRateRNode::ShortRateRNode(const std::vector<unsigned> &frozenBits, Node *parent)
	: RateRNode(frozenBits, parent) {
}

ShortRateRNode::~ShortRateRNode() {
}


void ShortRateRNode::decode() {
	xmPathList->allocateStage(mStage);

	unsigned pathCount = xmPathList->PathCount();
	for(unsigned path=0; path < pathCount; ++path) {
		FastSscAvx::F_function(xmPathList->Llr(path, mStage+1), xmPathList->Llr(path, mStage), mBlockLength);
	}

	mLeft->decode();

	xmPathList->prepareRightDecoding(mStage);
	pathCount = xmPathList->PathCount();
	for(unsigned path=0; path < pathCount; ++path) {
		FastSscAvx::G_function(xmPathList->Llr(path, mStage+1), xmPathList->Llr(path, mStage), xmPathList->LeftBit(path, mStage), mBlockLength);
	}

	mRight->decode();

	pathCount = xmPathList->PathCount();
	for(unsigned path=0; path < pathCount; ++path) {
		xmPathList->getWriteAccessToBit(path, mStage+1);
		FastSscAvx::CombineSoftBitsShort(xmPathList->LeftBit(path, mStage), xmPathList->Bit(path, mStage), xmPathList->Bit(path, mStage+1), mBlockLength);
	}

	xmPathList->clearStage(mStage);
}

/*************
 * RateZeroDecoder
 * ***********/
RateZeroDecoder::RateZeroDecoder(Node *parent)
	: Node(parent) {
}

RateZeroDecoder::~RateZeroDecoder() {
}

void RateZeroDecoder::decode() {
	const __m256 zero = _mm256_setzero_ps();
	const __m256 inf = _mm256_set1_ps(INFINITY);
	unsigned pathCount = xmPathList->PathCount();
	float *fBits, *LlrSource;

	for(unsigned path = 0; path < pathCount; ++path) {
		__m256 punishment = _mm256_setzero_ps();
		fBits = xmPathList->Bit(path, mStage);
		LlrSource = xmPathList->Llr(path, mStage);
		for(unsigned i=mBlockLength; i<8; ++i) {
			LlrSource[i] = 0.0;
		}
		for(unsigned bit = 0; bit < mBlockLength; bit += 8) {
			_mm256_store_ps(fBits+bit, inf);
			__m256 LlrIn = _mm256_load_ps(LlrSource+bit);
			punishment = _mm256_add_ps(punishment, _mm256_min_ps(LlrIn, zero));
		}
		xmPathList->Metric(path) += reduce_add_ps(punishment);
	}

}

/*************
 * RateOneDecoder
 * ***********/
RateOneDecoder::RateOneDecoder(Node *parent)
	: Node(parent)
{
	mIndices.resize(std::max(mBlockLength, mListSize*4));
	mMetrics.resize(mListSize * 4);
	mBitFlipHints.resize(mListSize * 4);
}

RateOneDecoder::~RateOneDecoder() {
}

void RateOneDecoder::decode() {
	const __m256 sgnMask = _mm256_set1_ps(-0.0);
	unsigned pathCount = xmPathList->PathCount();
	block_t *tempBlock = xmDataPool->allocate(mBlockLength);
	float *temp = tempBlock->data;

	for(unsigned path = 0; path < pathCount; ++path) {
		float metric = xmPathList->Metric(path);
		float* LlrSource = xmPathList->Llr(path, mStage);
		for(unsigned i=mBlockLength; i<8; ++i) {
			LlrSource[i] = INFINITY;
		}
		for(unsigned i=0; i<mBlockLength; i+=8) {
			__m256 Llr = _mm256_load_ps(LlrSource+i);
			Llr = _mm256_andnot_ps(sgnMask, Llr);
			_mm256_store_ps(temp+i, Llr);
		}
		findWeakLlrs(mIndices, temp, mBlockLength, 2);
		mMetrics[path*4] = metric;
		mMetrics[path*4+1] = metric - temp[0];
		mMetrics[path*4+2] = metric - temp[1];
		mMetrics[path*4+3] = metric - temp[0] - temp[1];

		mBitFlipHints[path*4] = {};
		mBitFlipHints[path*4+1] = {mIndices[0]};
		mBitFlipHints[path*4+2] = {mIndices[1]};
		mBitFlipHints[path*4+3] = {mIndices[0], mIndices[1]};
	}
	xmDataPool->release(tempBlock);

	unsigned newPathCount = std::min(pathCount*4, (unsigned)mListSize);
	xmPathList->setNextPathCount(newPathCount);
	simplePartialSortDescending<unsigned,float>(mIndices, mMetrics, newPathCount, pathCount*4);

	for(unsigned path = 0; path < newPathCount; ++path) {
		xmPathList->duplicatePath(path, mIndices[path]/4, mStage);
	}

	xmPathList->clearOldPaths(mStage);

	union {
		float* fBitDestination;
		unsigned int* iBitDestination;
	};

	for(unsigned path = 0; path < newPathCount; ++path) {
		xmPathList->getWriteAccessToNextBit(path, mStage);
		xmPathList->NextMetric(path) = mMetrics[path];
		float* LlrSource = xmPathList->NextLlr(path, mStage);
		fBitDestination = xmPathList->NextBit(path, mStage);
		for(unsigned i=0; i<mBlockLength; i+=8) {
			__m256 Llr = _mm256_load_ps(LlrSource+i);
			_mm256_store_ps(fBitDestination+i, Llr);
		}

		for(unsigned index : mBitFlipHints[mIndices[path]]) {
			iBitDestination[index] ^= 0x80000000;
		}
	}

	xmPathList->switchToNext();
}


/*************
 * RepetitionDecoder
 * ***********/
RepetitionDecoder::RepetitionDecoder(Node *parent)
	: Node(parent)
{
	mIndices.resize(mListSize * 2);
	mMetrics.resize(mListSize * 2);
	mResults.resize(mListSize * 2);
}

RepetitionDecoder::~RepetitionDecoder() {
}

void RepetitionDecoder::decode() {
	const __m256 zero = _mm256_setzero_ps();
	unsigned pathCount = xmPathList->PathCount();

	for(unsigned path = 0; path < pathCount; ++path) {
		float metric  = xmPathList->Metric(path);
		__m256 vZero  = _mm256_setzero_ps();//metric for '0' decision
		__m256 vOne   = _mm256_setzero_ps();//metric for '1' decision
		__m256 vResult = _mm256_setzero_ps();//repetition decoding result
		float* LlrSource = xmPathList->Llr(path, mStage);
		for(unsigned i=mBlockLength; i<8; ++i) {
			LlrSource[i] = 0.0;
		}
		for(unsigned i=0; i<mBlockLength; i+=8) {
			__m256 Llr = _mm256_load_ps(LlrSource+i);
			vZero   = _mm256_add_ps(vZero, _mm256_min_ps(Llr, zero));
			vOne    = _mm256_add_ps(vOne,  _mm256_max_ps(Llr, zero));
			vResult = _mm256_add_ps(vResult, Llr);
		}

		{// Calculate output LLR, but fix the sign bit to the candidate's assumption
			union {
				float        fResultZero;
				unsigned int iResultZero;
			};
			union {
				float        fResultOne;
				unsigned int iResultOne;
			};

			fResultOne = fResultZero = reduce_add_ps(vResult);
			iResultZero &= 0x7FFFFFFF;// Unset sign bit
			iResultOne  |= 0x80000000;// Set sign bit

			mResults[path*2]   = fResultZero;
			mResults[path*2+1] = fResultOne;
		}

		mMetrics[path*2]   = metric + reduce_add_ps(vZero);
		mMetrics[path*2+1] = metric - reduce_add_ps(vOne);
	}

	unsigned newPathCount = std::min(pathCount*2, (unsigned)mListSize);
	xmPathList->setNextPathCount(newPathCount);
	simplePartialSortDescending<unsigned,float>(mIndices, mMetrics, newPathCount, pathCount*2);

	for(unsigned path = 0; path < newPathCount; ++path) {
		xmPathList->duplicatePath(path, mIndices[path]/2, mStage);
	}

	xmPathList->clearOldPaths(mStage);

	for(unsigned path = 0; path < newPathCount; ++path) {
		xmPathList->getWriteAccessToNextBit(path, mStage);
		xmPathList->NextMetric(path) = mMetrics[path];

		__m256 output = _mm256_set1_ps(mResults[mIndices[path]]);
		float* bitDestination = xmPathList->NextBit(path, mStage);

		for(unsigned i=0; i<mBlockLength; i+=8) {
			_mm256_store_ps(bitDestination+i, output);
		}
	}

	xmPathList->switchToNext();
}

/*************
 * SpcDecoder
 * ***********/
SpcDecoder::SpcDecoder(Node *parent)
	: Node(parent)
{
	mIndices.resize(std::max(mBlockLength, mListSize*8));
	mMetrics.resize(mListSize * 8);
	mBitFlipHints.resize(mListSize * 8);
	mBitFlipCount.resize(mListSize * 8);
}

SpcDecoder::~SpcDecoder() {
}

void SpcDecoder::decode() {
	const __m256 sgnMask = _mm256_set1_ps(-0.0);
	unsigned pathCount = xmPathList->PathCount();
	block_t *tempBlock = xmDataPool->allocate(mBlockLength);
	float *temp = tempBlock->data;

	__m256 vParity;
	float fParityInv;

	union {
		float uFloat;
		unsigned int uInt;
	};

	for(unsigned path = 0; path < pathCount; ++path) {
		float metric = xmPathList->Metric(path);
		float* LlrSource = xmPathList->Llr(path, mStage);
		vParity = _mm256_set1_ps(0.0f);

		//For short SPC codes (N<8), neutralize unused vector elements
		for(unsigned i=mBlockLength; i<8; ++i) {
			LlrSource[i] = INFINITY;
		}

		//Calculate parity and save absolute values
		for(unsigned i=0; i<mBlockLength; i+=8) {
			__m256 Llr = _mm256_load_ps(LlrSource+i);
			vParity = _mm256_xor_ps(vParity, Llr);
			Llr = _mm256_andnot_ps(sgnMask, Llr);
			_mm256_store_ps(temp+i, Llr);
		}

		findWeakLlrs(mIndices, temp, mBlockLength, 4);

		//Create candidates
		uFloat = reduce_xor_ps(vParity);
		if(uInt&0x80000000) {
			fParityInv = 0.0;
			metric -= temp[0];
			mBitFlipCount[path*8  ] = 1; mBitFlipHints[path*8  ][0] = mIndices[0];
			mBitFlipCount[path*8+1] = 0;
			mBitFlipCount[path*8+2] = 0;
			mBitFlipCount[path*8+3] = 0;
			mBitFlipCount[path*8+4] = 1; mBitFlipHints[path*8+4][0] = mIndices[0];
			mBitFlipCount[path*8+5] = 1; mBitFlipHints[path*8+5][0] = mIndices[0];
			mBitFlipCount[path*8+6] = 1; mBitFlipHints[path*8+6][0] = mIndices[0];
			mBitFlipCount[path*8+7] = 0;
		} else {
			fParityInv = 1.0;
			mBitFlipCount[path*8  ] = 0;
			mBitFlipCount[path*8+1] = 1; mBitFlipHints[path*8+1][0] = mIndices[0];
			mBitFlipCount[path*8+2] = 1; mBitFlipHints[path*8+2][0] = mIndices[0];
			mBitFlipCount[path*8+3] = 1; mBitFlipHints[path*8+3][0] = mIndices[0];
			mBitFlipCount[path*8+4] = 0;
			mBitFlipCount[path*8+5] = 0;
			mBitFlipCount[path*8+6] = 0;
			mBitFlipCount[path*8+7] = 1; mBitFlipHints[path*8+7][0] = mIndices[0];
		}

		mMetrics[path*8  ] = metric;
		mMetrics[path*8+1] = metric - fParityInv * temp[0] - temp[1];
		mMetrics[path*8+2] = metric - fParityInv * temp[0] - temp[2];
		mMetrics[path*8+3] = metric - fParityInv * temp[0] - temp[3];
		mMetrics[path*8+4] = metric -              temp[1] - temp[2];
		mMetrics[path*8+5] = metric -              temp[1] - temp[3];
		mMetrics[path*8+6] = metric -              temp[2] - temp[3];
		mMetrics[path*8+7] = metric - fParityInv * temp[0] - temp[1] - temp[2] - temp[3];

		mBitFlipHints[path*8+1][mBitFlipCount[path*8+1]++] = mIndices[1];
		mBitFlipHints[path*8+2][mBitFlipCount[path*8+2]++] = mIndices[2];
		mBitFlipHints[path*8+3][mBitFlipCount[path*8+3]++] = mIndices[3];
		mBitFlipHints[path*8+4][mBitFlipCount[path*8+4]++] = mIndices[1];
		mBitFlipHints[path*8+4][mBitFlipCount[path*8+4]++] = mIndices[2];
		mBitFlipHints[path*8+5][mBitFlipCount[path*8+5]++] = mIndices[1];
		mBitFlipHints[path*8+5][mBitFlipCount[path*8+5]++] = mIndices[3];
		mBitFlipHints[path*8+6][mBitFlipCount[path*8+6]++] = mIndices[2];
		mBitFlipHints[path*8+6][mBitFlipCount[path*8+6]++] = mIndices[3];
		mBitFlipHints[path*8+7][mBitFlipCount[path*8+7]++] = mIndices[1];
		mBitFlipHints[path*8+7][mBitFlipCount[path*8+7]++] = mIndices[2];
		mBitFlipHints[path*8+7][mBitFlipCount[path*8+7]++] = mIndices[3];
	}
	xmDataPool->release(tempBlock);

	unsigned newPathCount = std::min(pathCount*8, (unsigned)mListSize);
	xmPathList->setNextPathCount(newPathCount);
	simplePartialSortDescending<unsigned,float>(mIndices, mMetrics, newPathCount, pathCount*8);

	for(unsigned path = 0; path < newPathCount; ++path) {
		xmPathList->duplicatePath(path, mIndices[path]/8, mStage);
	}

	xmPathList->clearOldPaths(mStage);

	union {
		float* fBitDestination;
		unsigned int* iBitDestination;
	};

	for(unsigned path = 0; path < newPathCount; ++path) {
		xmPathList->getWriteAccessToNextBit(path, mStage);
		xmPathList->NextMetric(path) = mMetrics[path];
		float* LlrSource = xmPathList->NextLlr(path, mStage);
		fBitDestination = xmPathList->NextBit(path, mStage);
		for(unsigned i=0; i<mBlockLength; i+=8) {
			__m256 Llr = _mm256_load_ps(LlrSource+i);
			_mm256_store_ps(fBitDestination+i, Llr);
		}

		unsigned source = mIndices[path], max = mBitFlipCount[source];
		for(unsigned i=0; i<max; ++i) {
			iBitDestination[mBitFlipHints[source][i]] ^= 0x80000000;
		}
	}

	xmPathList->switchToNext();
}



Node* createDecoder(const std::vector<unsigned> &frozenBits, Node *parent) {
	size_t blockLength = parent->blockLength();
	size_t frozenBitCount = frozenBits.size();

	if(frozenBitCount == blockLength) {
		return new RateZeroDecoder(parent);
	}

	if(frozenBitCount == blockLength-1) {
		return new RepetitionDecoder(parent);
	}

	if(frozenBitCount == 1) {
		return new SpcDecoder(parent);
	}

	if(frozenBitCount == 0) {
		return new RateOneDecoder(parent);
	}

	if(blockLength <= 8) {
		return new ShortRateRNode(frozenBits, parent);
	} else {
		return new RateRNode(frozenBits, parent);
	}
}

}// namespace SclAvx

SclAvxFloat::SclAvxFloat(size_t blockLength, size_t listSize, const std::vector<unsigned> &frozenBits)
	: mListSize(listSize) {
	initialize(blockLength, frozenBits);
}

SclAvxFloat::~SclAvxFloat() {
	clear();
}

void SclAvxFloat::clear() {
	if(mRootNode) delete mRootNode;
	delete mNodeBase;
	delete mPathList;
	delete mDataPool;
}

void SclAvxFloat::initialize(size_t blockLength, const std::vector<unsigned> &frozenBits) {
	if(blockLength == mBlockLength && frozenBits == mFrozenBits) {
		return;
	}
	if(mBlockLength != 0) {
		clear();
	}
	mBlockLength = blockLength;
	mFrozenBits.assign(frozenBits.begin(), frozenBits.end());
	mDataPool = new SclAvx::datapool_t();
	mPathList = new SclAvx::PathList(mListSize, __builtin_ctz(mBlockLength)+1, mDataPool);
	mNodeBase = new SclAvx::Node(mBlockLength, mListSize, mDataPool, mPathList);
	mRootNode = SclAvx::createDecoder(mFrozenBits, mNodeBase);
	mLlrContainer = new FloatContainer(mBlockLength);
	mBitContainer = new FloatContainer(mBlockLength, mFrozenBits);
	mOutputContainer = new unsigned char[(mBlockLength-mFrozenBits.size()+7)/8];
}

bool SclAvxFloat::decode() {
	makeInitialPathList();

	mRootNode->decode();

	return extractBestPath();
}

void SclAvxFloat::makeInitialPathList() {
	mPathList->clear();
	mPathList->setFirstPath(dynamic_cast<FloatContainer*>(mLlrContainer)->data());
}

bool SclAvxFloat::extractBestPath() {
	unsigned dataStage = __builtin_ctz(mBlockLength);
	unsigned byteLength = (mBlockLength-mFrozenBits.size()+7)/8;
	unsigned pathCount = mPathList->PathCount();
	bool decoderSuccess = false;
	if(mSystematic) {
		for(unsigned path = 0; path < pathCount; ++path) {
			mBitContainer->insertLlr(mPathList->Bit(path, dataStage));
			mBitContainer->getPackedInformationBits(mOutputContainer);
			if(mErrorDetector->check(mOutputContainer, byteLength)) {
				decoderSuccess = true;
				break;
			}
		}
		// Fall back to ML path, if none of the candidates was free of errors
		if(!decoderSuccess) {
			mBitContainer->insertLlr(mPathList->Bit(0, dataStage));
			mBitContainer->getPackedInformationBits(mOutputContainer);
		}
	} else {// non-systematic
		PolarCode::Encoding::Encoder *encoder
				= new PolarCode::Encoding::ButterflyAvx2Packed(mBlockLength, mFrozenBits);
		PolarCode::PackedContainer *container
				= new PolarCode::PackedContainer(mBlockLength, mFrozenBits);
		for(unsigned path = 0; path < pathCount; ++path) {
			container->insertLlr(mPathList->Bit(path, dataStage));
			encoder->setCodeword(container->data());
			encoder->encode();
			encoder->getInformation(mOutputContainer);
			if(mErrorDetector->check(mOutputContainer, byteLength)) {
				decoderSuccess = true;
				break;
			}
		}
		// Fall back to ML path, if none of the candidates was free of errors
		if(!decoderSuccess) {
			container->insertLlr(mPathList->Bit(0, dataStage));
			encoder->setCodeword(container->data());
			encoder->encode();
			encoder->getInformation(mOutputContainer);
		}
		delete container;
		delete encoder;
	}
	mPathList->clear();// Clean up
	return decoderSuccess;
}

}// namespace Decoding
}// namespace PolarCode
