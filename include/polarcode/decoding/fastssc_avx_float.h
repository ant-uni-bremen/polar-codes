#ifndef PC_DEC_FASTSSC_AVX_FLOAT_H
#define PC_DEC_FASTSSC_AVX_FLOAT_H

#include <polarcode/decoding/decoder.h>
#include <polarcode/datapool.txx>
#include <polarcode/decoding/avx_float.h>

namespace PolarCode {
namespace Decoding {

namespace FastSscAvx {

/*!
 * \brief A node of the polar decoding tree.
 */
class Node {
	typedef DataPool<float, 32> datapool_t;
	Block<float> *mLlr, *mBit;

	void clearBlocks();
	void clearLlrBlock();
	void clearBitBlock();

protected:
	//xm = eXternal member (not owned by this Node)
	datapool_t *xmDataPool;///< Pointer to a DataPool object.
	size_t mBlockLength;   ///< Length of the subcode.

public:
	Node();
	/*!
	 * \brief Initialize a polar code's root node
	 * \param blockLength Length of the code.
	 * \param pool Pointer to a DataPool, which provides lazy-copyable memory blocks.
	 */
	Node(size_t blockLength, datapool_t *pool);
	virtual ~Node();

	virtual void decode(float *LlrIn, float *BitsOut);///< Execute a specialized decoding algorithm.

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
	float* input();

	/*!
	 * \brief Get a pointer to bits of this node.
	 * \return Pointer to bit storage.
	 */
	float* output();

};


void   RateZeroDecode(float *LlrIn, float *BitsOut, const size_t blockLength);
void    RateOneDecode(float *LlrIn, float *BitsOut, const size_t blockLength);
void RepetitionDecode(float *LlrIn, float *BitsOut, const size_t blockLength);
void        SpcDecode(float *LlrIn, float *BitsOut, const size_t blockLength);
void    ZeroSpcDecode(float *LlrIn, float *BitsOut, const size_t blockLength);

void simplifiedRightRateOneDecode(float *LlrIn, float *BitsOut, const size_t blockLength);

enum ChildCreationFlags {
	BOTH,
	NO_LEFT = 0x01,
	NO_RIGHT = 0x02
};

/*!
 * \brief A Rate-R node redirects decoding to polar subcodes of lower complexity.
 */
class RateRNode : public Node {
protected:
	Node *mParent;///< The parent node
	Node *mLeft,  ///< Left child node
		 *mRight; ///< Right child node
	Block<float> *ChildLlr;///< Temporarily holds the LLRs child nodes have to decode.
	void (*leftDecoder)(float*, float*, size_t);///< Pointer to special decoding function of left child.
	void (*rightDecoder)(float*, float*, size_t);///< Pointer to special decoding function of right child.

public:
	/*!
	 * \brief Using the set of frozen bits, specialized subcodes are selected.
	 * \param frozenBits The set of frozen bits of this code.
	 * \param parent The parent node, defining the length of this code.
	 * \param flags Set to [NO_LEFT | NO_RIGHT] to disable child creation.
	 */
	RateRNode(const std::vector<unsigned> &frozenBits, Node *parent, ChildCreationFlags flags = BOTH);
	~RateRNode();
	void decode(float *LlrIn, float *BitsOut);
};

/*!
 * \brief A rate-R node of subvector length needs child bits in separate blocks.
 */
class ShortRateRNode : public RateRNode {
	Block<float> *LeftBits, *RightBits;

public:
	/*!
	 * \brief Using the set of frozen bits, specialized subcodes are selected.
	 * \param frozenBits The set of frozen bits of this code.
	 * \param parent The parent node, defining the length of this code.
	 */
	ShortRateRNode(const std::vector<unsigned> &frozenBits, Node *parent);
	~ShortRateRNode();
	void decode(float *LlrIn, float *BitsOut);
};

/*!
 * \brief Optimized decoding, if the right subcode is rate-1.
 */
class ROneNode : public RateRNode {
public:
	/*!
	 * \brief Initialize the right-rate-1 optimized decoder.
	 * \param frozenBits The set of frozen bits of both subcodes (apparently only left subcode has frozen bits).
	 * \param parent The parent node.
	 */
	ROneNode(const std::vector<unsigned> &frozenBits, Node *parent);
	~ROneNode();
	void decode(float *LlrIn, float *BitsOut);
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
	void decode(float *LlrIn, float *BitsOut);
};

/*!
 * \brief Create a specialized decoder for the given set of frozen bits.
 * \param frozenBits The set of frozen bits.
 * \param parent The parent node from which the code length is fetched.
 * \return Pointer to a polymorphic decoder object.
 */
Node* createDecoder(const std::vector<unsigned> &frozenBits, Node* parent, void (**specialDecoder)(float*, float*, size_t));

}// namespace FastSscAvx

/*!
 * \brief The recursive systematic Fast-SSC decoder.
 */
class FastSscAvxFloat : public Decoder {
	FastSscAvx::Node *mNodeBase,///< General code information
					  *mRootNode;///< Actual decoder
	DataPool<float, 32> *mDataPool;///< Lazy-copy data-block pool
	void (*mSpecialDecoder)(float*, float*, size_t);

	void clear();

public:
	/*!
	 * \brief Create a Fast-SSC decoder with AVX float-bit decoding.
	 * \param blockLength Length of the Polar Code.
	 * \param frozenBits Set of frozen bits in the code word.
	 */
	FastSscAvxFloat(size_t blockLength, const std::vector<unsigned> &frozenBits);
	~FastSscAvxFloat();

	bool decode();
	void initialize(size_t blockLength, const std::vector<unsigned> &frozenBits);
};

}//namespace Decoding
}//namespace PolarCode

#endif //PC_DEC_FASTSSC_AVX_FLOAT_H
