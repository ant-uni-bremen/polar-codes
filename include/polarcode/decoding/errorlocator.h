/* -*- c++ -*- */
/*
 * Copyright 2018 Florian Lotze
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#ifndef PC_DEC_ERRLOC_H
#define PC_DEC_ERRLOC_H

#include <polarcode/decoding/decoder.h>
#include <polarcode/datapool.txx>

namespace PolarCode {
namespace Decoding {

class ErrorLocator;

namespace ErrorLocatorNodes {

typedef DataPool<float, 32> datapool_t;
typedef Block<float> block_t;

class Node {
protected:
	Decoding::ErrorLocator *xmDecoder;
	unsigned mBlockLength;
	datapool_t *xmDataPool;
	block_t *mLlr, *mBit;
	float *mInput, *mOutput;

public:
	int mId;
	float mDesiredValue, mValue;
	bool mReplace;

	Node();
	Node(Node *other);
	Node(unsigned blockLength, datapool_t *pool, Decoding::ErrorLocator *decoder);
	virtual ~Node();

	virtual void decode();

	unsigned blockLength();
	Decoding::ErrorLocator* decoder();

	void setInput(float *);
	virtual void setOutput(float *);

	float* input();
	float* output();
};


class RateRNode : public Node {
protected:
	Node *mLeft,
		 *mRight;
	block_t *mLeftLlr, *mRightLlr;

public:
	RateRNode(const std::vector<unsigned> &frozenBits, Node *parent);
	~RateRNode();
	void setOutput(float *);
	void decode();
};

class ShortRateRNode : public RateRNode {
	block_t *mLeftBits, *mRightBits;

public:
	ShortRateRNode(const std::vector<unsigned> &frozenBits, Node *parent);
	~ShortRateRNode();
	void setOutput(float *);
	void decode();
};

class Bit : public Node {
public:
	Bit(Node *parent);
	~Bit();
	void decode();
};

Node* createDecoder(const std::vector<unsigned> &frozenBits, Node* parent);

}// namespace ErrorLocatorNodes

class ErrorLocator : public Decoder {
	ErrorLocatorNodes::Node
		*mNodeBase,
		*mRootNode;
	ErrorLocatorNodes::datapool_t *mDataPool;

	std::vector<ErrorLocatorNodes::Node*> mSystematicNodes;
	std::vector<ErrorLocatorNodes::Node*> mInfoBits;

	int mFirstError;
	int mCorrectionCount;

	bool mIsReferenceDecoder;

	void prepare();
	bool findErrors();
	int findFirstError();

public:
	ErrorLocator(unsigned blockLength, const std::vector<unsigned> &frozenBits);
	~ErrorLocator();

	bool decode();
	void initialize(unsigned blockLength, const std::vector<unsigned> &frozenBits);
	void clear();

	int decodeFindFirstError();

	void pushBit(ErrorLocatorNodes::Node *node, bool frozen);

	std::vector<float> getOutput();
	void setDesiredOutput(std::vector<float>);

	void setAsReferenceDecoder();

	int firstError();
	int correctionCount();

};

}// namespace Decoding
}// namespace PolarCode


#endif
