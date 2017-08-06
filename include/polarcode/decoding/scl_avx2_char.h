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

struct PathItem {
	std::vector<block_t*> Llr;
	std::vector<block_t*> Bits;
	long Metric;
};

typedef std::multimap<long, PathItem> PathList;

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
	void clearPathList();
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
