#include <polarcode/decoding/depth_first.h>
#include <polarcode/polarcode.h>
#include <polarcode/encoding/butterfly_fip_packed.h>
#include <list>
#include <algorithm>

namespace PolarCode {
namespace Decoding {

namespace DepthFirstObjects {

Manager::Manager(int trialLimit)
	: mTrialLimit(trialLimit)
{
	mNodeList.clear();
}

Manager::~Manager() {
}

void Manager::pushDecoder(Node *node) {
	mNodeList.push_back(node);
}

void Manager::setRootNode(Node *node) {
	xmRootNode = node;
}

void Manager::decode() {
	xmRootNode->decode();

	//mConfigList.clear();
	while(!mConfigList.empty())mConfigList.pop();

	//Create initial configurations, including the base that has just been decoded
	std::vector<DecoderHint> hintList;

	//Collect a new list of reliabilities and accumulate paths' total reliability
	hintList.clear();
	float metric = 0.0f;
	for(auto decoder : mNodeList) {
		metric += decoder->reliability();
		hintList.push_back({decoder, decoder->reliability()});
	}

	auto myCompareHints = [](const DecoderHint a, const DecoderHint b) -> bool { return a.reliability < b.reliability; };

	//Sort all nodes
	std::sort(hintList.begin(), hintList.end(), myCompareHints);
	firstRun = true;

	//Create configurations
	int nodeRank = 0;
	do {
		int optionCount = hintList[nodeRank].node->optionCount();
		Configuration conf;
		conf.depth = 0;
		conf.parentMetric = metric;
		conf.nodeList = hintList;
		if(nodeRank != 0) {
			std::rotate(conf.nodeList.begin(), conf.nodeList.begin()+nodeRank, conf.nodeList.begin()+nodeRank+1);
		}

		for(int i=(nodeRank==0?0:1); i<optionCount; ++i) {
			conf.nodeOptions.clear();
			conf.nodeOptions.push_back(i);//Set option for weakest node

			mConfigList.push(conf);
		}
		nodeRank++;
	} while((nodeRank < mTrialLimit*2/3 || hintList[nodeRank].reliability < log(9)) && mConfigList.size() < (unsigned)mTrialLimit);

	mBestMetric = metric;
	mBestConfig = mConfigList.front();
}


void Manager::decodeNext() {
	Configuration currentConfig = mConfigList.front();
	mConfigList.pop();
	int depth = currentConfig.depth;
	float metric = 0.0f;

	if(mConfigList.size() < (unsigned)mTrialLimit) {
		std::vector<DecoderHint> hintList;

		if(!firstRun) {
			//Collect a new list of reliabilities
			hintList.clear();
			for(auto hint : currentConfig.nodeList) {
				metric += hint.node->reliability();
				hintList.push_back({hint.node, hint.node->reliability()});
			}
			//Sort all nodes, excluding the current configuration
			//to prevent double configuring of already considered nodes
			auto myCompareHints = [](const DecoderHint a, const DecoderHint b) -> bool { return a.reliability < b.reliability; };
			std::sort(hintList.begin()+depth+1, hintList.end(), myCompareHints);
		} else {
			hintList = currentConfig.nodeList;
			metric = currentConfig.parentMetric;
			firstRun = false;
		}

		//Create new configurations based on changing the most unreliable node
		DecoderHint &weakestNode = hintList.at(depth+1);
		int optionCount = weakestNode.node->optionCount();

		for(int i=0; i<optionCount; ++i) {
			Configuration conf;
			conf.depth = depth+1;
			conf.parentMetric = metric;
			conf.nodeList = hintList;
			conf.nodeOptions = currentConfig.nodeOptions;
			conf.nodeOptions.push_back(i);
			mConfigList.push(conf);
		}
	} else {
		if(!firstRun) {
			for(auto hint : currentConfig.nodeList) {
				metric += hint.node->reliability();
			}
		} else {
			metric = currentConfig.parentMetric;
			firstRun = false;
		}
	}

	//Save current config, if it is better than previous ones
	if(metric > mBestMetric) {
		mBestConfig = currentConfig;
		mBestMetric = metric;
	}

	//Configure every node according to the next configuration
	const Configuration &newConfig = mConfigList.front();
	depth = newConfig.depth;
	for(int i=0; i <= depth; ++i) {
		newConfig.nodeList[i].node->setOption(newConfig.nodeOptions[i]);
	}

	//Decode
	xmRootNode->decode();
}

void Manager::decodeBestConfig() {
	int depth = mBestConfig.depth;
	for(int i=0; i <= depth; ++i) {
		mBestConfig.nodeList[i].node->setOption(mBestConfig.nodeOptions[i]);
	}

	//Decode
	xmRootNode->decode();
}


Node::Node()
	: mBlockLength(0)
	, xmDataPool(nullptr)
	, xmManager(nullptr)
	, mLlr(nullptr)
	, mBit(nullptr)
	, mInput(nullptr)
	, mOutput(nullptr)
	, mReliability(INFINITY)
	, mOptionCount(0)
	, mOption(0)
{
}

Node::Node(unsigned blockLength, datapool_t *pool, Manager *manager)
	: mBlockLength(blockLength)
	, xmDataPool(pool)
	, xmManager(manager)
	, mLlr(pool->allocate(blockLength))
	, mBit(pool->allocate(blockLength))
	, mInput(mLlr->data)
	, mOutput(mBit->data)
	, mReliability(INFINITY)
	, mOptionCount(0)
	, mOption(0)
{
}

Node::Node(Node *other)
	: mBlockLength(other->mBlockLength)
	, xmDataPool(other->xmDataPool)
	, xmManager(other->xmManager)
	, mLlr(xmDataPool->allocate(mBlockLength))
	, mBit(xmDataPool->allocate(mBlockLength))
	, mInput(other->mInput)
	, mOutput(other->mOutput)
	, mReliability(INFINITY)
	, mOptionCount(0)
	, mOption(0)
{
}

Node::~Node() {
	xmDataPool->release(mLlr);
	xmDataPool->release(mBit);
}

void Node::setInput(float *input) {
	mInput = input;
}

void Node::setOutput(float *output) {
	mOutput = output;
}

unsigned Node::blockLength() {
	return mBlockLength;
}

float* Node::input() {
	return mInput;
}

float* Node::output() {
	return mOutput;
}

void Node::decode() {
	//Should never be called
}

float Node::reliability() {
	return mReliability;
}

int Node::optionCount() {
	return mOptionCount;
}

void Node::setOption(int d) {
	mOption = d;
}


/*************
 * RateRNode
 * ***********/

RateRNode::RateRNode() {
}

RateRNode::RateRNode(const std::vector<unsigned> &frozenBits, Node *parent)
	: Node(parent)
{
	mBlockLength /= 2;

	mLeftLlr  = xmDataPool->allocate(mBlockLength);
	mRightLlr = xmDataPool->allocate(mBlockLength);

	std::vector<unsigned> leftFrozenBits, rightFrozenBits;
	splitFrozenBits(frozenBits, mBlockLength, leftFrozenBits, rightFrozenBits);

	mLeft = createDecoder(leftFrozenBits, this);
	mLeft->setInput(mLeftLlr->data);
	mLeft->setOutput(mOutput);

	mRight = createDecoder(rightFrozenBits, this);
	mRight->setInput(mRightLlr->data);
	mRight->setOutput(mOutput + mBlockLength);
}

RateRNode::~RateRNode() {
	delete mLeft;
	delete mRight;
	xmDataPool->release(mLeftLlr);
	xmDataPool->release(mRightLlr);
}

void RateRNode::setOutput(float *output) {
	mOutput = output;
	mLeft->setOutput(mOutput);
	mRight->setOutput(mOutput + mBlockLength);
}

void RateRNode::decode() {
	FastSscAvx::F_function(mInput, mLeftLlr->data, mBlockLength);
	mLeft->decode();
	FastSscAvx::G_function(mInput, mRightLlr->data, mOutput, mBlockLength);
	mRight->decode();
	FastSscAvx::Combine(mOutput, mBlockLength);
}

/*************
 * ShortRateRNode
 * ***********/

ShortRateRNode::ShortRateRNode(const std::vector<unsigned> &frozenBits, Node *parent)
	: RateRNode(frozenBits, parent)
{
	mLeftBits  = xmDataPool->allocate(mBlockLength);
	mRightBits = xmDataPool->allocate(mBlockLength);
	mLeft->setOutput(mLeftBits->data);
	mRight->setOutput(mRightBits->data);
}

ShortRateRNode::~ShortRateRNode() {
	xmDataPool->release(mLeftBits);
	xmDataPool->release(mRightBits);
}

void ShortRateRNode::setOutput(float *output) {
	mOutput = output;
}

void ShortRateRNode::decode() {
	FastSscAvx::F_function(mInput, mLeftLlr->data, mBlockLength);
	mLeft->decode();
	FastSscAvx::G_function(mInput, mRightLlr->data, mLeftBits->data, mBlockLength);
	mRight->decode();
	FastSscAvx::CombineBitsShort(mLeftBits->data, mRightBits->data, mOutput, mBlockLength);
}

/*************
 * RateZeroDecoder
 * ***********/

inline void memFloatFill(float *dst, float value, const size_t blockLength) {
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

RateZeroDecoder::RateZeroDecoder(Node *parent)
	: Node(parent) {
}

RateZeroDecoder::~RateZeroDecoder() {
}

void RateZeroDecoder::decode() {
	memFloatFill(mOutput, INFINITY, mBlockLength);
}

/*************
 * RateOneDecoder
 * ***********/

RateOneDecoder::RateOneDecoder(Node *parent)
	: Node(parent) {
	xmManager->pushDecoder(this);
	mOptionCount = /*mBlockLength == 1 ?*/ 2 /*: 4*/;
	mFlipIndices = new unsigned[mBlockLength];
	mTempBlock = xmDataPool->allocate(mBlockLength);
	mTempBlockPtr = mTempBlock->data;
}

RateOneDecoder::~RateOneDecoder() {
	delete [] mFlipIndices;
	xmDataPool->release(mTempBlock);
}

void RateOneDecoder::findWeakLlrs() {
	for(unsigned i=0; i<mBlockLength; ++i) {
		mFlipIndices[i] = i;
	}

//	unsigned limit = std::min(2u, mBlockLength);

//	for(unsigned bit=0; bit<limit; ++bit) {
		unsigned index = /*bit*/0;
		for(unsigned i=index+1; i<mBlockLength; ++i) {
			if(mTempBlockPtr[i] < mTempBlockPtr[index]) {
				index = i;
			}
		}
		std::swap(mTempBlockPtr[/*bit*/0], mTempBlockPtr[index]);
		std::swap(mFlipIndices[/*bit*/0], mFlipIndices[index]);
//	}
}

void RateOneDecoder::decode() {
	const __m256 sgnMask = _mm256_set1_ps(-0.0);

	for(unsigned i=0; i<mBlockLength; i+=8) {
		__m256 Llr = _mm256_load_ps(mInput+i);
		_mm256_store_ps(mOutput+i, Llr);

		Llr = _mm256_andnot_ps(sgnMask, Llr);
		_mm256_store_ps(mTempBlockPtr+i, Llr);
	}
	findWeakLlrs();
	mReliability = mTempBlockPtr[0];

	/* 0: Swap nothing
	 * 1: Swap first bit
	 * //2: Swap second bit
	 * //3: Swap both
	 */
	if(mOption/*&1*/) {
		mOutput[mFlipIndices[0]] = -mInput[mFlipIndices[0]];
	}

//	if(mBlockLength > 1 && (mOption & 2)) {
//		mOutput[mFlipIndices[1]] = -mInput[mFlipIndices[1]];
//	}

	//Reset option for next configuration
	mOption = 0;
}

/*************
 * RepetitionDecoder
 * ***********/

RepetitionDecoder::RepetitionDecoder(Node *parent)
	: Node(parent) {
	xmManager->pushDecoder(this);
	mOptionCount = 2;
}

RepetitionDecoder::~RepetitionDecoder() {
}


void RepetitionDecoder::decode() {
	__m256 vSum = _mm256_setzero_ps();
	float sum;

	for(unsigned i=mBlockLength; i<8; ++i) {
		mInput[i] = 0.0;//Neutral element of Repetition code
	}

	for(unsigned i=0; i<mBlockLength; i+=8) {
		__m256 Llr = _mm256_load_ps(mInput+i);
		vSum = _mm256_add_ps(vSum, Llr);
	}
	sum = reduce_add_ps(vSum);
	mReliability = std::abs(sum);

	if(mOption == 1) {
		sum = -sum;
	}

	if(mBlockLength >= 8) {
		vSum = _mm256_set1_ps(sum);
		for(unsigned i=0; i<mBlockLength; i+=8) {
			_mm256_store_ps(mOutput+i, vSum);
		}
	} else {
		for(unsigned i=0; i<mBlockLength; i++) {
			mOutput[i] = sum;
		}
	}

	//Reset option for next configuration
	mOption = 0;
}


/*************
 * SpcDecoder
 * ***********/

SpcDecoder::SpcDecoder(Node *parent)
	: Node(parent) {
	xmManager->pushDecoder(this);
	mOptionCount = 2;
	mFlipIndices = new unsigned[mBlockLength];
	mTempBlock = xmDataPool->allocate(mBlockLength);
	mTempBlockPtr = mTempBlock->data;
}

SpcDecoder::~SpcDecoder() {
	delete [] mFlipIndices;
	xmDataPool->release(mTempBlock);
}

void SpcDecoder::findWeakLlrs() {
	for(unsigned i=0; i<mBlockLength; ++i) {
		mFlipIndices[i] = i;
	}

	for(unsigned bit=0; bit<2; ++bit) {
		unsigned index = bit;
		for(unsigned i=index+1; i<mBlockLength; ++i) {
			if(mTempBlockPtr[i] < mTempBlockPtr[index]) {
				index = i;
			}
		}
		std::swap(mTempBlockPtr[bit], mTempBlockPtr[index]);
		std::swap(mFlipIndices[bit], mFlipIndices[index]);
	}
}

void SpcDecoder::decode() {
	const __m256 sgnMask = _mm256_set1_ps(-0.0);
	__m256 vParity = _mm256_setzero_ps();
	union {
		float fParity;
		unsigned int uParity;
	};

	for(unsigned i=mBlockLength; i<8; ++i) {
		mInput[i] = INFINITY;
	}

	for(unsigned i=0; i<mBlockLength; i+=8) {
		__m256 Llr = _mm256_load_ps(mInput+i);
		_mm256_store_ps(mOutput+i, Llr);
		vParity = _mm256_xor_ps(vParity, Llr);

		Llr = _mm256_andnot_ps(sgnMask, Llr);
		_mm256_store_ps(mTempBlockPtr+i, Llr);
	}
	findWeakLlrs();
	mReliability = mTempBlockPtr[0];

	fParity = reduce_xor_ps(vParity);

	if(uParity & 0x80000000) {
		//Flip one bit
		mOutput[mFlipIndices[mOption]] = -mInput[mFlipIndices[mOption]];
	} else {
		if(mOption) {
			//Flip two bits to keep SPC equation fulfilled
			mOutput[mFlipIndices[0]] = -mInput[mFlipIndices[0]];
			mOutput[mFlipIndices[1]] = -mInput[mFlipIndices[1]];
		}
	}

	//Reset option for next configuration
	mOption = 0;
}

Node* createDecoder(const std::vector<unsigned> &frozenBits, Node *parent) {
	unsigned blockLength = parent->blockLength();
	unsigned frozenBitCount = frozenBits.size();

	if(blockLength == 1) {
		if(frozenBitCount == blockLength) {
			return new RateZeroDecoder(parent);
		}

		if(frozenBitCount == 0) {
			return new RateOneDecoder(parent);
		}
	}
/*
	if(frozenBitCount == blockLength-1) {
		return new RepetitionDecoder(parent);
	}

	if(frozenBitCount == 1) {
		return new SpcDecoder(parent);
	}*/

	if(blockLength <= 8) {
		return new ShortRateRNode(frozenBits, parent);
	} else {
		return new RateRNode(frozenBits, parent);
	}
}

}// namespace DepthFirstObjects

DepthFirst::DepthFirst(size_t blockLength, size_t trialLimit, const std::vector<unsigned> &frozenBits) {
	mTrialLimit = trialLimit;
	initialize(blockLength, frozenBits);
}

DepthFirst::~DepthFirst() {
	clear();
}

void DepthFirst::clear() {
	delete mEncoder;
	delete mRootNode;
	delete mNodeBase;
	delete mDataPool;
}

void DepthFirst::initialize(size_t blockLength, const std::vector<unsigned> &frozenBits) {
	if(blockLength == mBlockLength && frozenBits == mFrozenBits) {
		return;
	}
	if(mBlockLength != 0) {
		clear();
	}
	mBlockLength = blockLength;
	mFrozenBits.assign(frozenBits.begin(), frozenBits.end());
	mEncoder = new Encoding::ButterflyFipPacked(mBlockLength, mFrozenBits);
	mEncoder->setSystematic(false);
	mDataPool = new DepthFirstObjects::datapool_t();
	mManager  = new DepthFirstObjects::Manager(mTrialLimit);
	mNodeBase = new DepthFirstObjects::Node(blockLength, mDataPool, mManager);
	mRootNode = DepthFirstObjects::createDecoder(frozenBits, mNodeBase);
	mLlrContainer = new FloatContainer(mNodeBase->input(),  mBlockLength);
	mBitContainer = new FloatContainer(mNodeBase->output(), mBlockLength);
	mLlrContainer->setFrozenBits(mFrozenBits);
	mBitContainer->setFrozenBits(mFrozenBits);
	mOutputContainer = new unsigned char[(mBlockLength-frozenBits.size()+7)/8];

	mManager->setRootNode(mRootNode);
}

bool DepthFirst::decode() {
	bool success = false;

	unsigned run = 1;
	mManager->decode();

	for(;;) {
		if(!mSystematic) {
			mEncoder->setFloatCodeword(dynamic_cast<FloatContainer*>(mBitContainer)->data());
			mEncoder->encode();
			mEncoder->getInformation(mOutputContainer);
		} else {
			mBitContainer->getPackedInformationBits(mOutputContainer);
		}

		success = mErrorDetector->check(mOutputContainer, (mBlockLength-mFrozenBits.size()+7)/8);

		if(!success && run < mTrialLimit) {
			++run;
			mManager->decodeNext();
		} else {
			break;
		}
	}

/*	if(!success && mTrialLimit > 1) {//Restore initial guess
		mManager->decodeBestConfig();
		if(!mSystematic) {
			encoder->setCodeword(dynamic_cast<FloatContainer*>(mBitContainer)->data());
			encoder->encode();
			encoder->getEncodedData(dynamic_cast<FloatContainer*>(mBitContainer)->data());
		}
		mBitContainer->getPackedInformationBits(mOutputContainer);
	}*/

	return success;
}


}// namespace Decoding
}// namespace PolarCode
