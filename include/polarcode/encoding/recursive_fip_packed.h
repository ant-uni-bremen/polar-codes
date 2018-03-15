#ifndef PC_ENC_RECURSIVE_FIP_PACKED_H
#define PC_ENC_RECURSIVE_FIP_PACKED_H

#include <polarcode/encoding/encoder.h>
#include <polarcode/encoding/butterfly_fip_packed.h>
#include <polarcode/avxconvenience.h>
#include <polarcode/datapool.txx>

namespace PolarCode {
namespace Encoding {

namespace RecursiveFip {

/*!
 * \brief Base class for a recursive encoding node.
 */
class Node {
	fipv* mBit;

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

	virtual void encode(fipv*);///< Execute a specialized encoding algorithm.

	/*!
	 * \brief Get the length of this code node.
	 * \return The length of this node.
	 */
	size_t blockLength();

	/*!
	 * \brief Get a pointer to bits of this node.
	 * \return Pointer to bit storage.
	 */
	fipv* block();

};

/*!
 * \brief A recursice rate-1 encoder. Optimization yields a no-op.
 */
class RateOneNode : public Node {
	Node *mParent;
public:
	RateOneNode(Node* parent);///< Initialize the rate-1 encoder.
	~RateOneNode();
	void encode(fipv *Bits);///< Do nothing.
};

/*!
 * \brief A recursice rate-0 encoder. Simply sets all bits to zero.
 */
class RateZeroNode : public Node {
	Node *mParent;
public:
	RateZeroNode(Node* parent);///< Initialize the rate-0 encoder.
	~RateZeroNode();
	void encode(fipv *Bits);///< Set bits to zero.
};

/*!
 * \brief A repetition encoder. Sets all bits to the single info bit.
 */
class RepetitionNode : public Node {
	Node *mParent;
public:
	RepetitionNode(Node* parent);///< Initialize the repetition encoder.
	~RepetitionNode();
	void encode(fipv *Bits);///< Repeat the information bit.
};

/*!
 * \brief The SPC encoder sets the single parity bit.
 */
class SpcNode : public Node {
	Node *mParent;
public:
	SpcNode(Node* parent);///< Initialize the SPC encoder.
	~SpcNode();
	void encode(fipv *Bits);///< Add even-parity bit.
};

/*!
 * \brief For codes shorter than vector-length, falling back to butterfly encoder is easier.
 */
class ShortButterflyNode : public Node {
	Node *mParent;
	Encoder *mButterflyEncoder;
public:
	ShortButterflyNode(std::vector<unsigned> &frozenBits, Node* parent);///< Initialize the butterfly encoder.
	~ShortButterflyNode();
	void encode(fipv *Bits);///< Perform butterfly encoding.
};

/*!
 * \brief The RateRNode splits a complex code into simpler codes of half length.
 */
class RateRNode : public Node {
	Node *mParent;
	Node *mLeft, *mRight;
	int mStage;
public:
	RateRNode(std::vector<unsigned> &frozenBits, Node* parent);///< Initialize the general recursive encoder.
	~RateRNode();
	void encode(fipv *Bits);///< Recursively invoke simpler decoders.
};

Node* createEncoder(std::vector<unsigned> &frozenBits, Node* parent);

size_t nBit2vecCount(size_t blockLength);


}// namespace RecursiveAvx2

/*!
 * \brief Recursive Polar Encoding using AVX2.
 *
 * This encoder performs recursive polar encoding on 256-bit operators.
 *
 */
class RecursiveFipPacked : public Encoder {
	RecursiveFip::Node *mNodeBase,
						*mRootNode;
	void clear();

public:
	RecursiveFipPacked();

	/*!
	 * \brief Create the recursive encoder and initialize its parameters.
	 * \param blockLength Number of code bits.
	 * \param frozenBits Set of frozen channel indices.
	 */
	RecursiveFipPacked(size_t blockLength,
					  const std::vector<unsigned> &frozenBits);

	~RecursiveFipPacked();

	void encode();
	void initialize(size_t blockLength,
					const std::vector<unsigned> &frozenBits);
};


}//namespace Encoding
}//namespace PolarCode

#endif // PC_ENC_RECURSIVE_FIP_PACKED_H


