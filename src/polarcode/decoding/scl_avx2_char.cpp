#include <polarcode/decoding/scl_avx2_char.h>

namespace PolarCode {
namespace Decoding {



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
	clearPathList();
	delete mPathList;
	delete mDataPool;
}

void SclAvx2Char::clearPathList() {
	for(auto item : *mPathList) {
		for(SclAvx2::block_t* block : item.second.Llr) {
			mDataPool->release(block);
		}
		for(SclAvx2::block_t* block : item.second.Bits) {
			mDataPool->release(block);
		}
	}
	mPathList->clear();
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
	mPathList = new SclAvx2::PathList();
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
	int dataStage = __builtin_ctz(mBlockLength);
	clearPathList();
	SclAvx2::PathItem pi;
		pi.Llr.resize(dataStage+1, nullptr);
		pi.Bits.resize(pi.Llr.size(), nullptr);
		pi.Metric = 0;
		pi.Llr[dataStage] = mDataPool->allocate(SclAvx2::nBit2vecCount(mBlockLength));
	memcpy(pi.Llr[dataStage]->data, mLlrContainer->data(), mBlockLength);
	mPathList->insert(std::make_pair(pi.Metric, pi));
}

bool SclAvx2Char::extractBestPath() {
	int dataStage = __builtin_ctz(mBlockLength);
	SclAvx2::PathList::iterator pathIt = mPathList->begin();
	if(mSystematic) {
		mBitContainer->insertCharBits(pathIt->second.Bits[dataStage]->data);
		mBitContainer->getPackedInformationBits(mOutputContainer);
	}
	clearPathList();
	bool decoderSuccess = mErrorDetector->check(mOutputContainer, (mBlockLength-mFrozenBits.size()+7)/8);
	return decoderSuccess;
}
}// namespace Decoding
}// namespace PolarCode
