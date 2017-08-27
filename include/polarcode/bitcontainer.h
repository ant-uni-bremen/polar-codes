#ifndef PC_BITCONTAINER_H
#define PC_BITCONTAINER_H

#include <cstddef>
#include <vector>

namespace PolarCode {

/*!
 * \brief The BitContainer skeleton-class
 *
 * As this project uses different bit formats in the coding algorithms,
 * a converting bit container is needed.
 */
class BitContainer {
	void clear();
	void calculateLUT();

protected:
	size_t mElementCount;///< The fixed number of bits stored in this container.
	std::vector<unsigned> mFrozenBits;///< The set of frozen bits.
	unsigned mInformationBitCount; ///< Parameter K. The amount of information bits per code.
	unsigned *mLUT; ///< A lookup table for fast information bit insertion.

public:
	BitContainer();
	BitContainer(size_t size);///< Create a BitContainer with given bit count.
	/*!
	 * \brief Create a fully configured bit container.
	 * \param size Length of the code word to be stored.
	 * \param frozenBits Set of frozen bits.
	 */
	BitContainer(size_t size, const std::vector<unsigned> &frozenBits);

	virtual ~BitContainer();

	/*!
	 * \brief Sets the number of bits to be stored in this container.
	 * \param newSize The new bit count.
	 */
	virtual void setSize(size_t newSize) = 0;

	/*!
	 * \brief Set a new set of frozen bits for this container.
	 *
	 * The information bit count and lookup table are updated according to the
	 * new Polar Code when calling this function.
	 *
	 * \param frozenBits The new set of frozen bits.
	 */
	void setFrozenBits(const std::vector<unsigned> &frozenBits);

	/*!
	 * \brief Fill internal storage with given data.
	 *
	 * This function reads mElementCount/8 bytes from pData and inserts it
	 * into internal storage in the needed bit format.
	 *
	 * \sa insertPackedInformationBits()
	 * \param pData Any kind of pointer to memory location of packed bytes
	 */
	virtual void insertPackedBits(const void* pData) = 0;

	/*!
	 * \brief Insert bits into non-frozen locations.
	 * \sa insertPackedBits()
	 * \param pData Information bits.
	 */
	virtual void insertPackedInformationBits(const void *pData) = 0;

	/*!
	 * \brief Insert byte-wise defined bits, for example after decoding.
	 * \param pData Pointer to char-bits.
	 */
	virtual void insertCharBits(const void* pData) = 0;

	/*!
	 * \brief Insert single precision floating point soft-bits of received code word.
	 * \param pLlr LLRs of received signal.
	 */
	virtual void insertLlr(const float *pLlr) = 0;

	/*!
	 * \brief Insert 8-bit quantized soft-bits of received code word.
	 * \param pLlr LLRs of received signal.
	 */
	virtual void insertLlr(const char  *pLlr) = 0;

	/*!
	 * \brief Write packed bits into pData.
	 *
	 * This function writes packed data into the preallocated memory location
	 * at pData. Note that pData must hold at least (mElementCount/8) bytes
	 * of allocated memory.
	 *
	 * \sa getPackedInformationBits()
	 * \param pData Pointer to destination memory for packed data.
	 */
	virtual void getPackedBits(void* pData) = 0;

	/*!
	 * \brief Write packed information bits into pData.
	 *
	 * This function iterates through internal storage and ignores frozen bits.
	 * Packed information bits are then written into pData. Note that
	 * at least (mElementCount-frozenBits.size())/8 bytes must be allocated
	 * to pData.
	 *
	 * \sa getPackedBits()
	 * \param pData Memory location for packed information data.
	 */
	virtual void getPackedInformationBits(void* pData) = 0;

	/*!
	 * \brief Copy the contents of the container into pData in container's
	 *        native LLR representation type.
	 * \param pData Memory location for container data.
	 */
	virtual void getSoftBits(void *pData) = 0;

	/*!
	 * \brief Extract LLRs of information bits into pData in container's
	 *        native LLR representation type.
	 * \param pData Memory location for information LLRs.
	 */
	virtual void getSoftInformation(void *pData) = 0;

	/*!
	 * \brief Set all frozen bits to zero.
	 */
	virtual void resetFrozenBits() = 0;
};

/*!
 * \brief A class that holds bits in floating point format of +/- zero.
 *
 * The polar coding calculations involving single precision floating point
 * arithmetic use a special bit format where the data bit is stored in the most
 * significant bit of the 32-bit word, which is the sign-bit. The absolute value
 * is fixed to zero and must not be other than zero as some functions use the
 * implicit sign()-function for XOR-operations, where ONLY the sign bit of the
 * other operand is to be changed according to the bit state.
 *
 * \sa CharContainer
 */
class FloatContainer : public BitContainer {
	float *mData;

public:
	FloatContainer();
	FloatContainer(size_t size);///<Initialize the container to specified size.
	FloatContainer(size_t size, std::vector<unsigned> &frozenBits);///<Configure this container to a given Polar Code.
	~FloatContainer();
	void setSize(size_t newSize);
	void insertPackedBits(const void* pData);
	void insertPackedInformationBits(const void *pData);
	void insertCharBits(const void* pData);
	void insertLlr(const float *pLlr);
	void insertLlr(const char  *pLlr);
	void getPackedBits(void* pData);
	void getPackedInformationBits(void* pData);
	void getSoftBits(void* pData);
	void getSoftInformation(void* pData);
	void resetFrozenBits();

	float* data();///< Get a pointer to the container's memory.
};


/*!
 * \brief A class that holds bits in classic (0,1)-format.
 *
 * Polar Coding on eight-bit integers has its own challenges, different from
 * those of floating point arithmetic. Multiplication has to be avoided.
 * Instead the bits can be stored in classic (0,1)-format, which allows them to
 * be used as a mask-operand.
 */
class CharContainer : public BitContainer {
	char *mData;
	bool mDataIsExternal;

public:
	CharContainer();
	CharContainer(size_t size);///<Initialize the container to specified size.
	CharContainer(char *external, size_t size);///<Assign an external storage to this container.
	CharContainer(size_t size, const std::vector<unsigned> &frozenBits);///<Configure this container to a given Polar Code.
	~CharContainer();
	void setSize(size_t newSize);
	void insertPackedBits(const void* pData);
	void insertPackedInformationBits(const void *pData);
	void insertCharBits(const void *pData);
	void insertLlr(const float *pLlr);
	void insertLlr(const char  *pLlr);
	void getPackedBits(void* pData);
	void getPackedInformationBits(void* pData);
	void getSoftBits(void* pData);
	void getSoftInformation(void* pData);
	void resetFrozenBits();

	char* data();///< Get a pointer to the container's memory.
};

/*!
 * \brief A class that holds packed bits for encoding.
 *
 * Encoding polar codes can be done fully utilizing the 256 bits per register
 * with AVX2-commands very similar to the char bit implementation.
 * This container cannot contain LLR values.
 */
class PackedContainer : public BitContainer {
	char *mData;
	size_t mFakeSize;
	bool mDataIsExternal;

	void insertBit(unsigned int bit, char value);
	void clearBit(unsigned int bit);

	void byteWiseInjection(const void *pData);
	void vectorWiseInjection(const void *pData);
	void fullyVectorizedInjection(const void *pData);

	void resetFrozenBitsSimple();
	void resetFrozenBitsVectorized();

public:
	PackedContainer();
	PackedContainer(size_t size);///<Initialize the container to specified size.
	PackedContainer(size_t size, std::vector<unsigned> &frozenBits);///<Configure this container to a given Polar Code.
	PackedContainer(char *external, size_t size);///<Assign an external storage to this container.
	~PackedContainer();
	void setSize(size_t newSize);
	void insertPackedBits(const void* pData);
	void insertPackedInformationBits(const void *pData);
	void insertCharBits(const void* pData);
	void getPackedBits(void* pData);
	void getPackedInformationBits(void* pData);
	void resetFrozenBits();

	/* The following functions are dummies */
	void insertLlr(const float *pLlr);
	void insertLlr(const char  *pLlr);
	void getSoftBits(void* pData);
	void getSoftInformation(void* pData);

	char* data();///< Get a pointer to the container's memory.
};

}//namespace PolarCode

#endif
