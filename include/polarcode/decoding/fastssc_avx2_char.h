#ifndef PC_DEC_FASTSSC_AVX2_H
#define PC_DEC_FASTSSC_AVX2_H

#include <polarcode/decoding/decoder.h>
#include <polarcode/datapool.txx>
#include <polarcode/decoding/avx2_char.h>

namespace PolarCode {
namespace Decoding {

namespace FastSscAvx2 {

/*!
 * \brief A node of the polar decoding tree.
 */
class Node {
	typedef DataPool<__m256i, 32> datapool_t;
	Block<__m256i> *mLlr, *mBit;

protected:
	//xm = eXternal member (not owned by this Node)
	datapool_t *xmDataPool;///< Pointer to a DataPool object.
	size_t mBlockLength,   ///< Length of the subcode.
		   mVecCount;      ///< Number of AVX-vectors the data can be stored in.

public:
	Node();
	Node(Node *parent);
	/*!
	 * \brief Initialize a polar code's root node
	 * \param blockLength Length of the code.
	 * \param pool Pointer to a DataPool, which provides lazy-copyable memory blocks.
	 * \param softOutput Whether XOR or Boxplus will be used for bit combination.
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

class ShortNode : public Node {
protected:
	Block<__m256i> *mLeftBits, *mRightBits;

public:
	ShortNode(Node *parent);
	virtual ~ShortNode();
	virtual void decode(__m256i *LlrIn, __m256i *BitsOut) = 0;
};


/*!
 * \brief A Rate-R node redirects decoding to polar subcodes of lower complexity.
 */
class RateRNode : public Node {
protected:
	Node *mLeft,  ///< Left child node
		 *mRight; ///< Right child node
	Block<__m256i> *ChildLlr;///< Temporarily holds the LLRs child nodes have to decode.

public:
	/*!
	 * \brief Using the set of frozen bits, specialized subcodes are selected.
	 * \param frozenBits The set of frozen bits of this code.
	 * \param parent The parent node, defining the length of this code.
	 */
	RateRNode(const std::vector<unsigned> &frozenBits, Node *parent);
	~RateRNode();
	void decode(__m256i *LlrIn, __m256i *BitsOut);
};

/*!
 * \brief A rate-R node of subvector length needs child bits in separate blocks.
 */
class ShortRateRNode : public RateRNode {
	void simplifiedRightRateOneDecodeShort(__m256i *LlrIn, __m256i *BitsOut);

protected:
	Block<__m256i> *LeftBits, *RightBits;

public:
	/*!
	 * \brief Using the set of frozen bits, specialized subcodes are selected.
	 * \param frozenBits The set of frozen bits of this code.
	 * \param parent The parent node, defining the length of this code.
	 */
	ShortRateRNode(const std::vector<unsigned> &frozenBits, Node *parent);
	~ShortRateRNode();
	void decode(__m256i *LlrIn, __m256i *BitsOut);
};

/*!
 * \brief Optimized decoding, if the right subcode is rate-1.
 */
class ROneNode : public RateRNode {
	void simplifiedRightRateOneDecode(__m256i *LlrIn, __m256i *BitsOut);

public:
	/*!
	 * \brief Initialize the right-rate-1 optimized decoder.
	 * \param frozenBits The set of frozen bits of both subcodes (apparently only left subcode has frozen bits).
	 * \param parent The parent node.
	 */
	ROneNode(const std::vector<unsigned> &frozenBits, Node *parent);
	~ROneNode();
	void decode(__m256i *LlrIn, __m256i *BitsOut);
};

/*!
 * \brief Optimized decoding, if the right subcode is rate-1.
 */
class ShortROneNode : public ShortRateRNode {
	void simplifiedRightRateOneDecodeShort(__m256i *LlrIn, __m256i *BitsOut);
public:
	/*!
	 * \brief Initialize the right-rate-1 optimized decoder.
	 * \param frozenBits The set of frozen bits of both subcodes (apparently only left subcode has frozen bits).
	 * \param parent The parent node.
	 */
	ShortROneNode(const std::vector<unsigned> &frozenBits, Node *parent);
	~ShortROneNode();
	void decode(__m256i *LlrIn, __m256i *BitsOut);
};

/*!
 * \brief Optimized decoding, if the left subcode is rate-0.
 */
class ZeroRNode : public RateRNode {
public:
	/*!
	 * \brief Initialize the left-rate-0 optimized decoder.
	 * \param frozenBits The set of frozen bits for both subcodes.
	 * \param parent The parent node.
	 */
	ZeroRNode(const std::vector<unsigned> &frozenBits, Node *parent);
	~ZeroRNode();
	void decode(__m256i *LlrIn, __m256i *BitsOut);
};

/*!
 * \brief Optimized decoding, if the left subcode is rate-0.
 */
class ShortZeroRNode : public ShortRateRNode {
public:
	/*!
	 * \brief Initialize the left-rate-0 optimized decoder.
	 * \param frozenBits The set of frozen bits for both subcodes.
	 * \param parent The parent node.
	 */
	ShortZeroRNode(const std::vector<unsigned> &frozenBits, Node *parent);
	~ShortZeroRNode();
	void decode(__m256i *LlrIn, __m256i *BitsOut);
};

class RateZeroDecoder : public Node {
public:
	RateZeroDecoder(Node *parent);
	~RateZeroDecoder();
	void decode(__m256i*, __m256i *BitsOut);
};

class RateOneDecoder : public Node {
public:
	RateOneDecoder(Node *parent);
	~RateOneDecoder();
	void decode(__m256i *LlrIn, __m256i *BitsOut);
};

class RepetitionDecoder : public Node {
public:
	RepetitionDecoder(Node *parent);
	~RepetitionDecoder();
	void decode(__m256i *LlrIn, __m256i *BitsOut);
};

class ShortRepetitionDecoder : public ShortNode {
public:
	ShortRepetitionDecoder(Node *parent);
	~ShortRepetitionDecoder();
	void decode(__m256i *LlrIn, __m256i *BitsOut);
};

class SpcDecoder : public Node {
public:
	SpcDecoder(Node *parent);
	~SpcDecoder();
	void decode(__m256i *LlrIn, __m256i *BitsOut);
};

class ShortSpcDecoder : public ShortNode {
public:
	ShortSpcDecoder(Node *parent);
	~ShortSpcDecoder();
	void decode(__m256i *LlrIn, __m256i *BitsOut);
};

class ZeroSpcDecoder : public Node {
	unsigned mSubBlockLength;
	unsigned mSubVecCount;

public:
	ZeroSpcDecoder(Node *parent);
	~ZeroSpcDecoder();
	void decode(__m256i *LlrIn, __m256i *BitsOut);
};

class ShortZeroSpcDecoder : public ShortNode {
	unsigned mSubBlockLength;

public:
	ShortZeroSpcDecoder(Node *parent);
	~ShortZeroSpcDecoder();
	void decode(__m256i *LlrIn, __m256i *BitsOut);
};

class ShortZeroOneDecoder : public ShortNode {
	unsigned mSubBlockLength;

public:
	ShortZeroOneDecoder(Node *parent);
	~ShortZeroOneDecoder();
	void decode(__m256i *LlrIn, __m256i *BitsOut);
};

/*!
 * \brief Create a specialized decoder for the given set of frozen bits.
 * \param frozenBits The set of frozen bits.
 * \param parent The parent node from which the code length is fetched.
 * \return Pointer to a polymorphic decoder object.
 */
Node* createDecoder(const std::vector<unsigned> &frozenBits, Node* parent);

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
	FastSscAvx2Char(size_t blockLength, const std::vector<unsigned> &frozenBits);
	~FastSscAvx2Char();

	bool decode();
	void initialize(size_t blockLength, const std::vector<unsigned> &frozenBits);
};

}//namespace Decoding
}//namespace PolarCode

#endif //PC_DEC_FASTSSC_AVX2_H
