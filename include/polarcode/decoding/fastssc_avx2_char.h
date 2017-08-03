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
	//xm = eXternal member (not owned by this Node)
	datapool_t *xmDataPool;///< Pointer to a DataPool object.
	size_t mBlockLength,   ///< Length of the subcode.
		   mVecCount;      ///< Number of AVX-vectors the data can be stored in.

public:
	Node();
	/*!
	 * \brief Initialize a polar code's root node
	 * \param blockLength Length of the code.
	 * \param pool Pointer to a DataPool, which provides lazy-copyable memory blocks.
	 */
	Node(size_t blockLength, datapool_t *pool);
	virtual ~Node();

	virtual void decode(__m256i *LlrIn, __m256i *BitsOut);///< Execute a specialized decoding algorithm.

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

/*!
 * \brief The Preparer may set unused bits in an AVX-vector to neutral values.
 *
 * This base class itself is a dummy object. The prepare()-function is empty.
 * Derived classes can be specialized to set the value that is neutral to a
 * specific decoder, at positions which are unused for subvector operation.
 * Subvector operations happen, when the Polar Code subject to decoding is
 * shorter than the AVX-vector size of 32 char-bits.
 */
class Preparer {
public:
	Preparer();
	virtual ~Preparer();

	/*!
	 * \brief Prepare the given vector for correct decoding.
	 * \param x Pointer to the char-bit vector.
	 */
	virtual void prepare(__m256i *x);
};

/*!
 * \brief The preparer-class to neutralize values for repetition decoder.
 */
class RepetitionPrep : public Preparer {
	unsigned mCodeLength;
public:
	RepetitionPrep(size_t codeLength);///< Create a repetition preparer for given subcode length.
	~RepetitionPrep();
	void prepare(__m256i *x);
};

/*!
 * \brief The preparer-class to neutralize values for single parity check
 *        decoder.
 */
class SpcPrep : public Preparer {
	unsigned mCodeLength;
public:
	SpcPrep(size_t codeLength);///< Create an SPC preparer for given subcode length.
	~SpcPrep();
	void prepare(__m256i *x);
};

void F_function_calc(__m256i &Left, __m256i &Right, __m256i *Out);
void G_function_calc(__m256i &Left, __m256i &Right, __m256i &Bits, __m256i *Out);

void F_function(__m256i *LLRin, __m256i *LLRout, unsigned subBlockLength);
void G_function(__m256i *LLRin, __m256i *LLRout, __m256i *BitsIn, unsigned subBlockLength);

void Combine(__m256i *Bits, const unsigned vecCount);
void CombineShortBits(__m256i *Left, __m256i *Right, __m256i *Out, const unsigned subBlockLength);

/*!
 * \brief A Rate-R node redirects decoding to polar subcodes of lower complexity.
 */
class RateRNode : public Node {
protected:
	Node *mParent;///< The parent node
	Node *mLeft,///< Left child node
		 *mRight;///< Right child node

	Block<__m256i> *ChildLlr;///< Temporarily holds the LLRs child nodes have to decode.

public:
	/*!
	 * \brief Using the set of frozen bits, specialized subcodes are selected.
	 * \param frozenBits The set of frozen bits of this code.
	 * \param parent The parent node, defining the length of this code.
	 */
	RateRNode(std::set<unsigned> &frozenBits, Node *parent);
	~RateRNode();
	void decode(__m256i *LlrIn, __m256i *BitsOut);
};

/*!
 * \brief A rate-R node of subvector length needs child bits in separate blocks.
 */
class ShortRateRNode : public RateRNode {
	Block<__m256i> *LeftBits, *RightBits;

public:
	/*!
	 * \brief Using the set of frozen bits, specialized subcodes are selected.
	 * \param frozenBits The set of frozen bits of this code.
	 * \param parent The parent node, defining the length of this code.
	 */
	ShortRateRNode(std::set<unsigned> &frozenBits, Node *parent);
	~ShortRateRNode();
	void decode(__m256i *LlrIn, __m256i *BitsOut);
};

/*!
 * \brief A rate-0 decoder simply sets all bits to zero. (Who would have thought?)
 */
class RateZeroNode : public Node {
	Node *mParent;
public:
	RateZeroNode(Node* parent);///< Initialize the rate-0 decoder.
	~RateZeroNode();
	void decode(__m256i *LlrIn, __m256i *BitsOut);
};

/*!
 * \brief Perform hard-decoding of the given LLR-vector.
 */
class RateOneNode : public Node {
	Node *mParent;
public:
	RateOneNode(Node* parent);///< Initialize the rate-1 decoder.
	~RateOneNode();
	void decode(__m256i *LlrIn, __m256i *BitsOut);
};

/*!
 * \brief A specialized decoder for Polar Repetition Subcodes.
 */
class RepetitionNode : public Node {
	Node *mParent;
	Preparer *mPreparer;
public:
	RepetitionNode(Node* parent);///< Initialize the repetition decoder.
	~RepetitionNode();
	void decode(__m256i *LlrIn, __m256i *BitsOut);
};

/*!
 * \brief A specialized decoder for Polar Single Parity Check Subcodes.
 */
class SpcNode : public Node {
	Node *mParent;
	Preparer *mPreparer;
public:
	SpcNode(Node* parent);///< Initialize the SPC decoder.
	~SpcNode();
	void decode(__m256i *LlrIn, __m256i *BitsOut);
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
	/*!
	 * \brief Create a Fast-SSC decoder with AVX char-bit decoding.
	 * \param blockLength Length of the Polar Code.
	 * \param frozenBits Set of frozen bits in the code word.
	 */
	FastSscAvx2Char(size_t blockLength, const std::set<unsigned> &frozenBits);
	~FastSscAvx2Char();

	bool decode();
	void initialize(size_t blockLength, const std::set<unsigned> &frozenBits);
};

}//namespace Decoding
}//namespace PolarCode

#endif //PC_DEC_FASTSSC_AVX2_H
