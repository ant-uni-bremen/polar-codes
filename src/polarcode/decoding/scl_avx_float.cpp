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
	mMetric.assign(listSize, 0);
	mNextLlrTree.resize(listSize);
	mNextBitTree.resize(listSize);
	mNextMetric.assign(listSize, 0);
	for(unsigned i=0; i<mPathLimit; ++i) {
		mLlrTree[i].resize(stageCount);
		mBitTree[i].resize(stageCount);
		mNextLlrTree[i].resize(stageCount);
		mNextBitTree[i].resize(stageCount);
	}
	tempBlock = xmDataPool->allocate(nBit2fCount(1<<stageCount));
}

PathList::~PathList() {
	xmDataPool->release(tempBlock);
	clear();
}

void PathList::clear() {
	for(unsigned path=0; path < mPathCount; ++path) {
		xmDataPool->release(mLlrTree[path][mStageCount-1]);
		xmDataPool->release(mBitTree[path][mStageCount-1]);
	}
	mPathCount = 0;
}

void PathList::duplicatePath(unsigned destination, unsigned source, unsigned stage) {
	for(unsigned i = stage; i<mStageCount; ++i) {
		mNextLlrTree[destination][i] = xmDataPool->lazyDuplicate(mLlrTree[source][i]);
		mNextBitTree[destination][i] = xmDataPool->lazyDuplicate(mBitTree[source][i]);
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

void PathList::clearOldPath(unsigned path, unsigned stage) {
	for(unsigned i = stage; i < mStageCount; ++i) {
		xmDataPool->release(mLlrTree[path][i]);
		xmDataPool->release(mBitTree[path][i]);
	}
}

void PathList::switchToNext() {
	std::swap(mLlrTree, mNextLlrTree);
	std::swap(mBitTree, mNextBitTree);
	std::swap(mMetric, mNextMetric);
	mPathCount = mNextPathCount;
}

void PathList::setFirstPath(void *pLlr, unsigned bitCount) {
	unsigned expandedBitCount = nBit2fCount(bitCount);
	mLlrTree[0][mStageCount-1] = xmDataPool->allocate(expandedBitCount);
	mBitTree[0][mStageCount-1] = xmDataPool->allocate(expandedBitCount);
	mPathCount = 1;

	memcpy(Llr(0, mStageCount-1), pLlr, 4*expandedBitCount);
}

void PathList::allocateStage(unsigned stage, unsigned bitCount) {
	unsigned expandedBitCount = nBit2fCount(bitCount);
	for(unsigned path = 0; path < mPathCount; ++path) {
		mLlrTree[path][stage] = xmDataPool->allocate(expandedBitCount);
		mBitTree[path][stage] = xmDataPool->allocate(expandedBitCount);
	}
}

void PathList::clearStage(unsigned stage) {
	for(unsigned path = 0; path < mPathCount; ++path) {
		xmDataPool->release(mLlrTree[path][stage]);
		xmDataPool->release(mBitTree[path][stage]);
	}
}

float* PathList::Llr(unsigned path, unsigned stage) {
	return mLlrTree[path][stage]->data;
}

float* PathList::Bit(unsigned path, unsigned stage) {
	return mBitTree[path][stage]->data;
}

block_t* PathList::exportBits(unsigned path, unsigned stage) {
	block_t *ret = mBitTree[path][stage];
	mBitTree[path][stage] = xmDataPool->allocate(ret->size);
	return ret;
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
	  mListSize(other->mListSize),
	  xmPathList(other->xmPathList),
	  mSoftOutput(other->mSoftOutput) {
}

Node::Node(size_t blockLength, size_t listSize, datapool_t *pool, PathList *pathList, bool so)
	: xmDataPool(pool),
	  mBlockLength(blockLength),
	  mBitCount(nBit2fCount(blockLength)),
	  mListSize(listSize),
	  xmPathList(pathList),
	  mSoftOutput(so) {
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

DecoderNode::DecoderNode() {
}

DecoderNode::DecoderNode(const std::vector<unsigned> &frozenBits, Node *parent)
	: Node(parent),
	  mParent(parent),
	  mStage(__builtin_ctz(mBlockLength)-1) {
	mBlockLength /= 2;
	mBitCount = nBit2fCount(mBlockLength);

	std::vector<unsigned> leftFrozenBits, rightFrozenBits;
	splitFrozenBits(frozenBits, mBlockLength, leftFrozenBits, rightFrozenBits);

	mLeft = createDecoder(leftFrozenBits, this, &leftDecoder);
	mRight = createDecoder(rightFrozenBits, this, &rightDecoder);

	combineFunction = mSoftOutput ? CombineSoft : Combine;
}

DecoderNode::~DecoderNode() {
	delete mLeft;
	delete mRight;
}

void DecoderNode::decode() {
	unsigned pathCount = xmPathList->PathCount();
	xmPathList->allocateStage(mStage, mBitCount);
	for(unsigned path=0; path < pathCount; ++path) {
		F_function(xmPathList->Llr(path, mStage+1), xmPathList->Llr(path, mStage), mBlockLength);
	}

	if(mLeft) {
		mLeft->decode();
	} else {
		for(unsigned path=0; path < pathCount; ++path) {
			xmPathList->getWriteAccessToBit(path, mStage);
		}
		leftDecoder(xmPathList, mStage);
	}

	pathCount = xmPathList->PathCount();
	for(unsigned path=0; path < pathCount; ++path) {
		xmPathList->getWriteAccessToBit(path, mStage+1);
		memcpy(xmPathList->Bit(path, mStage+1), xmPathList->Bit(path, mStage), mBlockLength*4);
		xmPathList->getWriteAccessToLlr(path, mStage);
		G_function(xmPathList->Llr(path, mStage+1), xmPathList->Llr(path, mStage), xmPathList->Bit(path, mStage), mBlockLength);
	}

	if(mRight) {
		mRight->decode();
	} else {
		for(unsigned path=0; path < pathCount; ++path) {
			xmPathList->getWriteAccessToBit(path, mStage);
		}
		rightDecoder(xmPathList, mStage);
	}

	pathCount = xmPathList->PathCount();
	for(unsigned path=0; path < pathCount; ++path) {
		xmPathList->getWriteAccessToBit(path, mStage+1);
		memcpy(xmPathList->Bit(path, mStage+1)+mBlockLength, xmPathList->Bit(path, mStage), mBlockLength*4);
		combineFunction(xmPathList->Bit(path, mStage+1), mBlockLength);
	}

	xmPathList->clearStage(mStage);
}

Node* createDecoder(const std::vector<unsigned> &frozenBits, Node *parent, void (**specialDecoder)(PathList *, unsigned)) {
	size_t blockLength = parent->blockLength();
	size_t frozenBitCount = frozenBits.size();

	if(blockLength == 1) {
		if(frozenBitCount == 0) {
			*specialDecoder = &RateOneDecodeSingleBit;
		} else {
			*specialDecoder = &RateZeroDecodeSingleBit;
		}
		return nullptr;
	}

	if(blockLength >= 8) {
		if(frozenBitCount == blockLength) {
			*specialDecoder = &RateZeroDecode;
			return nullptr;
		}
/*		if(frozenBitCount == 0) {
			*specialDecoder = &RateOneDecode;
			return nullptr;
		}*/
	}

	*specialDecoder = nullptr;

	return new DecoderNode(frozenBits, parent);
}

void RateZeroDecode(PathList *pathList, unsigned stage) {
	static const __m256 zero = _mm256_setzero_ps();
	static const __m256 inf = _mm256_set1_ps(INFINITY);
	unsigned pathCount = pathList->PathCount();
	unsigned stageBits = 1<<stage;
	float *fBits;
	for(unsigned path = 0; path < pathCount; ++path) {
		__m256 punishment = _mm256_setzero_ps();
		fBits = pathList->Bit(path, stage);
		for(unsigned bit = 0; bit < stageBits; bit += 8) {
			_mm256_store_ps(fBits+bit, inf);
			__m256 LlrIn = _mm256_load_ps(pathList->Llr(path, stage)+bit);
			punishment = _mm256_add_ps(punishment, _mm256_min_ps(LlrIn, zero));
		}
		pathList->Metric(path) += reduce_add_ps(punishment);
	}
}

void RateOneDecode(PathList *pathList, unsigned stage) {
	static const __m256 sgnMask = _mm256_set1_ps(-0.0);
	std::vector<unsigned> indices;
	std::vector<float> metrics;
	std::vector<std::vector<unsigned>> bitFlipHints;
	unsigned pathCount = pathList->PathCount();
	unsigned bitCount = nBit2fCount(1<<stage);
	float* vTempBlock = pathList->tempBlock->data;

	metrics.resize(pathCount*4);
	bitFlipHints.resize(pathCount*4);

	for(unsigned path = 0; path < pathCount; ++path) {
		long metric = pathList->Metric(path);
		float* LlrSource = pathList->Llr(path, stage);
		for(unsigned i=0; i<bitCount; i+=8) {
			__m256 Llr = _mm256_load_ps(LlrSource+i);
			Llr = _mm256_andnot_ps(sgnMask, Llr);
			_mm256_store_ps(vTempBlock+i, Llr);
		}
		simplePartialSortDescending<unsigned,float>(indices, vTempBlock, 1<<stage, 2);
		metrics[path*4] = metric;
		metrics[path*4+1] = metric-vTempBlock[0];
		metrics[path*4+2] = metric-vTempBlock[1];
		metrics[path*4+3] = metric-vTempBlock[0]-vTempBlock[1];

		bitFlipHints[path*4] = {};
		bitFlipHints[path*4+1] = {indices[0]};
		bitFlipHints[path*4+2] = {indices[1]};
		bitFlipHints[path*4+3] = {indices[0],indices[1]};
	}
	unsigned newPathCount = std::min(pathCount*4, pathList->PathLimit());
	pathList->setNextPathCount(newPathCount);
	simplePartialSortDescending<unsigned,float>(indices, metrics, newPathCount);

	for(unsigned path = 0; path < newPathCount; ++path) {
		pathList->duplicatePath(path, indices[path]/4, stage);
	}

	for(unsigned path = 0; path < pathCount; ++path) {
		pathList->clearOldPath(path, stage);
	}

	for(unsigned path = 0; path < newPathCount; ++path) {
		pathList->getWriteAccessToNextBit(path, stage);
		pathList->NextMetric(path) = metrics[path];
		float* LlrSource = pathList->NextLlr(path, stage);
		union {
			float* fBitDestination;
			unsigned int* iBitDestination;
		};
		fBitDestination = pathList->NextBit(path, stage);
		for(unsigned i=0; i<bitCount; i+=8) {
			__m256 Llr = _mm256_load_ps(LlrSource+i);
/*			__m256 Bit = hardDecode(Llr);*/
			_mm256_store_ps(fBitDestination+i, /*Bit*/ Llr);
		}

		for(unsigned index : bitFlipHints[indices[path]]) {
			iBitDestination[index] ^= 0x80000000;
		}
	}

	pathList->switchToNext();
}

float readSingleLlr(PathList *pl, unsigned path, unsigned stage) {
	return *(pl->Llr(path, stage));
}

float readSingleNextLlr(PathList *pl, unsigned path, unsigned stage) {
	return *(pl->NextLlr(path, stage));
}

void writeSingleBit(PathList *pl, unsigned path, unsigned stage, float value) {
	*(pl->NextBit(path, stage)) = value;
}

void RateZeroDecodeSingleBit(PathList *pathList, unsigned) {
	unsigned pathCount = pathList->PathCount();
	for(unsigned path = 0; path < pathCount; ++path) {
		*(pathList->Bit(path, 0)) = INFINITY;
		float Llr = readSingleLlr(pathList, path, 0);
		pathList->Metric(path) += std::min(Llr, 0.0f);
	}
}

void RateOneDecodeSingleBit(PathList *pathList, unsigned) {
	std::vector<unsigned> indices;
	std::vector<float> metrics;
	unsigned pathCount = pathList->PathCount();

	metrics.resize(pathCount*2);

	for(unsigned path = 0; path < pathCount; ++path) {
		float metric = pathList->Metric(path);
		float llr = readSingleLlr(pathList, path, 0);
		float absllr = std::abs(llr);
		metrics[path*2] = metric;
		metrics[path*2+1] = metric-absllr;
	}
	unsigned newPathCount = std::min(pathCount*2, pathList->PathLimit());
	pathList->setNextPathCount(newPathCount);
	simplePartialSortDescending<unsigned,float>(indices, metrics, newPathCount);

	for(unsigned path = 0; path < newPathCount; ++path) {
		pathList->duplicatePath(path, indices[path]/2, 0);
	}

	for(unsigned path = 0; path < pathCount; ++path) {
		pathList->clearOldPath(path, 0);
	}

	for(unsigned path = 0; path < newPathCount; ++path) {
		pathList->getWriteAccessToNextBit(path, 0);
		pathList->NextMetric(path) = metrics[path];
		union {
			float fBit;
			unsigned iBit;
		};
		fBit = /*hardDecode(*/readSingleNextLlr(pathList, path, 0)/*)*/;
		iBit ^= (indices[path]&1)<<31;
		writeSingleBit(pathList, path, 0, fBit);
	}

	pathList->switchToNext();
}

}// namespace SclAvx

SclAvxFloat::SclAvxFloat(size_t blockLength, size_t listSize, const std::vector<unsigned> &frozenBits, bool softOutput)
	: mListSize(listSize) {
	mSoftOutput = softOutput;
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
	mNodeBase = new SclAvx::Node(mBlockLength, mListSize, mDataPool, mPathList, mSoftOutput);
	mRootNode = SclAvx::createDecoder(frozenBits, mNodeBase, &mSpecialDecoder);
	mLlrContainer = new FloatContainer(mBlockLength);
	mBitContainer = new FloatContainer(mBlockLength, mFrozenBits);
	mOutputContainer = new unsigned char[(mBlockLength-frozenBits.size()+7)/8];
}

bool SclAvxFloat::decode() {
	makeInitialPathList();

	if(mRootNode) {
		mRootNode->decode();
	}

	return extractBestPath();
}

void SclAvxFloat::makeInitialPathList() {
	mPathList->clear();
	mPathList->setFirstPath(dynamic_cast<FloatContainer*>(mLlrContainer)->data(), nBit2fCount(mBlockLength));
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
