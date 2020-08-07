/* -*- c++ -*- */
/*
 * Copyright 2018 Florian Lotze
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#include <polarcode/decoding/fastsscan_float.h>
#include <polarcode/polarcode.h>

namespace PolarCode {
namespace Decoding {

namespace FastSscanObjects {

Node::Node() {
}

Node::Node(unsigned blockLength, datapool_t *pool)
	: mBlockLength(blockLength)
	, xmDataPool(pool)
	, mLlr(pool->allocate(blockLength))
	, mExt(pool->allocate(blockLength))
	, mInput(mLlr->data)
	, mOutput(mExt->data)
{
}

Node::Node(Node *other)
	: mBlockLength(other->mBlockLength)
	, xmDataPool(other->xmDataPool)
	, mLlr(nullptr)
	, mExt(nullptr)
	, mInput(other->mInput)
	, mOutput(other->mOutput)
{
}

Node::~Node() {
	xmDataPool->release(mLlr);
	xmDataPool->release(mExt);
}

unsigned Node::blockLength() {
	return mBlockLength;
}

void Node::reset() {
	memFloatFill(mExt->data, 0.0f, mBlockLength);
}

void Node::decode() {
	// No-op
}

void Node::setInput(float *input) {
	mInput = input;
}

void Node::setOutput(float *output) {
	mOutput = output;
}

float* Node::input() {
	return mInput;
}

float* Node::output() {
	return mOutput;
}

/*************
 * RateRNode
 * ***********/

RateRNode::RateRNode(const std::vector<unsigned> &frozenBits, Node *parent)
	: Node(parent)
{
	mBlockLength /= 2;

	mLeftLlr  = xmDataPool->allocate(mBlockLength);
	mRightLlr = xmDataPool->allocate(mBlockLength);
	mLeftExt  = xmDataPool->allocate(mBlockLength);
	mRightExt = xmDataPool->allocate(mBlockLength);
	mTemp     = xmDataPool->allocate(mBlockLength);

	std::vector<unsigned> leftFrozenBits, rightFrozenBits;
	splitFrozenBits(frozenBits, mBlockLength, leftFrozenBits, rightFrozenBits);

	mLeft = createDecoder(leftFrozenBits, this);
	mLeft->setInput(mLeftLlr->data);
	mLeft->setOutput(mLeftExt->data);

	mRight = createDecoder(rightFrozenBits, this);
	mRight->setInput(mRightLlr->data);
	mRight->setOutput(mRightExt->data);
}

RateRNode::~RateRNode() {
	delete mLeft;
	delete mRight;
	xmDataPool->release(mLeftLlr);
	xmDataPool->release(mRightLlr);
	xmDataPool->release(mLeftExt);
	xmDataPool->release(mRightExt);
}

inline void addVectors(float *a, float *b, float *dst, unsigned length) {
	__m256 va, vb, vsum;
	for(unsigned i = 0; i < length; i += 8) {
		va = _mm256_load_ps(a + i);
		vb = _mm256_load_ps(b + i);
		vsum = _mm256_add_ps(va, vb);
		_mm256_store_ps(dst + i, vsum);
	}
}

inline void addVector(float *a, float *b, float *dst, unsigned length) {
	for(unsigned i = 0; i < length; i++) {
		dst[i] = a[i] + b[i];
	}
}

void RateRNode::decode() {
	if(mBlockLength < 8) {
		// F-function
		addVector(mRightExt->data, mInput + mBlockLength, mTemp->data, mBlockLength);
		FastSscAvx::boxplus(mTemp->data, mInput, mLeftLlr->data, mBlockLength);

		// Left decoder
		mLeft->decode();

		// G-function
		FastSscAvx::boxplus(mLeftExt->data, mInput, mTemp->data, mBlockLength);
		addVector(mTemp->data, mInput + mBlockLength, mRightLlr->data, mBlockLength);

		// Right decoder
		mRight->decode();

		// C-function
		//  Upper half
		addVector(mRightExt->data, mInput + mBlockLength, mTemp->data, mBlockLength);
		FastSscAvx::boxplus(mLeftExt->data, mTemp->data, mOutput, mBlockLength);
		//  Lower half
		FastSscAvx::boxplus(mLeftExt->data, mInput, mTemp->data, mBlockLength);
		addVector(mRightExt->data, mTemp->data, mOutput + mBlockLength, mBlockLength);
	} else {
		// F-function
		addVectors(mRightExt->data, mInput + mBlockLength, mTemp->data, mBlockLength);
		FastSscAvx::boxplusVectors(mTemp->data, mInput, mLeftLlr->data, mBlockLength);

		// Left decoder
		mLeft->decode();

		// G-function
		FastSscAvx::boxplusVectors(mLeftExt->data, mInput, mTemp->data, mBlockLength);
		addVectors(mTemp->data, mInput + mBlockLength, mRightLlr->data, mBlockLength);

		// Right decoder
		mRight->decode();

		// C-function
		//  Upper half
		addVectors(mRightExt->data, mInput + mBlockLength, mTemp->data, mBlockLength);
		FastSscAvx::boxplusVectors(mLeftExt->data, mTemp->data, mOutput, mBlockLength);
		//  Lower half
		FastSscAvx::boxplusVectors(mLeftExt->data, mInput, mTemp->data, mBlockLength);
		addVectors(mRightExt->data, mTemp->data, mOutput + mBlockLength, mBlockLength);
	}
}

void RateRNode::reset() {
	memFloatFill(mOutput, 0.0f, 2 * mBlockLength);
	mLeft->reset();
	mRight->reset();
}

/*************
 * RateZeroNode
 * ***********/

RateZeroNode::RateZeroNode(Node *parent)
	: Node(parent) {
}

RateZeroNode::~RateZeroNode() {
}

void RateZeroNode::reset() {
	memFloatFill(mOutput, INFINITY, mBlockLength);
}

/*************
 * RateOneNode
 * ***********/

RateOneNode::RateOneNode(Node *parent)
	: Node(parent) {
}

RateOneNode::~RateOneNode() {
}

void RateOneNode::reset() {
	memFloatFill(mOutput, 0.0f, mBlockLength);
}

/*************
 * RepetitionNode
 * ***********/

RepetitionNode::RepetitionNode(Node *parent)
	: Node(parent)
{
}

RepetitionNode::~RepetitionNode() {
}

void RepetitionNode::reset() {
	memFloatFill(mOutput, 0.0f, mBlockLength);
}

void RepetitionNode::decode() {
	if(mBlockLength < 8) {
		float sum = 0.0f;
		for(unsigned i = 0; i < mBlockLength; ++i) {
			sum += mInput[i];
		}
		for(unsigned i = 0; i < mBlockLength; ++i) {
			mOutput[i] = sum - mInput[i];
		}
	} else {
		__m256 sum = _mm256_setzero_ps();
		for(unsigned i = 0; i < mBlockLength; i += 8) {
			sum = _mm256_add_ps(sum, _mm256_load_ps(mInput + i));
		}
		sum = _mm256_set1_ps(reduce_add_ps(sum));
		for(unsigned i = 0; i < mBlockLength; i += 8) {
			__m256 extr = _mm256_sub_ps(sum, _mm256_load_ps(mInput + i));
			_mm256_store_ps(mOutput + i, extr);
		}
	}
}

TwoBitNode::TwoBitNode(Node *parent)
	: Node(parent) {
}

TwoBitNode::~TwoBitNode() {
}

void TwoBitNode::reset() {
}

void TwoBitNode::decode() {
	mOutput[0] = mInput[1];
	mOutput[1] = mInput[0];
}


Node* createDecoder(const std::vector<unsigned> &frozenBits, Node *parent) {
	unsigned blockLength = parent->blockLength();
	unsigned frozenBitCount = frozenBits.size();

	if(frozenBitCount == blockLength) {
		return new RateZeroNode(parent);
	}

	if(frozenBitCount == 0) {
		return new RateOneNode(parent);
	}

	if(blockLength == 2) {
		return new TwoBitNode(parent);
	}

	if(frozenBitCount == blockLength-1) {
		return new RepetitionNode(parent);
	}

	/* To do
	if(frozenBitCount == 1) {
		return new SpcDecoder(parent);
	}*/

	return new RateRNode(frozenBits, parent);
}

}// namespace FastSscanObjects

FastSscanFloat::FastSscanFloat(unsigned blockLength, unsigned trialLimit, const std::vector<unsigned> &frozenBits) {
	mBlockLength = 0;
	mTrialLimit = trialLimit;
	initialize(blockLength, frozenBits);
}

FastSscanFloat::~FastSscanFloat() {
	clear();
}

void FastSscanFloat::clear() {
	mDataPool->release(mTemp);
	delete mRootNode;
	delete mNodeBase;
	delete mDataPool;
}

void FastSscanFloat::initialize(unsigned blockLength, const std::vector<unsigned> &frozenBits) {
	if(blockLength == mBlockLength && frozenBits == mFrozenBits) {
		return;
	}
	if(mBlockLength != 0) {
		clear();
	}
	mBlockLength = blockLength;
	mFrozenBits.assign(frozenBits.begin(), frozenBits.end());
	mDataPool = new FastSscanObjects::datapool_t();
	mNodeBase = new FastSscanObjects::Node(blockLength, mDataPool);
	mRootNode = FastSscanObjects::createDecoder(frozenBits, mNodeBase);
	mLlrContainer = new FloatContainer(mNodeBase->input(), mBlockLength);
	mLlrContainer->setFrozenBits(mFrozenBits);
	mBitContainer = new FloatContainer(mBlockLength, mFrozenBits);
	mOutputContainer = new unsigned char[(mBlockLength - frozenBits.size() + 7) / 8];
	mTemp = mDataPool->allocate(mBlockLength);
}

bool FastSscanFloat::decode() {
	bool success = false;
	mRootNode->reset();

	for(unsigned trial = 0; trial < mTrialLimit && !success; ++trial) {
		success = decodeAgain();
	}
	return success;
}

bool FastSscanFloat::decodeAgain() {
	mRootNode->decode();
	calculateOutput();
	return check();
}

void FastSscanFloat::calculateOutput() {
	FastSscanObjects::addVectors(
				mNodeBase->input(),	mNodeBase->output(),
				mTemp->data, mBlockLength);
	mBitContainer->insertLlr(mTemp->data);
	mBitContainer->getPackedInformationBits(mOutputContainer);
}

bool FastSscanFloat::check() {
	return mErrorDetector->check(mOutputContainer,
								 (mBlockLength - mFrozenBits.size() + 7) / 8);
}

}// namespace Decoding
}// namespace PolarCode
