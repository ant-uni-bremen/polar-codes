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


void   RateZeroDecode(__m256i *LlrIn, __m256i *BitsOut, const size_t blockLength);
void    RateOneDecode(__m256i *LlrIn, __m256i *BitsOut, const size_t blockLength);
void RepetitionDecode(__m256i *LlrIn, __m256i *BitsOut, const size_t blockLength);
void        SpcDecode(__m256i *LlrIn, __m256i *BitsOut, const size_t blockLength);



/*!
 * \brief A Rate-R node redirects decoding to polar subcodes of lower complexity.
 */
class RateRNode : public Node {
protected:
	Node *mParent;///< The parent node
	Node *mLeft,  ///< Left child node
		 *mRight; ///< Right child node
	Block<__m256i> *ChildLlr;///< Temporarily holds the LLRs child nodes have to decode.
	void (*leftDecoder)(__m256i*, __m256i*, size_t);///< Pointer to special decoding function of left child.
	void (*rightDecoder)(__m256i*, __m256i*, size_t);///< Pointer to special decoding function of right child.

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
 * \brief Create a specialized decoder for the given set of frozen bits.
 * \param frozenBits The set of frozen bits.
 * \param parent The parent node from which the code length is fetched.
 * \return Pointer to a polymorphic decoder object.
 */
Node* createDecoder(const std::vector<unsigned> &frozenBits, Node* parent, void (**specialDecoder)(__m256i*, __m256i*, size_t));

}// namespace FastSscAvx2

/*!
 * \brief The recursive systematic Fast-SSC decoder.
 */
class FastSscAvx2Char : public Decoder {
	FastSscAvx2::Node *mNodeBase,///< General code information
					  *mRootNode;///< Actual decoder
	DataPool<__m256i, 32> *mDataPool;///< Lazy-copy data-block pool
	void (*mSpecialDecoder)(__m256i*, __m256i*, size_t);

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
