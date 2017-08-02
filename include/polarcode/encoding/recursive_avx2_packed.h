#ifndef PC_ENC_RECURSIVE_AVX2_PACKED_H
#define PC_ENC_RECURSIVE_AVX2_PACKED_H

#include <polarcode/encoding/encoder.h>
#include <polarcode/encoding/butterfly_avx2_packed.h>
#include <polarcode/avxconvenience.h>
#include <polarcode/datapool.txx>

namespace PolarCode {
namespace Encoding {

namespace RecursiveAvx2 {

/*!
 * \brief Base class for a recursive encoding node.
 */
class Node {
	__m256i* mBit;

	void clearBlock();

protected:
	size_t mBlockLength,///< Length of the subcode.
		   mVecCount;  ///< Number of AVX-vectors the data can be stored in.

	void unsetBlockPointer();///< Prevent _mBit_ from being free()'d accidentally.

public:
	Node();
	/*!
	 * \brief Initialize a polar code's root node
	 * \param blockLength Length of the code.
	 */
	Node(size_t blockLength);
	virtual ~Node();

	virtual void encode(__m256i*);///< Execute a specialized encoding algorithm.

	/*!
	 * \brief Get the length of this code node.
	 * \return The length of this node.
	 */
	size_t blockLength();

	/*!
	 * \brief Get a pointer to bits of this node.
	 * \return Pointer to bit storage.
	 */
	__m256i* block();

};

/*!
 * \brief A recursice rate-1 encoder. Optimization yields a no-op.
 */
class RateOneNode : public Node {
	Node *mParent;
public:
	RateOneNode(Node* parent);///< Initialize the rate-1 encoder.
	~RateOneNode();
	void encode(__m256i *Bits);///< Do nothing.
};

/*!
 * \brief A recursice rate-0 encoder. Simply sets all bits to zero.
 */
class RateZeroNode : public Node {
	Node *mParent;
public:
	RateZeroNode(Node* parent);///< Initialize the rate-0 encoder.
	~RateZeroNode();
	void encode(__m256i *Bits);///< Set bits to zero.
};

/*!
 * \brief A repetition encoder. Sets all bits to the single info bit.
 */
class RepetitionNode : public Node {
	Node *mParent;
public:
	RepetitionNode(Node* parent);///< Initialize the repetition encoder.
	~RepetitionNode();
	void encode(__m256i *Bits);///< Repeat the information bit.
};

/*!
 * \brief The SPC encoder sets the single parity bit.
 */
class SpcNode : public Node {
	Node *mParent;
public:
	SpcNode(Node* parent);///< Initialize the SPC encoder.
	~SpcNode();
	void encode(__m256i *Bits);///< Add even-parity bit.
};

/*!
 * \brief For codes shorter than vector-length, falling back to butterfly encoder is easier.
 */
class ShortButterflyNode : public Node {
	Node *mParent;
	Encoder *mButterflyEncoder;
public:
	ShortButterflyNode(std::set<unsigned> &frozenBits, Node* parent);///< Initialize the butterfly encoder.
	~ShortButterflyNode();
	void encode(__m256i *Bits);///< Perform butterfly encoding.
};

/*!
 * \brief The RateRNode splits a complex code into simpler codes of half length.
 */
class RateRNode : public Node {
	Node *mParent;
	Node *mLeft, *mRight;
	int mStage;
public:
	RateRNode(std::set<unsigned> &frozenBits, Node* parent);///< Initialize the general recursive encoder.
	~RateRNode();
	void encode(__m256i *Bits);///< Recursively invoke simpler decoders.
};

Node* createEncoder(std::set<unsigned> &frozenBits, Node* parent);

size_t nBit2vecCount(size_t blockLength);


}// namespace RecursiveAvx2

/*!
 * \brief Recursive Polar Encoding using AVX2.
 *
 * This encoder performs recursive polar encoding on 256-bit operators.
 *
 */
class RecursiveAvx2Packed : public Encoder {
	RecursiveAvx2::Node *mNodeBase,
						*mRootNode;
	void clear();

public:
	RecursiveAvx2Packed();

	/*!
	 * \brief Create the recursive encoder and initialize its parameters.
	 * \param blockLength Number of code bits.
	 * \param frozenBits Set of frozen channel indices.
	 */
	RecursiveAvx2Packed(size_t blockLength,
					  const std::set<unsigned> &frozenBits);

	~RecursiveAvx2Packed();

	void encode();
	void initialize(size_t blockLength,
					const std::set<unsigned> &frozenBits);
};


}//namespace Encoding
}//namespace PolarCode

#endif // PC_ENC_RECURSIVE_AVX2_PACKED_H


