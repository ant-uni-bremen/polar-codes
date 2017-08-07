#ifndef PC_DEC_SCL_AVX2_H
#define PC_DEC_SCL_AVX2_H

#include <polarcode/decoding/decoder.h>
#include <polarcode/datapool.txx>
#include <vector>
#include <map>

namespace PolarCode {
namespace Decoding {

namespace SclAvx2 {

typedef DataPool<__m256i, 32> datapool_t;
typedef Block<__m256i> block_t;

class PathList {
	std::vector<std::vector<block_t*>> mLlrTree;
	std::vector<std::vector<block_t*>> mBitTree;
	std::vector<long> mMetric;
	std::vector<std::vector<block_t*>> mNextLlrTree;
	std::vector<std::vector<block_t*>> mNextBitTree;
	std::vector<long> mNextMetric;
	size_t mPathLimit, mPathCount, mNextPathCount;
	size_t mStageCount;
	datapool_t *xmDataPool;
public:

	PathList();
	PathList(size_t listSize, size_t stageCount, datapool_t* dataPool);
	~PathList();
	void clear();
	void duplicatePath(unsigned destination, unsigned source, unsigned stage);
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
};

class Node {
protected:
	datapool_t *xmDataPool;
	size_t mBlockLength,
		   mVecCount,
		   mListSize;
	PathList *xmPathList;

public:
	Node();
	Node(size_t blockLength, size_t listSize, datapool_t *pool, PathList *pathlist);
	virtual ~Node();
	virtual void decode();
};

/*!
 * \brief Convert block length to minimum AVX-vector count.
 * \param blockLength Bits to store
 * \return The number of AVX-vectors required to store _blockLength_ char bits.
 */
size_t nBit2vecCount(size_t blockLength);


}// namespace SclAvx2


class SclAvx2Char : public Decoder {
	size_t mListSize;
	SclAvx2::Node *mNodeBase,
				  *mRootNode;
	SclAvx2::datapool_t *mDataPool;
	SclAvx2::PathList *mPathList;

	void clear();
	void makeInitialPathList();
	bool extractBestPath();

public:
	SclAvx2Char(size_t blockLength, size_t listSize, const std::vector<unsigned> &frozenBits);
	~SclAvx2Char();

	bool decode();
	void initialize(size_t blockLength, const std::vector<unsigned> &frozenBits);
	void setListSize(size_t newListSize);
};





}// namespace Decoding
}// namespace PolarCode

#endif //PC_DEC_SCL_AVX2_H
