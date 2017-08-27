#include <polarcode/bitcontainer.h>
#include <polarcode/avxconvenience.h>
#include <immintrin.h>
#include <cassert>
#include <cmath>
#include <cstring>
#include <algorithm>

namespace PolarCode {

/*!
 * \brief Restrict a value to boundaries.
 * \param min The minimum value
 * \param x The value to be restricted
 * \param max The maximum value
 * \return x, if it is between the boundaries, or the respective boundary.
 */
float frestrict(float min, float x, float max) {
	return fmin(fmax(x, min), max);
}

/*!
 * \brief Quantize a single precision floating point value to an eight-bit integer.
 * \param x The value to be quantized.
 * \return An eight-bit integer in the range of -128 to 127.
 */
char convertFtoC(float x) {
	x = frestrict(-128.0, x, 127.0);
	x = round(x);
	return static_cast<char>(x);
}

BitContainer::BitContainer()
	: mElementCount(0),
	  mFrozenBits({}),
	  mInformationBitCount(0),
	  mLUT(nullptr) {
}

BitContainer::BitContainer(size_t size)
	: mElementCount(size),
	  mFrozenBits({}),
	  mInformationBitCount(size),
	  mLUT(nullptr) {
}

BitContainer::BitContainer(size_t size, const std::vector<unsigned> &frozenBits)
	: mElementCount(size),
	  mFrozenBits(frozenBits),
	  mInformationBitCount(size-frozenBits.size()),
	  mLUT(nullptr) {
	calculateLUT();
}

BitContainer::~BitContainer() {
	clear();
}

void BitContainer::clear() {
	mFrozenBits.clear();
	mInformationBitCount = mElementCount;
	if(mLUT != nullptr) {
		delete [] mLUT;
		mLUT = nullptr;
	}
}

void BitContainer::calculateLUT() {
	mLUT = new unsigned[mInformationBitCount];
	unsigned lutCounter = 0;
	unsigned frozenCounter = 0, countMax = mFrozenBits.size();
	for(unsigned i=0; i<mElementCount; ++i) {
		if(frozenCounter == countMax) {
			mLUT[lutCounter++] = i;
		} else {
			if(mFrozenBits[frozenCounter] > i)  {
				mLUT[lutCounter++] = i;
			} else {
				frozenCounter++;
			}
		}
	}
}

void BitContainer::setFrozenBits(const std::vector<unsigned> &frozenBits) {
	clear();
	//mFrozenBits = frozenBits;
	mFrozenBits.assign(frozenBits.begin(), frozenBits.end());
	mInformationBitCount = mElementCount - mFrozenBits.size();
	calculateLUT();
}

FloatContainer::FloatContainer()
	: mData(nullptr) {
	static_assert(sizeof(float) == 4, "sizeof(float) must be 4 bytes!");
}

FloatContainer::FloatContainer(size_t size)
	: BitContainer(size),
	  mData(nullptr) {
	setSize(size);
}

FloatContainer::FloatContainer(size_t size, std::vector<unsigned> &frozenBits)
	: BitContainer(size, frozenBits),
	  mData(nullptr) {
	setSize(size);
}

FloatContainer::~FloatContainer() {
	if(mData != nullptr) {
		_mm_free(mData);
	}
}

void FloatContainer::setSize(size_t newSize) {
	//Precautions
	assert(newSize%8 == 0);

	mElementCount = newSize;

	// Free previously allocated memory, if neccessary
	if(mData != nullptr) {
		_mm_free(mData);
	}

	// Allocate new memory
	mData = static_cast<float*>(_mm_malloc(4*mElementCount, BYTESPERVECTOR));
	if(mData == nullptr) {
		throw "Allocating memory for float-container failed.";
	}
}

void FloatContainer::insertPackedBits(const void* pData) {
	// Float bits are mapped as follows: 0 => 0.0, 1 => -0.0
	unsigned int nBytes = mElementCount/8;
	const unsigned char *charPtr = static_cast<const unsigned char*>(pData);
	unsigned int bitPool, currentBit;
	float *fBit = reinterpret_cast<float*>(&currentBit);

	for(unsigned int byte=0; byte < nBytes; ++byte) {
		bitPool = static_cast<unsigned int>(charPtr[byte])<<24;
		for(unsigned int bit=0; bit < 8; ++bit) {
			currentBit = bitPool&0x80000000;
			mData[byte*8+bit] = *fBit;
			bitPool <<= 1;
		}
	}
}

void FloatContainer::insertPackedInformationBits(const void *pData) {
	const unsigned char *charPtr = static_cast<const unsigned char*>(pData);
	unsigned int bitPool = charPtr[0]<<24;
	unsigned int bitCounter = 0, byteCounter = 1;
	unsigned int currentBit = 0;
	float *fBit = reinterpret_cast<float*>(&currentBit);
	unsigned *lutPtr = mLUT;

	memset(mData, 0, mElementCount*4);

	for(unsigned int bit=0; bit<mInformationBitCount; ++bit) {
		currentBit = bitPool&0x80000000;
		bitPool <<= 1;
		mData[*(lutPtr++)] = *fBit;
		if(++bitCounter == 8 && bit+1 != mElementCount) {
			bitPool = charPtr[byteCounter++]<<24;
			bitCounter = 0;
		}
	}
}

void FloatContainer::insertCharBits(const void *apData) {
	const char *pData = static_cast<const char*>(apData);
	unsigned int *iPtr = reinterpret_cast<unsigned int*>(mData);
	for(unsigned int bit = 0; bit < mElementCount; ++bit) {
		iPtr[bit] = convertFtoC(pData[bit]);
	}
}

void FloatContainer::insertLlr(const float *pLlr) {
	memcpy(mData, pLlr, 4*mElementCount);
}

void FloatContainer::insertLlr(const char *pLlr) {
	for(unsigned int bit = 0; bit < mElementCount; ++bit) {
		mData[bit] = static_cast<float>(pLlr[bit]);
	}
}

void FloatContainer::getPackedBits(void* pData) {
	unsigned int nBytes = mElementCount/8;
	unsigned char *charPtr = static_cast<unsigned char*>(pData);
	unsigned char currentByte;
	unsigned int *iBit = reinterpret_cast<unsigned int*>(mData);

	for(unsigned int byte=0; byte<nBytes; ++byte) {
		currentByte = 0;
		for(unsigned int bit=0; bit<8; ++bit) {
			currentByte |= (iBit[byte*8+bit]>>(24+bit));
		}
		charPtr[byte] = currentByte;
	}
}

void FloatContainer::getPackedInformationBits(void* pData) {
	unsigned char *charPtr = static_cast<unsigned char*>(pData);
	unsigned char currentByte = 0, currentBit = 0;
	unsigned int *iBit = reinterpret_cast<unsigned int*>(mData);
	unsigned *lutPtr = mLUT;

	for(unsigned bit = 0; bit < mInformationBitCount; ++bit) {
		currentByte |= static_cast<unsigned char>(iBit[*(lutPtr++)]>>(24+(currentBit++)));
		if(currentBit == 8) {
			*charPtr = currentByte;
			currentByte = 0;
			currentBit = 0;
			++charPtr;
		}
	}
}

void FloatContainer::getSoftBits(void *pData) {
	memcpy(pData, mData, mElementCount*sizeof(float));
}

void FloatContainer::getSoftInformation(void *pData) {
	float *fData = static_cast<float*>(pData);
	unsigned *lutPtr = mLUT;

	for(unsigned bit = 0; bit < mInformationBitCount; ++bit) {
		*(fData++) = mData[*(lutPtr++)];
	}
}

void FloatContainer::resetFrozenBits() {
	for(unsigned i : mFrozenBits) {
		mData[i] = 0.0;
	}
}

float* FloatContainer::data() {
	return mData;
}




CharContainer::CharContainer()
	: mData(nullptr), mDataIsExternal(false) {
}

CharContainer::CharContainer(size_t size)
	: BitContainer(size), mData(nullptr), mDataIsExternal(false) {
	setSize(size);
}

CharContainer::CharContainer(size_t size, const std::vector<unsigned> &frozenBits)
	: BitContainer(size, frozenBits), mData(nullptr), mDataIsExternal(false) {
	setSize(size);
}

CharContainer::CharContainer(char *external, size_t size)
	: BitContainer(size), mData(external), mDataIsExternal(true) {
	mElementCount = size;
}

CharContainer::~CharContainer() {
	if(mData != nullptr && !mDataIsExternal) {
		_mm_free(mData);
	}
}

void CharContainer::setSize(size_t newSize) {
	//Precautions
	assert(newSize%8 == 0);
	assert(!mDataIsExternal);

	mElementCount = newSize;

	// Free previously allocated memory, if neccessary
	if(mData != nullptr) {
		_mm_free(mData);
	}

	size_t allocBytes = std::max(static_cast<size_t>(BYTESPERVECTOR), mElementCount);

	// Allocate new memory
	mData = static_cast<char*>(_mm_malloc(allocBytes, BYTESPERVECTOR));
	if(mData == nullptr) {
		throw "Allocating memory for char-container failed.";
	}
}

void CharContainer::insertPackedBits(const void* pData) {
	// Char bits are mapped as follows: 0 => 127, 1 => 128 (-128 when signed)
/*	{
		unsigned char x = 0x80;
		char y;
		y = 127+(x>>7);
		assert(y == -128);
	}*/
	unsigned int nBytes = mElementCount/8;
	const unsigned char *charPtr = static_cast<const unsigned char*>(pData);
	unsigned char bitPool;

	for(unsigned int byte=0; byte < nBytes; ++byte) {
		bitPool = charPtr[byte];
		for(unsigned int bit=0; bit < 8; ++bit) {
			mData[byte*8+bit] = 127+(bitPool>>7);
			bitPool <<= 1;
		}
	}
}

void CharContainer::insertPackedInformationBits(const void *pData) {
	const unsigned char *charPtr = static_cast<const unsigned char*>(pData);
	unsigned char bitPool = *(charPtr++);
	unsigned int bitCounter = 0;
	unsigned *lutPtr = mLUT;

	memset(mData, 0, mElementCount);

	for(unsigned int bit=0; bit<mInformationBitCount; ++bit) {
		mData[*(lutPtr++)] = 127+(bitPool>>7);
		bitPool <<= 1;
		if(++bitCounter == 8 && bit+1 != mElementCount) {
			bitPool = *(charPtr++);
			bitCounter = 0;
		}
	}
}

void CharContainer::insertCharBits(const void *pData) {
	memcpy(mData, pData, mElementCount);
}

void CharContainer::insertLlr(const float *pLlr) {
	for(unsigned int bit=0; bit < mElementCount; ++bit) {
		mData[bit] = convertFtoC(pLlr[bit]);
	}
}

void CharContainer::insertLlr(const char *pLlr) {
	memcpy(mData, pLlr, mElementCount);
}

void CharContainer::getPackedBits(void* pData) {
	unsigned int nBytes = mElementCount/8;
	unsigned char *uData = reinterpret_cast<unsigned char *>(mData);
	unsigned char *charPtr = static_cast<unsigned char*>(pData);
	unsigned char currentByte;

	for(unsigned int byte=0; byte<nBytes; ++byte) {
		currentByte = 0;
		for(unsigned int bit=0; bit<8; ++bit) {
			currentByte |= (uData[byte*8+bit] & 0x80) >> bit;
		}
		charPtr[byte] = currentByte;
	}
}

void CharContainer::getPackedInformationBits(void* pData) {
	unsigned char *charPtr = static_cast<unsigned char*>(pData);
	unsigned char *uData = reinterpret_cast<unsigned char *>(mData);
	unsigned char currentByte = 0, currentBit = 0;
	unsigned *lutPtr = mLUT;

	for(unsigned bit = 0; bit < mInformationBitCount; ++bit) {
		currentByte |= (uData[*(lutPtr++)] & 0x80) >> (currentBit++);
		if(currentBit == 8) {
			*charPtr = currentByte;
			currentByte = 0;
			currentBit = 0;
			++charPtr;
		}
	}

	if(mFrozenBits.size()%8 != 0) {
		*charPtr = currentByte;
	}
}

void CharContainer::getSoftBits(void *pData) {
	memcpy(pData, mData, mElementCount*sizeof(char));
}

void CharContainer::getSoftInformation(void *pData) {
	char *cData = static_cast<char*>(pData);
	unsigned *lutPtr = mLUT;

	for(unsigned bit = 0; bit < mInformationBitCount; ++bit) {
		*(cData++) = mData[*(lutPtr++)];
	}
}

void CharContainer::resetFrozenBits() {
	for(unsigned i : mFrozenBits) {
		mData[i] = 0;
	}
}

char* CharContainer::data() {
	return mData;
}


PackedContainer::PackedContainer()
	: mData(nullptr),
	  mDataIsExternal(false) {
}

PackedContainer::PackedContainer(size_t size)
	: BitContainer(size),
	  mData(nullptr),
	  mDataIsExternal(false) {
	setSize(size);
}

PackedContainer::PackedContainer(size_t size, std::vector<unsigned> &frozenBits)
	: BitContainer(size, frozenBits),
	  mData(nullptr),
	  mDataIsExternal(false) {
	setSize(size);
}

PackedContainer::PackedContainer(char *external, size_t size)
	: BitContainer(size),
	  mData(external),
	  mFakeSize(std::max((size_t)256, size)),
	  mDataIsExternal(true) {
}

PackedContainer::~PackedContainer() {
	if(mData != nullptr && !mDataIsExternal) {
		_mm_free(mData);
	}
}

void PackedContainer::setSize(size_t newSize) {
	//Precautions
	assert(newSize%8 == 0);

	mElementCount = newSize;
	mFakeSize = std::max((size_t)256, mElementCount);

	// Free previously allocated memory, if neccessary
	if(mData != nullptr) {
		_mm_free(mData);
	}

	// Allocate new memory
	mData = static_cast<char*>(_mm_malloc(mFakeSize/8, 32));
	if(mData == nullptr) {
		throw "Allocating memory for packed bit container failed.";
	}
}

void PackedContainer::insertPackedBits(const void* pData) {
	unsigned int nBytes = mElementCount/8;

	if(nBytes < mFakeSize/8) {
		memcpy(mData+(32-nBytes), pData, nBytes);
	} else {
		memcpy(mData, pData, nBytes);
	}
}

void PackedContainer::byteWiseInjection(const void *pData) {
	const unsigned char *charPtr = static_cast<const unsigned char*>(pData);
	unsigned char bitPool = *(charPtr++);
	unsigned *lutPtr = mLUT;

	unsigned char *uData = reinterpret_cast<unsigned char*>(mData);
	unsigned char currentByte = 0;
	int currentDestinationByte = -1;

	uData += (mFakeSize-mElementCount)/8;

	for(unsigned infoBit = 0; infoBit < mInformationBitCount; ++infoBit) {
		int infoDestination = *(lutPtr++);
		if(infoDestination/8 > currentDestinationByte) {
			if(currentDestinationByte != -1) {
				uData[currentDestinationByte] = currentByte;
				currentByte = 0;
			}
			currentDestinationByte = infoDestination/8;
		}
		currentByte |= (bitPool&0x80)>>(infoDestination%8);
		if(infoBit%8 == 7 && infoBit+1 != mInformationBitCount) {
			bitPool = *(charPtr++);
		} else {
			bitPool <<= 1;
		}
	}
	uData[currentDestinationByte] = currentByte;
}

void PackedContainer::vectorWiseInjection(const void *pData) {
	const unsigned char *charPtr = static_cast<const unsigned char*>(pData);
	unsigned char bitPool = *(charPtr++);
	unsigned *lutPtr = mLUT;

	int* iData = reinterpret_cast<int*>(mData);
	__m256i tempVector = _mm256_setzero_si256();
	unsigned char *cTemp = reinterpret_cast<unsigned char*>(&tempVector);
	unsigned vectorIndex = 0;

	for(unsigned infoBit = 0; infoBit < mInformationBitCount; ++infoBit) {
		unsigned infoDestination = *(lutPtr++);
		infoDestination = (infoDestination&0xFFFFFFF8)+7-(infoDestination%8);
		if(infoDestination/32 != vectorIndex) {
			int packedBits = _mm256_movemask_epi8(tempVector);
			iData[vectorIndex] = packedBits;
			tempVector = _mm256_setzero_si256();
			vectorIndex = infoDestination/32;
		}
		cTemp[infoDestination%32] = bitPool&0x80;
		if(infoBit%8 == 7) {
			bitPool = *(charPtr++);
		} else {
			bitPool <<= 1;
		}
	}
	int packedBits = _mm256_movemask_epi8(tempVector);
	iData[vectorIndex] = packedBits;
}

int bitVecAddress(int index) {
	const int byteIndex = (3 - index/8) * 8;
	const int bitIndex = index%8;
	const int vectorIndex = 31 - (byteIndex+bitIndex);
	return vectorIndex;
}

void PackedContainer::fullyVectorizedInjection(const void *pData) {
	const unsigned int *inputPtr = static_cast<const unsigned int*>(pData);
	__m256i inputVector = _mm256_get_mask_epi8(inputPtr[0]);
	char* inputChar = reinterpret_cast<char*>(&inputVector);
	int currentInfoChunk = 0;

	int *outputPtr = reinterpret_cast<int*>(mData);
	__m256i outputVector = _mm256_get_mask_epi8(outputPtr[0]);
	char* outputChar = reinterpret_cast<char*>(&outputVector);
	int currentOutputChunk = 0;

	for(unsigned infoBit = 0; infoBit < mInformationBitCount; ++infoBit) {
		int infoChunk = infoBit/32;
		if(infoChunk != currentInfoChunk) {
			inputVector = _mm256_get_mask_epi8(inputPtr[infoChunk]);
			currentInfoChunk = infoChunk;
		}
		int outputChunk = mLUT[infoBit]/32;
		if(outputChunk != currentOutputChunk) {
			outputPtr[currentOutputChunk] = _mm256_movemask_epi8(outputVector);
			outputVector = _mm256_setzero_si256();//loading not needed
			currentOutputChunk = outputChunk;
		}

		int inputAddress = bitVecAddress(infoBit%32);
		int outputAddress = bitVecAddress(mLUT[infoBit]%32);

		outputChar[outputAddress] = inputChar[inputAddress];
	}
	outputPtr[currentOutputChunk] = _mm256_movemask_epi8(outputVector);
}

void PackedContainer::insertPackedInformationBits(const void *pData) {
	unsigned nPackedVectors = mFakeSize/256;
	memset(mData, 0, mFakeSize/8);

	if(nPackedVectors == 1) {
		byteWiseInjection(pData);
	} else if (mInformationBitCount < 32) {
		vectorWiseInjection(pData);
	} else {
		fullyVectorizedInjection(pData);
	}
}

void PackedContainer::insertCharBits(const void *pData) {
	unsigned int nBytes = mElementCount/8;
	unsigned char *outPtr = reinterpret_cast<unsigned char*>(mData);
	const unsigned char *inPtr  = static_cast<const unsigned char*>(pData);
	unsigned char currentByte;

	for(unsigned int byte=0; byte<nBytes; ++byte) {
		currentByte = 0;
		for(unsigned int bit=0; bit<8; ++bit) {
			currentByte |= (inPtr[byte*8+bit] & 0x80) >> bit;
		}
		outPtr[byte] = currentByte;
	}
}

void PackedContainer::getPackedBits(void* pData) {
	unsigned int nBytes = mElementCount/8;
	if(mFakeSize != mElementCount) {
		memcpy(pData, mData+(mFakeSize-mElementCount)/8, nBytes);
	} else {
		memcpy(pData, mData, nBytes);
	}
}

void PackedContainer::resetFrozenBits() {
	if(mElementCount < 256) {
		resetFrozenBitsSimple();
	} else {
		resetFrozenBitsVectorized();
	}
}

void PackedContainer::resetFrozenBitsSimple() {
	for(unsigned i : mFrozenBits) {
		clearBit(i);
	}
}

void PackedContainer::resetFrozenBitsVectorized() {
	__m256i currentChunk;
	char* chunkPtr = reinterpret_cast<char*>(&currentChunk);
	const int chunkSize = 32;
	unsigned int *uiData = reinterpret_cast<unsigned int*>(mData);//get mask expects unsigned as input
	int *iData = reinterpret_cast<int*>(mData);//movemask creates signed as output
	currentChunk = _mm256_get_mask_epi8(uiData[0]);
	unsigned chunkIndex = 0;
	for(unsigned frozenIndex : mFrozenBits) {
		unsigned bitChunkIndex = frozenIndex/chunkSize;
		if(bitChunkIndex != chunkIndex) {
			iData[chunkIndex] = _mm256_movemask_epi8(currentChunk);
			currentChunk = _mm256_get_mask_epi8(uiData[bitChunkIndex]);
			chunkIndex = bitChunkIndex;
		}
		unsigned address = bitVecAddress(frozenIndex%chunkSize);
		chunkPtr[address] = 0;
	}
	iData[chunkIndex] = _mm256_movemask_epi8(currentChunk);
}

char* PackedContainer::data() {
	return mData;
}

//Dummy
void PackedContainer::insertLlr(const float*){}
void PackedContainer::insertLlr(const char*){}
void PackedContainer::getSoftBits(void*){}
void PackedContainer::getSoftInformation(void*){}


void PackedContainer::getPackedInformationBits(void* pData) {
	const unsigned offset = mFakeSize-mElementCount;
	const unsigned int *inputPtr = reinterpret_cast<const unsigned int*>(mData);
	__m256i inputVector = _mm256_get_mask_epi8(inputPtr[offset/32]);
	char* inputChar = reinterpret_cast<char*>(&inputVector);
	int currentInfoChunk = 0;

	int *outputPtr = reinterpret_cast<int*>(pData);
	__m256i outputVector = _mm256_setzero_si256();
	char* outputChar = reinterpret_cast<char*>(&outputVector);
	int currentOutputChunk = 0;

	for(unsigned infoBit = 0; infoBit < mInformationBitCount; ++infoBit) {
		int infoChunk = mLUT[infoBit]/32;
		if(infoChunk != currentInfoChunk) {
			inputVector = _mm256_get_mask_epi8(inputPtr[infoChunk+offset/32]);
			currentInfoChunk = infoChunk;
		}
		int outputChunk = infoBit/32;
		if(outputChunk != currentOutputChunk) {
			outputPtr[currentOutputChunk] = _mm256_movemask_epi8(outputVector);
			outputVector = _mm256_setzero_si256();//loading not needed
			currentOutputChunk = outputChunk;
		}

		int inputAddress = bitVecAddress((mLUT[infoBit]+offset)%32);
		int outputAddress = bitVecAddress(infoBit%32);

		outputChar[outputAddress] = inputChar[inputAddress];
	}
	outputPtr[currentOutputChunk] = _mm256_movemask_epi8(outputVector);
}


void PackedContainer::insertBit(unsigned int bit, char value) {
	unsigned byteAddress;

	if(mElementCount<mFakeSize) {
		byteAddress = (mFakeSize-mElementCount+bit)/8;
	} else {
		byteAddress = bit/8;
	}

	char currentByte = mData[byteAddress];
	unsigned int offset = bit%8;
	currentByte |= (value<<(7-offset));
	mData[byteAddress] = currentByte;

}

void PackedContainer::clearBit(unsigned int bit) {
	unsigned byteAddress;

	if(mElementCount<mFakeSize) {
		byteAddress = (mFakeSize-mElementCount+bit)/8;
	} else {
		byteAddress = bit/8;
	}

	char currentByte = mData[byteAddress];
	unsigned int offset = bit%8;
	char mask = 1<<(7-offset);//Set the bit to be cleared in mask
	mask = ~mask;//Invert for bit removal operation
	currentByte &= mask;//Clear the bit
	mData[byteAddress] = currentByte;//Save the result
}

}//namespace PolarCode
