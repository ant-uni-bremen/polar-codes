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
	std::vector<std::vector<block_t*>> mLeftBitTree;
	std::vector<long> mMetric;
	std::vector<std::vector<block_t*>> mNextLlrTree;
	std::vector<std::vector<block_t*>> mNextBitTree;
	std::vector<std::vector<block_t*>> mNextLeftBitTree;
	std::vector<long> mNextMetric;
	unsigned mPathLimit, mPathCount, mNextPathCount;
	unsigned mStageCount;
	datapool_t *xmDataPool;
public:
	block_t* tempBlock;///< Pointer to a permanently allocated block for temporary storage.

	PathList();

	/*!
	 * \brief Create a PathList object to manage list decoding.
	 * \param listSize Maximum number of paths.
	 * \param stageCount Depth of recursion.
	 * \param dataPool Data pool which provides an easy lazy-copy container.
	 */
	PathList(size_t listSize, size_t stageCount, datapool_t* dataPool);
	~PathList();

	/*!
	 * \brief Release all allocated data blocks.
	 */
	void clear();

	/*!
	 * \brief Copy an existing decoding path.
	 *
	 * At each constituent decoding node, a new group of paths is created based
	 * on the existing group. An existing path might either be abandoned or
	 * copied once or multiple times. The exact number is not known prior to
	 * cleaning up the old group. If a given path has only one decendent, the
	 * copy operation results in a simple reassociation instead of doing a
	 * performance-relevant memory copy, due to lazy-copying.
	 *
	 * \param destination Path number for the new path.
	 * \param source Path number of the old path.
	 * \param stage Parameter to omit copying of non-existing values.
	 */
	void duplicatePath(unsigned destination, unsigned source, unsigned stage);

	/*!
	 * \brief Get write acces to an LLR-block. May invoke a copy operation.
	 *
	 * Before writing to a lazy-copied block, this function assures that the
	 * memory location is not referenced by other blocks. If this block was
	 * referenced by other blocks, the data will be copied into a new block for
	 * non-destructive write access.
	 *
	 * \param path Index of the path to be altered.
	 * \param stage Index of the stage to be altered.
	 */
	void getWriteAccessToLlr(unsigned path, unsigned stage);

	/*!
	 * \brief Get write acces to a bit-block. May invoke a copy operation.
	 *
	 * See getWriteAccessToLlr() for details.
	 *
	 * \param path Index of the path to be altered.
	 * \param stage Index of the stage to be altered.
	 */
	void getWriteAccessToBit(unsigned path, unsigned stage);

	/*!
	 * \brief Get write acces to a future bit-block.
	 *        May invoke a copy operation.
	 *
	 * See getWriteAccessToLlr() for details.
	 *
	 * \param path Index of the path to be altered.
	 * \param stage Index of the stage to be altered.
	 */
	void getWriteAccessToNextBit(unsigned path, unsigned stage);

	/*!
	 * \brief Mark an old path as unused by decreasing the block's reference
	 *        counter.
	 *
	 * This function cleans up old paths. If a path got abandoned, the data
	 * blocks associated to it are returned to the data pool.
	 *
	 * \param path Index of the path to clear.
	 * \param stage Index of the minimum stage.
	 */
	void clearOldPath(unsigned path, unsigned stage);

	/*!
	 * \brief Set the future path list to be the currently active list.
	 */
	void switchToNext();

	/*!
	 * \brief Initialize the path list by copying data into the first single
	 *        path of it.
	 *
	 * \param pLlr Pointer to LLRs.
	 * \param vecCount Number of AVX2-vectors to copy (32-element chunks).
	 */
	void setFirstPath(void* pLlr);

	/*!
	 * \brief Allocate LLR- and bit-blocks for the given stage.
	 *
	 * \param stage
	 */
	void allocateStage(unsigned stage);

	/*!
	 * \brief Return LLR- and bit-blocks to the data pool for the given stage.
	 *
	 * \param stage
	 */
	void clearStage(unsigned stage);


	/*!
	 * \brief Get a pointer to an LLR-block.
	 *
	 * \param path Index of the path.
	 * \param stage Index of the stage.
	 * \return A pointer to an AVX2 aligned memory block.
	 */
	__m256i* Llr(unsigned path, unsigned stage);

	/*!
	 * \brief Get a pointer to a bit-block.
	 *
	 * \param path Index of the path.
	 * \param stage Index of the stage.
	 * \return A pointer to an AVX2 aligned memory block.
	 */
	__m256i* Bit(unsigned path, unsigned stage);

	/*!
	 * \brief Get a pointer to a left bit-block.
	 *
	 * \param path Index of the path.
	 * \param stage Index of the stage.
	 * \return A pointer to an AVX2 aligned memory block.
	 */
	__m256i* LeftBit(unsigned path, unsigned stage);

	/*!
	 * \brief Swap bit blocks. This eliminates a copy operation.
	 * \param stage The stage to be swapped.
	 */
	void swapBitBlocks(unsigned stage);

	/*!
	 * \brief Get a pointer to a future LLR-block.
	 *
	 * \param path Index of the path.
	 * \param stage Index of the stage.
	 * \return A pointer to an AVX2 aligned memory block.
	 */
	__m256i* NextLlr(unsigned path, unsigned stage);

	/*!
	 * \brief Get a pointer to a future bit-block.
	 *
	 * \param path Index of the path.
	 * \param stage Index of the stage.
	 * \return A pointer to an AVX2 aligned memory block.
	 */
	__m256i* NextBit(unsigned path, unsigned stage);

	/*!
	 * \brief Get a reference to the path metric variable.
	 * \param path The requested path.
	 * \return Reference to path metric.
	 */
	long& Metric(unsigned path);

	/*!
	 * \brief Get a reference to a future path metric variable.
	 * \param path The requested path.
	 * \return Reference to path metric.
	 */
	long& NextMetric(unsigned path);

	/*!
	 * \brief Get the number of currently active paths.
	 */
	unsigned PathCount();

	/*!
	 * \brief Get the number of maximum allowed paths.
	 */
	unsigned PathLimit();

	/*!
	 * \brief Set the new number of active paths.
	 */
	void setNextPathCount(unsigned);
};

/*!
 * \brief A node of the decoding tree.
 */
class Node {
protected:
	//xm = eXternal member (not owned by this Node)
	datapool_t *xmDataPool; ///< Pointer to a DataPool object.
	unsigned mBlockLength,  ///< Length of the subcode.
			 mVecCount,     ///< Number of AVX-vectors the data can be stored in.
			 mStage,        ///< Recursion depth of this node
			 mListSize;     ///< Limit for number of concurrently active paths.
	PathList *xmPathList;   ///< Pointer to PathList object.

public:
	Node();

	/*!
	 * \brief Create a node and copy parameters of another.
	 * \param other The reference node.
	 */
	Node(Node *other);

	/*!
	 * \brief Initialize a node by given parameters.
	 * \param blockLength Length of the subcode.
	 * \param listSize Limit for number of concurrently active paths.
	 * \param pool Pointer to a DataPool.
	 * \param pathList Pointer to the PathList to use.
	 */
	Node(size_t blockLength, size_t listSize, datapool_t *pool, PathList *pathList);

	virtual ~Node();

	/*!
	 * \brief Invoke the decoding function of this node.
	 */
	virtual void decode();

	/*!
	 * \brief Get a pointer to the DataPool.
	 * \return A pointer to the DataPool.
	 */
	datapool_t* pool();

	/*!
	 * \brief Get the block length of this node.
	 * \return Block length of this node.
	 */
	unsigned blockLength();

	/*!
	 * \brief Get the maximum number of active paths.
	 * \return Maximum number of active paths.
	 */
	unsigned listSize();

	/*!
	 * \brief Get a pointer to the PathList object.
	 * \return A pointer to the PathList object.
	 */
	SclAvx2::PathList* pathList();
};

/*!
 * \brief The DecoderNode manages code splitting and combination.
 */
class RateRNode : public Node {
protected:
	Node *mLeft,    ///< Left child node
		 *mRight;   ///< Right child node

	/*! Pointer to Boxplus combination function */
	void (*combineFunction)(__m256i*, __m256i*, __m256i*, const unsigned);

public:
	/*!
	 * \brief Create a decoder node.
	 * \param frozenBits The set of frozen bits for this code.
	 * \param parent The parent node to copy all information from.
	 */
	RateRNode(const std::vector<unsigned> &frozenBits, Node *parent);
	~RateRNode();
	void decode();
};

class RateZeroDecoder : public Node {
public:
	RateZeroDecoder(Node *parent);
	~RateZeroDecoder();
	void decode();
};

class RateOneDecoder : public Node {
	std::vector<unsigned> mIndices;
	std::vector<long> mMetrics;
	std::vector<std::array<unsigned, 2>> mBitFlipHints;
	std::vector<unsigned> mBitFlipCount;

public:
	RateOneDecoder(Node *parent);
	~RateOneDecoder();
	void decode();
};

class RepetitionDecoder : public Node {
	std::vector<unsigned> mIndices;
	std::vector<long> mMetrics;
	std::vector<char> mResults;

public:
	RepetitionDecoder(Node *parent);
	~RepetitionDecoder();
	void decode();
};

class SpcDecoder : public Node {
	std::vector<unsigned> mIndices;
	std::vector<long> mMetrics;
	std::vector<std::array<unsigned, 4>> mBitFlipHints;
	std::vector<unsigned> mBitFlipCount;
public:
	SpcDecoder(Node *parent);
	~SpcDecoder();
	void decode();
};


Node* createDecoder(const std::vector<unsigned> &frozenBits, Node* parent);

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
