#ifndef PC_DEC_FASTSSC_AVX2_H
#define PC_DEC_FASTSSC_AVX2_H

#include <polarcode/decoding/decoder.h>
#include <polarcode/encoding/butterfly_avx2_char.h>
#include <polarcode/datapool.txx>

namespace PolarCode {
namespace Decoding {

namespace FastSscAvx2 {

/*!
 * \brief A node of the polar decoding tree.
 */
class Node {
	typedef DataPool<__m256i, 32> datapool_t;
	Block<__m256i> *mLlr, *mBit;

	void clearBlocks();
	void clearLlrBlock();
	void clearBitBlock();

protected:
	datapool_t *xmDataPool;//xm = eXternal member (not owned by this Node)
	size_t mBlockLength,
		   mVecCount;

	void setInput(Block<__m256i> *newInputBlock);
	void setOutput(Block<__m256i> *newOutputBlock);
	void unsetBlockPointers();

public:
	Node();
	/*!
	 * \brief Initialize a polar code's root node
	 * \param blockLength Length of the code.
	 * \param pool Pointer to a DataPool, which provides lazy-copyable memory blocks.
	 */
	Node(size_t blockLength, datapool_t *pool);
	virtual ~Node();

	virtual void decode();///< Execute a specialized decoding algorithm.

	/*!
	 * \brief Get a pointer to the datapool.
	 * \return A pointer to the datapool.
	 */
	datapool_t* pool();

	/*!
	 * \brief Get the length of this code node.
	 * \return The length of this node.
	 */
	size_t blockLength();

	/*!
	 * \brief Get a pointer to LLR values of this node.
	 * \return Pointer to LLRs.
	 */
	__m256i* input();

	/*!
	 * \brief Get a pointer to bits of this node.
	 * \return Pointer to bit storage.
	 */
	__m256i* output();

};

class Preparer {
public:
	Preparer();
	virtual ~Preparer();
	virtual void prepare(__m256i *x);
};

class RepetitionPrep : public Preparer {
	unsigned mCodeLength;
public:
	RepetitionPrep(size_t codeLength);
	~RepetitionPrep();
	void prepare(__m256i *x);
};

class SpcPrep : public Preparer {
	unsigned mCodeLength;
public:
	SpcPrep(size_t codeLength);
	~SpcPrep();
	void prepare(__m256i *x);
};

/*!
 * \brief A Rate-R node redirects decoding to polar subcodes of lower complexity.
 */
class RateRNode : public Node {
	Node *mParent;
	Node *mLeft, *mRight;

	Block<__m256i> *ChildLlr, *LeftBits, *RightBits;

	void F_function_calc(__m256i &Left, __m256i &Right, __m256i *Out);
	void G_function_calc(__m256i &Left, __m256i &Right, __m256i &Bits, __m256i *Out);

	void F_function(__m256i *LLRin, __m256i *LLRout);
	void G_function(__m256i *LLRin, __m256i *LLRout, __m256i *BitsIn);
	void Combine(__m256i *Left, __m256i *Right, __m256i *Out);

public:
	/*!
	 * \brief Using the set of frozen bits, specialized subcodes are selected.
	 * \param frozenBits The set of frozen bits of this code.
	 * \param parent The parent node, defining the length of this code.
	 */
	RateRNode(std::set<unsigned> &frozenBits, Node *parent);
	~RateRNode();
	void decode();
};

class RateZeroNode : public Node {
	Node *mParent;
public:
	RateZeroNode(Node* parent);
	~RateZeroNode();
	void decode();
};

class RateOneNode : public Node {
	Node *mParent;
public:
	RateOneNode(Node* parent);
	~RateOneNode();
	void decode();
};

class RepetitionNode : public Node {
	Node *mParent;
	Preparer *mPreparer;
public:
	RepetitionNode(Node* parent);
	~RepetitionNode();
	void decode();
};

class SpcNode : public Node {
	Node *mParent;
	Preparer *mPreparer;
public:
	SpcNode(Node* parent);
	~SpcNode();
	void decode();
};

/*!
 * \brief Create a specialized decoder for the given set of frozen bits.
 * \param frozenBits The set of frozen bits.
 * \param parent The parent node from which the code length is fetched.
 * \return Pointer to a polymorphic decoder object.
 */
Node* createDecoder(std::set<unsigned> frozenBits, Node* parent);

/*!
 * \brief Convert block length to minimum AVX-vector count.
 * \param blockLength Bits to store
 * \return The number of AVX-vectors required to store _blockLength_ char bits.
 */
size_t nBit2vecCount(size_t blockLength);

}// namespace FastSscAvx2

/*!
 * \brief The recursive systematic Fast-SSC decoder.
 */
class FastSscAvx2Char : public Decoder {
	FastSscAvx2::Node *mNodeBase,///< General code information
					  *mRootNode;///< Actual decoder
	DataPool<__m256i, 32> *mDataPool;///< Lazy-copy data-block pool

	void clear();

public:
	FastSscAvx2Char(size_t blockLength, const std::set<unsigned> &frozenBits);
	~FastSscAvx2Char();

	bool decode();
	void initialize(size_t blockLength, const std::set<unsigned> &frozenBits);
};

}//namespace Decoding
}//namespace PolarCode

#endif //PC_DEC_FASTSSC_AVX2_H
