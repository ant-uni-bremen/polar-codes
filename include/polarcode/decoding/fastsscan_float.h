/* -*- c++ -*- */
/*
 * Copyright 2018 Florian Lotze
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#ifndef PC_DEC_FASTSSCAN_H
#define PC_DEC_FASTSSCAN_H

#include <polarcode/datapool.txx>
#include <polarcode/decoding/avx_float.h>
#include <polarcode/decoding/decoder.h>

namespace PolarCode {
namespace Decoding {

namespace FastSscanObjects {
typedef DataPool<float, 32> datapool_t;
typedef Block<float> block_t;

class Node
{
protected:
    unsigned mBlockLength;
    datapool_t* xmDataPool;

    block_t *mLlr, *mExt;
    float *mInput, *mOutput;

public:
    Node();
    Node(Node* other);
    Node(unsigned blockLength, datapool_t* pool);
    virtual ~Node();

    unsigned blockLength();

    virtual void reset();
    virtual void decode();

    void setInput(float*);
    void setOutput(float*);
    float* input();
    float* output();
};

class RateRNode : public Node
{
protected:
    Node *mLeft, *mRight;
    block_t *mLeftLlr, *mRightLlr;
    block_t *mLeftExt, *mRightExt;
    block_t* mTemp;

public:
    RateRNode(const std::vector<unsigned>& frozenBits, Node* parent);
    ~RateRNode();

    void reset();
    void decode();
};

class RateZeroNode : public Node
{
public:
    RateZeroNode(Node* parent);
    ~RateZeroNode();

    void reset();
    // void decode() = Node::decode() = NOP
};

class RateOneNode : public Node
{
public:
    RateOneNode(Node* parent);
    ~RateOneNode();

    void reset();
    // void decode() = Node::decode() = NOP
};

class RepetitionNode : public Node
{
public:
    RepetitionNode(Node* parent);
    ~RepetitionNode();

    void reset();
    void decode();
};

class TwoBitNode : public Node
{
    float mLeftLlr, mRightLlr;

public:
    TwoBitNode(Node* parent);
    ~TwoBitNode();

    void reset();
    void decode();
};


Node* createDecoder(const std::vector<unsigned>& frozenBits, Node* parent);

} // namespace FastSscanObjects

class FastSscanFloat : public Decoder
{
    FastSscanObjects::Node *mNodeBase, *mRootNode;
    FastSscanObjects::datapool_t* mDataPool;
    FastSscanObjects::block_t* mTemp;
    unsigned mTrialLimit;

    void clear();
    void calculateOutput();
    bool check();

public:
    FastSscanFloat(unsigned blockLength,
                   unsigned trialLimit,
                   const std::vector<unsigned>& frozenBits);
    ~FastSscanFloat();

    bool decode();
    bool decodeAgain();
    void initialize(unsigned blockLength, const std::vector<unsigned>& frozenBits);
};

} // namespace Decoding
} // namespace PolarCode

#endif // PC_DEC_FASTSSCAN_H
