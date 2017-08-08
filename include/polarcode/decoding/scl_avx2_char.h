#ifndef PC_DEC_SCL_AVX2_H
#define PC_DEC_SCL_AVX2_H

#include <polarcode/decoding/decoder.h>
#include <polarcode/decoding/avx2_char.h>
#include <polarcode/datapool.txx>
#include <vector>
#include <map>

namespace PolarCode {
namespace Decoding {

namespace SclAvx2 {

typedef DataPool<__m256i, 32> datapool_t;
typedef Block<__m256i> block_t;

/*!
 * \brief This class manages the collection of decoding paths.
 *
 * In the former list decoder implementation every decoder function
 * reimplemented list access. Now it is centralized in an object of
 * PathList class.
 */
class PathList {
	std::vector<std::vector<block_t*>> mLlrTree;
	std::vector<std::vector<block_t*>> mBitTree;
	std::vector<long> mMetric;
	std::vector<std::vector<block_t*>> mNextLlrTree;
	std::vector<std::vector<block_t*>> mNextBitTree;
	std::vector<long> mNextMetric;
	unsigned mPathLimit, mPathCount, mNextPathCount;
	unsigned mStageCount;
	datapool_t *xmDataPool;
public:

	PathList();

	/*!
	 * \brief Create a PathList object to manage list decoding.
	 * \param listSize Maximum number of paths.
	 * \param stageCount Depth of recursion.
	 * \param dataPool Data pool which provides an easy lazy-copy container.
	 */
	PathList(size_t listSize, size_t stageCount, datapool_t* dataPool);
	~PathList();
	void clear();
	void duplicatePath(unsigned destination, unsigned source, unsigned stage);
	void getWriteAccessToLlr(unsigned path, unsigned stage);
	void getWriteAccessToBit(unsigned path, unsigned stage);
	void getWriteAccessToNextBit(unsigned path, unsigned stage);
	void clearOldPath(unsigned path, unsigned stage);
	void switchToNext();
	void setFirstPath(void* pLlr, unsigned vecCount);
	void allocateStage(unsigned stage, unsigned vecCount);
	void clearStage(unsigned stage);

	__m256i* Llr(unsigned path, unsigned stage);
	__m256i* Bit(unsigned path, unsigned stage);
	__m256i* NextLlr(unsigned path, unsigned stage);
	__m256i* NextBit(unsigned path, unsigned stage);
	long& Metric(unsigned path);
	long& NextMetric(unsigned path);
	unsigned PathCount();
	unsigned PathLimit();
	void setNextPathCount(unsigned);
};

/*!
 * \brief A node of the decoding tree.
 */
class Node {
protected:
	datapool_t *xmDataPool;
	size_t mBlockLength,
		   mVecCount,
		   mListSize;
	PathList *xmPathList;

public:
	Node();
	Node(Node *other);
	Node(size_t blockLength, size_t listSize, datapool_t *pool, PathList *pathList);
	virtual ~Node();
	virtual void decode();
	datapool_t* pool();
	size_t blockLength();
	size_t listSize();
	SclAvx2::PathList* pathList();
};

/*!
 * \brief The DecoderNode manages code splitting and combination.
 */
class DecoderNode : public Node {
protected:
	Node *mParent;
	Node *mLeft,
		 *mRight;
	unsigned mStage;

	std::vector<block_t*> childBits;

	void (*leftDecoder)(PathList*, unsigned);
	void (*rightDecoder)(PathList*, unsigned);

public:
	DecoderNode();

	/*!
	 * \brief Create a decoder node.
	 * \param frozenBits The set of frozen bits for this code.
	 * \param parent The parent node to copy all information from.
	 */
	DecoderNode(std::vector<unsigned> &frozenBits, Node *parent);
	~DecoderNode();
	void decode();
};

Node* createDecoder(std::vector<unsigned> frozenBits, Node* parent, void (**specialDecoder)(PathList*, unsigned));

void RateZeroDecode(PathList* pathList, unsigned stage);
void RateOneDecode(PathList* pathList, unsigned stage);


void RateZeroDecodeSingleBit(PathList* pathList, unsigned);
void RateOneDecodeSingleBit(PathList* pathList, unsigned);

}// namespace SclAvx2


/*!
 * \brief This class implements the list decoder interface.
 */
class SclAvx2Char : public Decoder {
	size_t mListSize;
	SclAvx2::Node *mNodeBase,
				  *mRootNode;
	SclAvx2::datapool_t *mDataPool;
	SclAvx2::PathList *mPathList;
	void (*mSpecialDecoder)(SclAvx2::PathList*, unsigned);

	void clear();
	void makeInitialPathList();
	bool extractBestPath();

public:
	/*!
	 * \brief Create a list decoder.
	 * \param blockLength Number of bits sent over a channel.
	 * \param listSize Number of paths to examine while decoding.
	 * \param frozenBits The set of frozen bits.
	 */
	SclAvx2Char(size_t blockLength, size_t listSize, const std::vector<unsigned> &frozenBits);
	~SclAvx2Char();

	bool decode();
	void initialize(size_t blockLength, const std::vector<unsigned> &frozenBits);

	/*!
	 * \brief Set the path limit parameter.
	 * \param newListSize The new maximum path count for decoding.
	 */
	void setListSize(size_t newListSize);
};





}// namespace Decoding
}// namespace PolarCode

#endif //PC_DEC_SCL_AVX2_H
