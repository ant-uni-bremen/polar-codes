/* -*- c++ -*- */
/*
 * Copyright 2018 Florian Lotze
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#ifndef PC_DEC_FASTSSC_AVX_FLOAT_H
#define PC_DEC_FASTSSC_AVX_FLOAT_H

#include <polarcode/datapool.txx>
#include <polarcode/decoding/avx_float.h>
#include <polarcode/decoding/decoder.h>
#include <polarcode/encoding/encoder.h>

namespace PolarCode {
namespace Decoding {

namespace FastSscAvx {

typedef DataPool<float, 32> datapool_t;
typedef Block<float> block_t;

/*!
 * \brief A node of the polar decoding tree.
 */
class Node
{
protected:
    unsigned mBlockLength;  ///< Length of the subcode.
    datapool_t* xmDataPool; ///< Pointer to a DataPool object.
    block_t *mLlr, *mBit;
    float *mInput, *mOutput;


public:
    Node();
    Node(Node* other);
    /*!
     * \brief Initialize a polar code's root node
     * \param blockLength Length of the code.
     * \param pool Pointer to a DataPool, which provides lazy-copyable memory blocks.
     */
    Node(size_t blockLength, datapool_t* pool);
    virtual ~Node();

    virtual void decode(); ///< Execute a specialized decoding algorithm.

    void setInput(float*);
    virtual void setOutput(float*);

    /*!
     * \brief Get a pointer to the datapool.
     * \return A pointer to the datapool.
     */
    datapool_t* pool();

    /*!
     * \brief Get the length of this code node.
     * \return The length of this node.
     */
    unsigned blockLength();

    /*!
     * \brief Get a pointer to LLR values of this node.
     * \return Pointer to LLRs.
     */
    float* input();

    /*!
     * \brief Get a pointer to bits of this node.
     * \return Pointer to bit storage.
     */
    float* output();
};

enum ChildCreationFlags { BOTH, NO_LEFT = 0x01, NO_RIGHT = 0x02 };

/*!
 * \brief A Rate-R node redirects decoding to polar subcodes of lower complexity.
 */
class RateRNode : public Node
{
protected:
    Node *mLeft, ///< Left child node
        *mRight; ///< Right child node
    block_t *mLeftLlr,
        *mRightLlr; ///< Temporarily holds the LLRs child nodes have to decode.

public:
    /*!
     * \brief Using the set of frozen bits, specialized subcodes are selected.
     * \param frozenBits The set of frozen bits of this code.
     * \param parent The parent node, defining the length of this code.
     * \param flags Set to [NO_LEFT | NO_RIGHT] to disable child creation.
     */
    RateRNode(const std::vector<unsigned>& frozenBits,
              Node* parent,
              ChildCreationFlags flags = BOTH);
    virtual ~RateRNode();
    void setOutput(float*);
    void decode();
};

/*!
 * \brief A rate-R node of subvector length needs child bits in separate blocks.
 */
class ShortRateRNode : public RateRNode
{
    block_t *mLeftBits, *mRightBits;

public:
    /*!
     * \brief Using the set of frozen bits, specialized subcodes are selected.
     * \param frozenBits The set of frozen bits of this code.
     * \param parent The parent node, defining the length of this code.
     */
    ShortRateRNode(const std::vector<unsigned>& frozenBits, Node* parent);
    virtual ~ShortRateRNode();
    void setOutput(float*);
    void decode();
};

class RateZeroDecoder : public Node
{
public:
    RateZeroDecoder(Node* parent);
    ~RateZeroDecoder();
    void decode();
};

class RateOneDecoder : public Node
{
public:
    RateOneDecoder(Node* parent);
    ~RateOneDecoder();
    void decode();
};

class RepetitionDecoder : public Node
{
public:
    RepetitionDecoder(Node* parent);
    ~RepetitionDecoder();
    void decode();
};

class DoubleRepetitionDecoder : public Node
{
public:
    DoubleRepetitionDecoder(Node* parent);
    ~DoubleRepetitionDecoder();
    void decode();
};

class SpcDecoder : public Node
{
    unsigned* mFlipIndices;
    block_t* mTempBlock;
    float* mTempBlockPtr;
    void findWeakLlr();

public:
    SpcDecoder(Node* parent);
    ~SpcDecoder();
    void decode();
};

class DoubleSpcDecoder : public Node
{
    unsigned* mFlipIndices;
    block_t* mTempBlock;
    float* mTempBlockPtr;

public:
    DoubleSpcDecoder(Node* parent);
    ~DoubleSpcDecoder();
    void decode();
};

class ZeroSpcDecoder : public Node
{
    unsigned* mFlipIndices;
    block_t* mTempBlock;
    float* mTempBlockPtr;
    void findWeakLlr();

public:
    ZeroSpcDecoder(Node* parent);
    ~ZeroSpcDecoder();
    void decode();
};

/*!
 * \brief Optimized decoding, if the right subcode is rate-1.
 */
class ROneNode : public RateRNode
{
    void rightDecode();

public:
    /*!
     * \brief Initialize the right-rate-1 optimized decoder.
     * \param frozenBits The set of frozen bits of both subcodes (apparently only left
     * subcode has frozen bits). \param parent The parent node.
     */
    ROneNode(const std::vector<unsigned>& frozenBits, Node* parent);
    ~ROneNode();
    void decode();
};

/*!
 * \brief Optimized decoding, if the left subcode is rate-0.
 */
class ZeroRNode : public RateRNode
{
public:
    /*!
     * \brief Initialize the left-rate-0 optimized decoder.
     * \param frozenBits The set of frozen bits for both subcodes.
     * \param parent The parent node.
     */
    ZeroRNode(const std::vector<unsigned>& frozenBits, Node* parent);
    ~ZeroRNode();
    void decode();
};

/*!
 * \brief Create a specialized decoder for the given set of frozen bits.
 * \param frozenBits The set of frozen bits.
 * \param parent The parent node from which the code length is fetched.
 * \return Pointer to a polymorphic decoder object.
 */
Node* createDecoder(const std::vector<unsigned>& frozenBits, Node* parent);

} // namespace FastSscAvx

/*!
 * \brief The recursive systematic Fast-SSC decoder.
 */
class FastSscAvxFloat : public Decoder
{
    FastSscAvx::Node *mNodeBase,       ///< General code information
        *mRootNode;                    ///< Actual decoder
    FastSscAvx::datapool_t* mDataPool; ///< Lazy-copy data-block pool
    Encoding::Encoder* mEncoder;

    void clear();

public:
    /*!
     * \brief Create a Fast-SSC decoder with AVX float-bit decoding.
     * \param blockLength Length of the Polar Code.
     * \param frozenBits Set of frozen bits in the code word.
     */
    FastSscAvxFloat(size_t blockLength, const std::vector<unsigned>& frozenBits);
    ~FastSscAvxFloat();

    bool decode();
    void initialize(size_t blockLength, const std::vector<unsigned>& frozenBits);
};

} // namespace Decoding
} // namespace PolarCode

#endif // PC_DEC_FASTSSC_AVX_FLOAT_H
