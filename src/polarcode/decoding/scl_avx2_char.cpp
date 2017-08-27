#include <polarcode/decoding/scl_avx2_char.h>
#include <polarcode/polarcode.h>
#include <polarcode/arrayfuncs.h>
#include <polarcode/encoding/butterfly_avx2_packed.h>

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
	tempBlock = xmDataPool->allocate(nBit2vecCount(1<<stageCount));
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
	  mVecCount(other->mVecCount),
	  mListSize(other->mListSize),
	  xmPathList(other->xmPathList),
	  mSoftOutput(other->mSoftOutput) {
}

Node::Node(size_t blockLength, size_t listSize, datapool_t *pool, PathList *pathList, bool so)
	: xmDataPool(pool),
	  mBlockLength(blockLength),
	  mVecCount(nBit2vecCount(blockLength)),
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

SclAvx2::PathList* Node::pathList() {
	return xmPathList;
}

DecoderNode::DecoderNode() {
}

DecoderNode::DecoderNode(const std::vector<unsigned> &frozenBits, Node *parent)
	: Node(parent),
	  mParent(parent),
	  mStage(__builtin_ctz(mBlockLength)-1) {
	mBlockLength /= 2;
	mVecCount = nBit2vecCount(mBlockLength);

	std::vector<unsigned> leftFrozenBits, rightFrozenBits;
	splitFrozenBits(frozenBits, mBlockLength, leftFrozenBits, rightFrozenBits);

	mLeft = createDecoder(leftFrozenBits, this, &leftDecoder);
	mRight = createDecoder(rightFrozenBits, this, &rightDecoder);

	combineFunction = mSoftOutput ? CombineSoftBits : CombineBits;

	childBits.resize(mListSize);
	for(unsigned i=0; i<mListSize; ++i) {
		childBits[i] = xmDataPool->allocate(nBit2vecCount(1<<mStage));
	}
}

DecoderNode::~DecoderNode() {
	delete mLeft;
	delete mRight;
	for(unsigned i=0; i<mListSize; ++i) {
		xmDataPool->release(childBits[i]);
	}
}

void DecoderNode::decode() {
	unsigned pathCount = xmPathList->PathCount();
	xmPathList->allocateStage(mStage, mVecCount);
	for(unsigned path=0; path < pathCount; ++path) {
		F_function(xmPathList->Llr(path, mStage+1), xmPathList->Llr(path, mStage), mBlockLength);
	}

	if(mLeft) {
		mLeft->decode();
	} else {
		leftDecoder(xmPathList, mStage);
	}

	pathCount = xmPathList->PathCount();
	for(unsigned path=0; path < pathCount; ++path) {
		memcpy(childBits[path]->data, xmPathList->Bit(path, mStage), mBlockLength);
		xmPathList->getWriteAccessToLlr(path, mStage);
		G_function(xmPathList->Llr(path, mStage+1), xmPathList->Llr(path, mStage), xmPathList->Bit(path, mStage), mBlockLength);
	}

	if(mRight) {
		mRight->decode();
	} else {
		rightDecoder(xmPathList, mStage);
	}

	pathCount = xmPathList->PathCount();
	for(unsigned path=0; path < pathCount; ++path) {
		xmPathList->getWriteAccessToBit(path, mStage+1);
		combineFunction(childBits[path]->data, xmPathList->Bit(path, mStage), xmPathList->Bit(path, mStage+1), mBlockLength);
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

	if(blockLength >= 32) {
		if(frozenBitCount == blockLength) {
			*specialDecoder = &RateZeroDecode;
			return nullptr;
		}
		if(frozenBitCount == 0) {
			*specialDecoder = &RateOneDecode;
			return nullptr;
		}
	}

	*specialDecoder = nullptr;

	return new DecoderNode(frozenBits, parent);
}

void RateZeroDecode(PathList *pathList, unsigned stage) {
	const __m256i zero = _mm256_setzero_si256();
	unsigned pathCount = pathList->PathCount();
	unsigned stageVectors = nBit2vecCount(1<<stage);
	for(unsigned path = 0; path < pathCount; ++path) {
		memset(pathList->Bit(path, stage), 127, 32*stageVectors);
		__m256i punishment = _mm256_setzero_si256();
		for(unsigned vector = 0; vector < stageVectors; ++vector) {
			__m256i LlrIn = _mm256_load_si256(pathList->Llr(path, stage)+vector);
			__m128i epi8a = _mm256_extracti128_si256(LlrIn, 1);
			__m128i epi8b = _mm256_extracti128_si256(LlrIn, 0);
			__m256i epi16a = _mm256_cvtepi8_epi16(epi8a);
			__m256i epi16b = _mm256_cvtepi8_epi16(epi8b);
			punishment = _mm256_adds_epi16(
							punishment,
							_mm256_adds_epi16(
								_mm256_min_epi16(epi16a, zero),
								_mm256_min_epi16(epi16b, zero)));
		}
		pathList->Metric(path) += reduce_adds_epi16(punishment);
	}
}

void RateOneDecode(PathList *pathList, unsigned stage) {
	static const __m256i absCorrector = _mm256_set1_epi8(-127);
	std::vector<unsigned> indices;
	std::vector<long> metrics;
	std::vector<std::vector<unsigned>> bitFlipHints;
	unsigned pathCount = pathList->PathCount();
	unsigned vecCount = nBit2vecCount(1<<stage);
	__m256i* vTempBlock = pathList->tempBlock->data;
	char *cTempBlock = reinterpret_cast<char*>(vTempBlock);

	metrics.resize(pathCount*4);
	bitFlipHints.resize(pathCount*4);

	for(unsigned path = 0; path < pathCount; ++path) {
		long metric = pathList->Metric(path);
		__m256i* LlrSource = pathList->Llr(path, stage);
		for(unsigned i=0; i<vecCount; ++ i) {
			__m256i Llr = _mm256_load_si256(LlrSource+i);
			Llr = _mm256_abs_epi8(_mm256_max_epi8(Llr, absCorrector));
			_mm256_store_si256(vTempBlock+i, Llr);
		}
		simplePartialSortDescending<unsigned,char>(indices, cTempBlock, 1<<stage, 2);
		metrics[path*4] = metric;
		metrics[path*4+1] = metric-cTempBlock[0];
		metrics[path*4+2] = metric-cTempBlock[1];
		metrics[path*4+3] = metric-cTempBlock[0]-cTempBlock[1];

		bitFlipHints[path*4] = {};
		bitFlipHints[path*4+1] = {indices[0]};
		bitFlipHints[path*4+2] = {indices[1]};
		bitFlipHints[path*4+3] = {indices[0],indices[1]};
	}
	unsigned newPathCount = std::min(pathCount*4, pathList->PathLimit());
	pathList->setNextPathCount(newPathCount);
	simplePartialSortDescending<unsigned,long>(indices, metrics, newPathCount);

	for(unsigned path = 0; path < newPathCount; ++path) {
		pathList->duplicatePath(path, indices[path]/4, stage);
	}

	for(unsigned path = 0; path < pathCount; ++path) {
		pathList->clearOldPath(path, stage);
	}

	for(unsigned path = 0; path < newPathCount; ++path) {
		pathList->getWriteAccessToNextBit(path, stage);
		pathList->NextMetric(path) = metrics[path];
		__m256i* LlrSource = pathList->NextLlr(path, stage);
		__m256i* BitDestination = pathList->NextBit(path, stage);
		unsigned char *cBitDestination = reinterpret_cast<unsigned char*>(BitDestination);
		for(unsigned i=0; i<vecCount; ++i) {
			__m256i Llr = _mm256_load_si256(LlrSource+i);
/*			__m256i Bit = hardDecode(Llr);*/
			_mm256_store_si256(BitDestination+i, /*Bit*/ Llr);
		}

		for(unsigned index : bitFlipHints[indices[path]]) {
			cBitDestination[index] = ~(cBitDestination[index]);
		}
	}

	pathList->switchToNext();
}

char readSingleLlr(PathList *pl, unsigned path, unsigned stage) {
	return *(reinterpret_cast<char*>(pl->Llr(path, stage)));
}

char readSingleNextLlr(PathList *pl, unsigned path, unsigned stage) {
	return *(reinterpret_cast<char*>(pl->NextLlr(path, stage)));
}

void writeSingleBit(PathList *pl, unsigned path, unsigned stage, char value) {
	*(reinterpret_cast<char*>(pl->NextBit(path, stage))) = value;
}

void RateZeroDecodeSingleBit(PathList *pathList, unsigned) {
	unsigned pathCount = pathList->PathCount();
	for(unsigned path = 0; path < pathCount; ++path) {
		*(reinterpret_cast<char*>(pathList->Bit(path, 0))) = 127;
		char Llr = readSingleLlr(pathList, path, 0);
		pathList->Metric(path) += std::min(static_cast<long>(Llr), (long)0);
	}
}

void RateOneDecodeSingleBit(PathList *pathList, unsigned) {
	std::vector<unsigned> indices;
	std::vector<long> metrics;
	unsigned pathCount = pathList->PathCount();

	metrics.resize(pathCount*2);

	for(unsigned path = 0; path < pathCount; ++path) {
		long metric = pathList->Metric(path);
		char llr = readSingleLlr(pathList, path, 0);
		long absllr = std::abs(static_cast<long>(llr));
		metrics[path*2] = metric;
		metrics[path*2+1] = metric-absllr;
	}
	unsigned newPathCount = std::min(pathCount*2, pathList->PathLimit());
	pathList->setNextPathCount(newPathCount);
	simplePartialSortDescending<unsigned,long>(indices, metrics, newPathCount);

	for(unsigned path = 0; path < newPathCount; ++path) {
		pathList->duplicatePath(path, indices[path]/2, 0);
	}

	for(unsigned path = 0; path < pathCount; ++path) {
		pathList->clearOldPath(path, 0);
	}

	for(unsigned path = 0; path < newPathCount; ++path) {
		pathList->getWriteAccessToNextBit(path, 0);
		pathList->NextMetric(path) = metrics[path];
		char bit = /*hardDecode(*/readSingleNextLlr(pathList, path, 0)/*)*/;
		if((indices[path]&1) == 0) {
			writeSingleBit(pathList, path, 0, bit);
		} else {
			writeSingleBit(pathList, path, 0, ~bit);
		}
	}

	pathList->switchToNext();
}

}// namespace SclAvx2

SclAvx2Char::SclAvx2Char(size_t blockLength, size_t listSize, const std::vector<unsigned> &frozenBits, bool softOutput)
	: mListSize(listSize) {
	mSoftOutput = softOutput;
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
	mNodeBase = new SclAvx2::Node(mBlockLength, mListSize, mDataPool, mPathList, mSoftOutput);
	mRootNode = SclAvx2::createDecoder(frozenBits, mNodeBase, &mSpecialDecoder);
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
	mPathList->setFirstPath(mLlrContainer->data(), nBit2vecCount(mBlockLength));
}

bool SclAvx2Char::extractBestPath() {
	unsigned dataStage = __builtin_ctz(mBlockLength);
	unsigned byteLength = (mBlockLength-mFrozenBits.size()+7)/8;
	unsigned pathCount = mPathList->PathCount();
	bool decoderSuccess = false;
	if(mSystematic) {
		for(unsigned path = 0; path < pathCount; ++path) {
			mBitContainer->insertCharBits(mPathList->Bit(path, dataStage));
			mBitContainer->getPackedInformationBits(mOutputContainer);
			if(mErrorDetector->check(mOutputContainer, byteLength)) {
				decoderSuccess = true;
				break;
			}
		}
		// Fall back to ML path, if none of the candidates was free of errors
		if(!decoderSuccess) {
			mBitContainer->insertCharBits(mPathList->Bit(0, dataStage));
			mBitContainer->getPackedInformationBits(mOutputContainer);
		}
	} else {
		PolarCode::Encoding::Encoder *encoder
				= new PolarCode::Encoding::ButterflyAvx2Packed(mBlockLength, mFrozenBits);
		PolarCode::PackedContainer *container
				= new PolarCode::PackedContainer(mBlockLength, mFrozenBits);
		for(unsigned path = 0; path < pathCount; ++path) {
			container->insertCharBits(mPathList->Bit(path, dataStage));
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
			container->insertCharBits(mPathList->Bit(0, dataStage));
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
