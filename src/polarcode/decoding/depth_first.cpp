#include <polarcode/decoding/depth_first.h>
#include <polarcode/polarcode.h>
#include <polarcode/encoding/butterfly_avx2_packed.h>
#include <list>

namespace PolarCode {
namespace Decoding {

namespace DepthFirstObjects {

Manager::Manager() {
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
	for(Node* node : mNodeList) {
		node->resetDecision();
	}
	xmRootNode->decode();

	//Collect reliabilities in ascending order
	mHintList.clear();
	for(Node* decoder : mNodeList) {
		mHintList.push_back({decoder, decoder->reliability()});
	}
	std::sort(mHintList.begin(), mHintList.end(), compareHints);
}



void Manager::decodeNext() {
	//Select next codeword by counter-like switching of decisions
	for(DecoderHint hint : mHintList) {
		if(hint.node->nextDecision()) {
			break;
		}
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
	, mOption(0)
{
}

Node::~Node() {
	if(mLlr != nullptr) {
		xmDataPool->release(mLlr);
	}
	if(mBit != nullptr) {
		xmDataPool->release(mBit);
	}
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

bool Node::nextDecision() {
	//mOption++;
	return false;
}

void Node::resetDecision() {
	mOption = 0;
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
	FastSscAvx::CombineSoft(mOutput, mBlockLength);
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
	FastSscAvx::CombineSoftBitsShort(mLeftBits->data, mRightBits->data, mOutput, mBlockLength);
}

/*************
 * RateZeroDecoder
 * ***********/

/*inline void memFloatFill(float *dst, float value, const size_t blockLength) {
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
}*/

RateZeroDecoder::RateZeroDecoder(Node *parent)
	: Node(parent) {
}

RateZeroDecoder::~RateZeroDecoder() {
}

void RateZeroDecoder::decode() {
	mOutput[0] = INFINITY;
	//To be extended for longer codes
}

/*************
 * RateOneDecoder
 * ***********/

RateOneDecoder::RateOneDecoder(Node *parent)
	: Node(parent) {
	xmManager->pushDecoder(this);
}

RateOneDecoder::~RateOneDecoder() {
}

void RateOneDecoder::decode() {
	if(mOption) {
		mOutput[0] = -mInput[0];
	} else {
		mOutput[0] = mInput[0];
	}
	mReliability = std::fabs(mInput[0]);
}

bool RateOneDecoder::nextDecision() {
	if(mOption == 1) {
		mOption = 0;
		return false;
	} else {
		mOption = 1;
		return true;
	}
}


Node* createDecoder(const std::vector<unsigned> &frozenBits, Node *parent) {
	unsigned blockLength = parent->blockLength();
	unsigned frozenBitCount = frozenBits.size();

/* As long as we are restricted to single bits, split the code before detecting
 * long Rate-0 or Rate-1 codes.
 */
	if(blockLength > 1) {
		if(blockLength <= 8) {
			return new ShortRateRNode(frozenBits, parent);
		} else {
			return new RateRNode(frozenBits, parent);
		}
	}

	if(frozenBitCount == blockLength) {
		return new RateZeroDecoder(parent);
	}

/* To do:
	if(frozenBitCount == blockLength-1) {
		return new RepetitionDecoder(parent);
	}

	if(frozenBitCount == 1) {
		return new SpcDecoder(parent);
	}
*/

	if(frozenBitCount == 0) {
		return new RateOneDecoder(parent);
	}

/* Used, when long codes are implemented
	if(blockLength <= 8) {
		return new ShortRateRNode(frozenBits, parent);
	} else {
		return new RateRNode(frozenBits, parent);
	}
*/
	return nullptr;//Should not be reached, but inserted until everything is ready
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
	mDataPool = new DepthFirstObjects::datapool_t();
	mManager  = new DepthFirstObjects::Manager();
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

	Encoding::Encoder* encoder = nullptr;
	if(!mSystematic) {
		Encoding::Encoder* encoder = new Encoding::ButterflyAvx2Packed(mBlockLength);
		encoder->setSystematic(false);
	}

	unsigned run = 1;
	mManager->decode();

	for(;;) {
		if(!mSystematic) {
			encoder->setCodeword(dynamic_cast<FloatContainer*>(mBitContainer)->data());
			encoder->encode();
			encoder->getEncodedData(dynamic_cast<FloatContainer*>(mBitContainer)->data());
		}
		mBitContainer->getPackedInformationBits(mOutputContainer);
		success = mErrorDetector->check(mOutputContainer, (mBlockLength-mFrozenBits.size()+7)/8);

		if(!success && run < mTrialLimit) {
			++run;
			mManager->decodeNext();
		} else {
			break;
		}
	}
	if(encoder != nullptr) {
		delete encoder;
	}
	return success;
}


}// namespace Decoding
}// namespace PolarCode
