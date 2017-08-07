#include <polarcode/decoding/scl_avx2_char.h>

namespace PolarCode {
namespace Decoding {

namespace SclAvx2 {

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
}

PathList::~PathList() {
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

void PathList::setFirstPath(void *pLlr, unsigned vecCount) {
	mLlrTree[0][mStageCount-1] = xmDataPool->allocate(vecCount);
	mBitTree[0][mStageCount-1] = xmDataPool->allocate(vecCount);
	mPathCount = 1;

	memcpy(Llr(0, mStageCount-1), pLlr, 32*vecCount);
}

void PathList::allocateStage(unsigned stage, unsigned vecCount) {
	for(unsigned path = 0; path < mPathCount; ++path) {
		mLlrTree[path][stage] = xmDataPool->allocate(vecCount);
		mBitTree[path][stage] = xmDataPool->allocate(vecCount);
	}
}

void PathList::clearStage(unsigned stage) {
	for(unsigned path = 0; path < mPathCount; ++path) {
		xmDataPool->release(mLlrTree[path][stage]);
		xmDataPool->release(mBitTree[path][stage]);
	}
}

__m256i* PathList::Llr(unsigned path, unsigned stage) {
	return mLlrTree[path][stage]->data;
}

__m256i* PathList::Bit(unsigned path, unsigned stage) {
	return mBitTree[path][stage]->data;
}

__m256i* PathList::NextLlr(unsigned path, unsigned stage) {
	return mNextLlrTree[path][stage]->data;
}

__m256i* PathList::NextBit(unsigned path, unsigned stage) {
	return mNextBitTree[path][stage]->data;
}

long& PathList::Metric(unsigned path) {
	return mMetric[path];
}

long& PathList::NextMetric(unsigned path) {
	return mNextMetric[path];
}

size_t nBit2vecCount(size_t blockLength) {
	return (blockLength+31)/32;
}

}// namespace SclAvx2

SclAvx2Char::SclAvx2Char(size_t blockLength, size_t listSize, const std::vector<unsigned> &frozenBits)
	: mListSize(listSize) {
	initialize(blockLength, frozenBits);
}

SclAvx2Char::~SclAvx2Char() {
	clear();
}

void SclAvx2Char::clear() {
	if(mRootNode) delete mRootNode;
	delete mNodeBase;
	delete mPathList;
	delete mDataPool;
}

void SclAvx2Char::initialize(size_t blockLength, const std::vector<unsigned> &frozenBits) {
	if(blockLength == mBlockLength && frozenBits == mFrozenBits) {
		return;
	}
	if(mBlockLength != 0) {
		clear();
	}
	mBlockLength = blockLength;
	mFrozenBits.assign(frozenBits.begin(), frozenBits.end());
	mDataPool = new SclAvx2::datapool_t();
	mPathList = new SclAvx2::PathList(mListSize, __builtin_ctz(mBlockLength)+1, mDataPool);
	mNodeBase = new SclAvx2::Node(mBlockLength, mListSize, mDataPool, mPathList);
	//mRootNode = new SclAvx2::createDecoder(frozenBits, mNodeBase, &mSpecialDecoder);
	mLlrContainer = new CharContainer(mBlockLength);
	mBitContainer = new CharContainer(mBlockLength, frozenBits);
	mOutputContainer = new unsigned char[(mBlockLength-frozenBits.size()+7)/8];
}

bool SclAvx2Char::decode() {
	makeInitialPathList();

	if(mRootNode) {
		mRootNode->decode();
	}

	return extractBestPath();
}

void SclAvx2Char::makeInitialPathList() {
	mPathList->clear();
	mPathList->setFirstPath(mLlrContainer->data(), SclAvx2::nBit2vecCount(mBlockLength));
}

bool SclAvx2Char::extractBestPath() {
	int dataStage = __builtin_ctz(mBlockLength);
	//SclAvx2::PathList::iterator pathIt = mPathList->begin();
	if(mSystematic) {
		mBitContainer->insertCharBits(mPathList->Bit(0, dataStage));
		mBitContainer->getPackedInformationBits(mOutputContainer);
	}
	mPathList->clear();
	bool decoderSuccess = mErrorDetector->check(mOutputContainer, (mBlockLength-mFrozenBits.size()+7)/8);
	return decoderSuccess;
}
}// namespace Decoding
}// namespace PolarCode
