#include <polarcode/bitcontainer.h>
#include <polarcode/avxconvenience.h>
#include <immintrin.h>
#include <cassert>
#include <cmath>
#include <cstring>
#include <algorithm>
#include <iostream>

namespace PolarCode {

/*!
 * \brief Restrict a value to boundaries.
 * \param min The minimum value
 * \param x The value to be restricted
 * \param max The maximum value
 * \return x, if it is between the boundaries, or the respective boundary.
 */
inline float frestrict(float min, float x, float max) {
	return fmin(fmax(x, min), max);
}

/*!
 * \brief Quantize a single precision floating point value to an eight-bit integer.
 * \param x The value to be quantized.
 * \return An eight-bit integer in the range of -128 to 127.
 */
inline char convertFtoC(float x) {
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
	  mInformationBitCount(size-frozenBits.size()),
	  mLUT(nullptr) {
	mFrozenBits.assign(frozenBits.begin(), frozenBits.end());
	calculateLUT();
}

BitContainer::~BitContainer() {
	clear();
}

void BitContainer::clear() {
	mFrozenBits.clear();
	mInformationBitCount = mElementCount;
	delete [] mLUT;
	mLUT = nullptr;
}

void BitContainer::calculateLUT() {
	mLUT = new unsigned[std::max(mInformationBitCount, 8U)];
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

size_t BitContainer::size() {
	return mElementCount;
}

void BitContainer::setFrozenBits(const std::vector<unsigned> &frozenBits) {
	clear();
	mFrozenBits.assign(frozenBits.begin(), frozenBits.end());
	mInformationBitCount = mElementCount - mFrozenBits.size();
	calculateLUT();
}

FloatContainer::FloatContainer()
	: mData(nullptr),
	  mDataIsExternal(false) {
	static_assert(sizeof(float) == 4, "sizeof(float) must be 4 bytes!");
}

FloatContainer::FloatContainer(size_t size)
	: BitContainer(size),
	  mData(nullptr),
	  mDataIsExternal(false) {
	setSize(size);
}

FloatContainer::FloatContainer(float *external, size_t size)
	: BitContainer(size), mData(external), mDataIsExternal(true) {
	mElementCount = size;
}

FloatContainer::FloatContainer(size_t size, std::vector<unsigned> &frozenBits)
	: BitContainer(size, frozenBits),
	  mData(nullptr),
	  mDataIsExternal(false) {
	setSize(size);
}

FloatContainer::~FloatContainer() {
	if(!mDataIsExternal) {
		_mm_free(mData);
	}
}

void FloatContainer::setSize(size_t newSize) {
	//Precautions
	assert(newSize%8 == 0);

	mElementCount = newSize;

	// Free previously allocated memory, if neccessary
	if(!mDataIsExternal) {
		_mm_free(mData);
	} else {
		mDataIsExternal = false;
	}

	// Allocate new memory
	mData = static_cast<float*>(_mm_malloc(4*mElementCount, 32/*BYTESPER(AVX)VECTOR*/));
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
	if(mInformationBitCount % 8 == 0) {
#ifdef __AVX2__
		//Masks
		const __m256i signmask = _mm256_set1_epi32(-0x80000000);
		const __m256i shift = _mm256_set_epi32(24,25,26,27,28,29,30,31);
		const __m256i reverse = _mm256_set_epi32(0,1,2,3,4,5,6,7);

		//Pointers
		int     const* cBit = reinterpret_cast<int*>(mData);
		__m256i const* cLUT = reinterpret_cast<__m256i*>(mLUT);
		const unsigned informationByteCount = mInformationBitCount/8;
		unsigned char* cData = (unsigned char*)pData;

		//Variables
		__m256i lut, vec;
		unsigned int currentByte;

		//Workload
		for(unsigned byte = 0; byte < informationByteCount; byte++) {
			lut = _mm256_loadu_si256(cLUT+byte);//load lookup table
			lut = _mm256_permutevar8x32_epi32(lut, reverse);//order it correctly
			vec = _mm256_i32gather_epi32(cBit, lut, 4);//collect info LLRs from positions given by lookup table
			vec = _mm256_and_si256(vec, signmask);//hard decode
			vec = _mm256_srlv_epi32(vec, shift);//move bits to position in output byte
			currentByte = reduce_or_epi32(vec);//merge split bits
			cData[byte] = static_cast<unsigned char>(currentByte);//save output
		}
#else
		unsigned char *charPtr = static_cast<unsigned char*>(pData);
		unsigned int currentByte = 0;
		unsigned int *iBit = reinterpret_cast<unsigned int*>(mData);

		for(unsigned bit = 0; bit < mInformationBitCount; bit+=8) {
			// less conditionals in this loop
			currentByte  = (iBit[mLUT[bit+0]]&0x80000000)>>24;
			currentByte |= (iBit[mLUT[bit+1]]&0x80000000)>>25;
			currentByte |= (iBit[mLUT[bit+2]]&0x80000000)>>26;
			currentByte |= (iBit[mLUT[bit+3]]&0x80000000)>>27;
			currentByte |= (iBit[mLUT[bit+4]]&0x80000000)>>28;
			currentByte |= (iBit[mLUT[bit+5]]&0x80000000)>>29;
			currentByte |= (iBit[mLUT[bit+6]]&0x80000000)>>30;
			currentByte |= (iBit[mLUT[bit+7]]&0x80000000)>>31;
			charPtr[bit/8] = static_cast<unsigned char>(currentByte);
		}
#endif
	} else {
		unsigned char *charPtr = static_cast<unsigned char*>(pData);
		unsigned int currentByte = 0, currentBit = 24;
		unsigned int *iBit = reinterpret_cast<unsigned int*>(mData);
		unsigned *lutPtr = mLUT;

		for(unsigned bit = 0; bit < mInformationBitCount; ++bit) {
			currentByte |= (iBit[*(lutPtr++)]&0x80000000)>>(currentBit++);
			if(currentBit == 32) {
				*charPtr = static_cast<unsigned char>(currentByte);
				currentByte = 0;
				currentBit = 24;
				++charPtr;
			}
		}
		if(currentBit != 24) {
			*charPtr = static_cast<unsigned char>(currentByte);
		}
	}
}

void FloatContainer::getSoftBits(void *pData) {
	memcpy(pData, mData, mElementCount*sizeof(float));
}

void FloatContainer::vectorizedHardDecode(float *dst) {
	const __m256 sgnMask = _mm256_set1_ps(-0.0);

	for(unsigned bit=0; bit < mElementCount; bit+=8) {
		__m256 softbit = _mm256_load_ps(mData+bit);
		__m256 hardbit = _mm256_and_ps(sgnMask, softbit);
		_mm256_store_ps(dst+bit, hardbit);
	}
}

void FloatContainer::simpleHardDecode(float *dst) {
	const unsigned int sgnMask = 0x80000000U;

	unsigned int *iDst = reinterpret_cast<unsigned int*>(dst);
	unsigned int *iSrc = reinterpret_cast<unsigned int*>(mData);

	for(unsigned bit=0; bit < mElementCount; bit++) {
		iDst[bit] = iSrc[bit] & sgnMask;
	}
}

void FloatContainer::getFloatBits(float *pData) {
	if(mElementCount>=8) {
		vectorizedHardDecode(pData);
	} else {
		simpleHardDecode(pData);
	}
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
	if(!mDataIsExternal) {
		_mm_free(mData);
	}
}

void CharContainer::setSize(size_t newSize) {
	//Precautions
	assert(newSize%8 == 0);
	assert(!mDataIsExternal);

	mElementCount = newSize;

	// Free previously allocated memory, if neccessary
	if(!mDataIsExternal) {
		_mm_free(mData);
	} else {
		mDataIsExternal = false;
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

void vectorizedFtoC(char *cPtr, const float *fPtr, const unsigned size) {
	const __m256 maximum = _mm256_set1_ps(127.0);
	const __m256 minimum = _mm256_set1_ps(-128.0);
	__m256 fvec;
	__m256i ivec;
	char* cvec = reinterpret_cast<char*>(&ivec);
	for(unsigned i=0; i<size; i+=8) {
		fvec = _mm256_loadu_ps(fPtr+i);
		fvec = _mm256_max_ps(fvec, minimum);
		fvec = _mm256_min_ps(fvec, maximum);
		fvec = _mm256_round_ps(fvec, _MM_FROUND_TO_NEAREST_INT);
		ivec = _mm256_cvtps_epi32(fvec);
		for(int j=0; j<8; ++j) {
			cPtr[i+j] = cvec[j*4];
		}
	}

}

void CharContainer::insertLlr(const float *pLlr) {
	if(mElementCount >= 8) {
		vectorizedFtoC(mData, pLlr, mElementCount);
	} else {
		for(unsigned int bit=0; bit < mElementCount; ++bit) {
			mData[bit] = convertFtoC(pLlr[bit]);
		}
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
	unsigned char currentByte = 0;
	union {
		unsigned char bits[8];
		__m64 bitmask;
	};

	unsigned safeBytes = mInformationBitCount/8;
	unsigned safeBits = safeBytes*8;
	unsigned remainingBits = mInformationBitCount%8;

	for(unsigned bit = 0; bit < safeBits; bit+=8) {
		for(int j=0; j<8; ++j) {
			bits[7-j] = uData[mLUT[bit+j]];
		}
		*(charPtr++) = static_cast<unsigned char>(_mm_movemask_pi8(bitmask));
	}

	// Assemble the remaining bits
	if(remainingBits) {
		for(unsigned bit=0; bit < remainingBits; ++bit) {
			currentByte |= (uData[mLUT[bit+safeBits]]&0x80) >> bit;
		}
		*charPtr = currentByte;
	}
}

void CharContainer::getSoftBits(void *pData) {
	memcpy(pData, mData, mElementCount*sizeof(char));
}

void CharContainer::getFloatBits(float *pData) {
	unsigned char *coData = reinterpret_cast<unsigned char*>(pData)+3;
	unsigned char *ciData = reinterpret_cast<unsigned char*>(mData);
	memset(pData, 0, 4*mElementCount);

	for(unsigned bit = 0; bit < mElementCount; ++bit) {
		coData[4*bit] = ciData[bit]&0x80;
	}
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
	: mData(nullptr)
	, mInformationMask(nullptr)
	, mDataIsExternal(false) {
}

PackedContainer::PackedContainer(size_t size)
	: BitContainer(size)
	, mData(nullptr)
	, mInformationMask(nullptr)
	, mDataIsExternal(false) {
	setSize(size);
}

PackedContainer::PackedContainer(size_t size, std::vector<unsigned> &frozenBits)
	: BitContainer(size, frozenBits)
	, mData(nullptr)
	, mInformationMask(nullptr)
	, mDataIsExternal(false) {
	setSize(size);
}

PackedContainer::PackedContainer(char *external, size_t size, std::vector<unsigned> &frozenBits)
	: BitContainer(size, frozenBits)
	, mData(external)
	, mInformationMask(nullptr)
	, mFakeSize(std::max((size_t)BITSPERVECTOR, size))
	, mDataIsExternal(true) {
	buildInformationMask();
}

PackedContainer::~PackedContainer() {
	if(!mDataIsExternal) {
		_mm_free(mData);
	}
	delete [] mInformationMask;
}

void PackedContainer::setSize(size_t newSize) {
	//Precautions
	assert(newSize % 8 == 0);

	mElementCount = newSize;
	mFakeSize = std::max((size_t)BITSPERVECTOR, mElementCount);

	// Free previously allocated memory
	if(!mDataIsExternal) {
		_mm_free(mData);
	} else {
		mDataIsExternal = false;
	}

	delete [] mInformationMask;
	mInformationMask = nullptr;

	// Allocate new memory
	mData = static_cast<char*>(_mm_malloc(mFakeSize / 8, BYTESPERVECTOR));
	if(mData == nullptr) {
		throw "Allocating memory for packed bit container failed.";
	}
	buildInformationMask();
}

void PackedContainer::buildInformationMask() {
	unsigned bitOffset = mFakeSize - mElementCount;
	unsigned frozenCounter = 0, frozenBitCount = mFrozenBits.size();
	unsigned begin = 0;

	if(mInformationMask == nullptr) {
		mInformationMask = new unsigned long[mFakeSize / 64];
	}

	if(bitOffset >= 64) {
		begin = bitOffset / 64;
		bitOffset %= 64;
		for(unsigned i = 0; i < begin; ++i) {
			mInformationMask[i] = 0;
		}
	}

	for(unsigned i = begin; i < mFakeSize / 64; ++i) {
		unsigned long mask = ~0ULL;//set all bits
		//clear frozen bits
		while(frozenCounter < frozenBitCount && mFrozenBits[frozenCounter] < (i - begin + 1) * 64) {//while the next frozen bit is in this qword
			unsigned long bit = mFrozenBits[frozenCounter++] % 64;
			bit = (bit & ~7ULL) + (7 - (bit % 8)) + bitOffset;//pay attention to endianness
			mask ^= (1ULL << bit);//clear the mask bit
		}
		mInformationMask[i] = mask;
	}
}

void PackedContainer::insertPackedBits(const void* pData) {
	unsigned int nBytes = mElementCount / 8;

	if(nBytes < mFakeSize / 8) {
		memcpy(mData+(BYTESPERVECTOR - nBytes), pData, nBytes);
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

#ifdef __AVX2__
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
#else
void PackedContainer::vectorWiseInjection(const void *pData) {
	const unsigned char *charPtr = static_cast<const unsigned char*>(pData);
	unsigned char bitPool = *(charPtr++);
	unsigned *lutPtr = mLUT;

	short* iData = reinterpret_cast<short*>(mData);
	__m128i tempVector = _mm_setzero_si128();
	unsigned char *cTemp = reinterpret_cast<unsigned char*>(&tempVector);
	unsigned vectorIndex = 0;

	for(unsigned infoBit = 0; infoBit < mInformationBitCount; ++infoBit) {
		unsigned infoDestination = *(lutPtr++);
		infoDestination = (infoDestination & 0xFFFFFFF8) + 7 - (infoDestination % 8);
		if(infoDestination / 16 != vectorIndex) {
			int packedBits = _mm_movemask_epi8(tempVector);
			iData[vectorIndex] = packedBits;
			tempVector = _mm_setzero_si128();
			vectorIndex = infoDestination / 16;
		}
		cTemp[infoDestination % 16] = bitPool & 0x80;
		if(infoBit % 8 == 7) {
			bitPool = *(charPtr++);
		} else {
			bitPool <<= 1;
		}
	}
	short packedBits = _mm_movemask_epi8(tempVector);
	iData[vectorIndex] = packedBits;
}
#endif

inline int bitVecAddress(int index) {
	const int byteIndex = (3 - index/8) * 8;
	const int bitIndex = index%8;
	const int vectorIndex = 31 - (byteIndex+bitIndex);
	return vectorIndex;
}

#ifdef __AVX2__
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
#else
void PackedContainer::fullyVectorizedInjection(const void *pData) {
	const unsigned short *inputPtr = static_cast<const unsigned short*>(pData);
	__m128i inputVector = _mm_get_mask_epi8(inputPtr[0]);
	char* inputChar = reinterpret_cast<char*>(&inputVector);
	int currentInfoChunk = 0;

	short *outputPtr = reinterpret_cast<short*>(mData);
	__m128i outputVector = _mm_get_mask_epi8(outputPtr[0]);
	char* outputChar = reinterpret_cast<char*>(&outputVector);
	int currentOutputChunk = 0;

	for(unsigned infoBit = 0; infoBit < mInformationBitCount; ++infoBit) {
		int infoChunk = infoBit/16;
		if(infoChunk != currentInfoChunk) {
			inputVector = _mm_get_mask_epi8(inputPtr[infoChunk]);
			currentInfoChunk = infoChunk;
		}
		int outputChunk = mLUT[infoBit]/16;
		if(outputChunk != currentOutputChunk) {
			outputPtr[currentOutputChunk] = _mm_movemask_epi8(outputVector);
			outputVector = _mm_setzero_si128();//loading not needed
			currentOutputChunk = outputChunk;
		}

		int inputAddress = bitVecAddress(infoBit%16);
		int outputAddress = bitVecAddress(mLUT[infoBit]%16);

		outputChar[outputAddress] = inputChar[inputAddress];
	}
	outputPtr[currentOutputChunk] = _mm_movemask_epi8(outputVector);
}
#endif

void PackedContainer::insertPackedInformationBits(const void *pData) {

#if 1
	unsigned nPackedVectors = mFakeSize / BITSPERVECTOR;
	memset(mData, 0, mFakeSize / 8);
	if(nPackedVectors == 1) {
		byteWiseInjection(pData);
	} else if (mInformationBitCount < BYTESPERVECTOR) {
		vectorWiseInjection(pData);
	} else {
		fullyVectorizedInjection(pData);
	}
#else

#warning TODO Check this

	Problem: the pdep function works from low bit to high bit, but bit insertion
			is expected to work from high to low. Otherwise this code would
			work nicely.

	unsigned long *uliData = (unsigned long*)pData;
	unsigned long *uloData = reinterpret_cast<unsigned long*>(mData);
	unsigned long input = __bswap_64(uliData[0]), newBits;
	unsigned long loadedBits = 64;
	unsigned long mask, bitCount, availableBitCount = 64;

	for(unsigned i = 0; i<mFakeSize/64; ++i) {
		//Get information bit positions and count
		mask = __bswap_64(mInformationMask[i]);
		bitCount = _mm_popcnt_u64(mask);

		//Load new bits, if neccessary
		while(bitCount > availableBitCount) {
			unsigned long nextWordToLoadFrom = loadedBits/64;
			unsigned long startBit = loadedBits%64;
			unsigned long bitsAvailableInThatWord =
					(startBit>availableBitCount) ? //check, if this word has enough new bits
					64-startBit : //if not, load all it provides
					64-startBit-availableBitCount;//if yes, load only the needed amount
			//load the bit providing qword
			unsigned long resourceWord = __bswap_64(uliData[nextWordToLoadFrom]);
			//extract the needed bits
			unsigned long freshBits = _bextr_u64(resourceWord, startBit, bitsAvailableInThatWord);
			//shift them to their new position in the input word
			freshBits <<= availableBitCount;
			//save them in the input word
			input |= freshBits;
			//update counters
			availableBitCount += bitsAvailableInThatWord;
			loadedBits += bitsAvailableInThatWord;
		}

		//Insert information bits and save
		newBits = _pdep_u64(input, mask);// <- such magic, much wow!
		uloData[i] = __bswap_64(newBits);

		//Shift away inserted bits
		if(bitCount < 64)
			input >>= bitCount;
		else
			input = 0;
		//Update counter
		availableBitCount -= bitCount;
	}
#endif
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

void PackedContainer::insertLlr(const float* pData) {
	unsigned char *coData = reinterpret_cast<unsigned char*>(mData);
	const unsigned int *iiData = reinterpret_cast<const unsigned int*>(pData);
	unsigned offset = 0;

	if(mFakeSize != mElementCount) {
		offset = (mFakeSize-mElementCount)/8;
	}

	for(unsigned groupbit=0; groupbit < mElementCount; groupbit+=8) {
		unsigned char currentByte = 0;
		for(unsigned bit=0; bit < 8; ++bit) {
			currentByte |= (iiData[groupbit+bit]&0x80000000)>>(bit+24);
		}
		coData[groupbit/8+offset] = currentByte;
	}
}

void PackedContainer::getPackedBits(void* pData) {
	unsigned int nBytes = mElementCount / 8;
	if(mFakeSize != mElementCount) {
		memcpy(pData, mData + (mFakeSize - mElementCount) / 8, nBytes);
	} else {
		memcpy(pData, mData, nBytes);
	}
}

void PackedContainer::resetFrozenBits() {
	unsigned long *lData = reinterpret_cast<unsigned long*>(mData);
	for(unsigned i = 0; i < mFakeSize / 64; ++i) {
		lData[i] &= mInformationMask[i];
	}
}

char* PackedContainer::data() {
	return mData;
}

//Dummy
void PackedContainer::insertLlr(const char*){}
void PackedContainer::getSoftBits(void*){}
void PackedContainer::getSoftInformation(void*){}

void PackedContainer::getFloatBits(float *pData) {
	unsigned char *coData = reinterpret_cast<unsigned char*>(pData)+3;//offset to MSB
	unsigned char *ciData = reinterpret_cast<unsigned char*>(mData);
	memset(pData, 0, 4*mElementCount);

	for(unsigned groupbit = 0; groupbit < mElementCount; groupbit += 8) {
		for(unsigned bit = 0; bit < 8; ++bit) {
			coData[4*(groupbit+bit)] = (ciData[groupbit/8]<<bit)&0x80;
		}
	}
}


void PackedContainer::getPackedInformationBits(void* pData) {
#ifdef __AVX2__
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
#else
	const unsigned offset = mFakeSize - mElementCount;
	const unsigned short *inputPtr = reinterpret_cast<const unsigned short*>(mData);
	__m128i inputVector = _mm_get_mask_epi8(inputPtr[offset / 16]);
	char* inputChar = reinterpret_cast<char*>(&inputVector);
	int currentInfoChunk = 0;

	short *outputPtr = reinterpret_cast<short*>(pData);
	__m128i outputVector = _mm_setzero_si128();
	char* outputChar = reinterpret_cast<char*>(&outputVector);
	int currentOutputChunk = 0;

	for(unsigned infoBit = 0; infoBit < mInformationBitCount; ++infoBit) {
		int infoChunk = mLUT[infoBit] / 16;
		if(infoChunk != currentInfoChunk) {
			inputVector = _mm_get_mask_epi8(inputPtr[infoChunk + offset / 16]);
			currentInfoChunk = infoChunk;
		}
		int outputChunk = infoBit / 16;
		if(outputChunk != currentOutputChunk) {
			outputPtr[currentOutputChunk] = _mm_movemask_epi8(outputVector);
			outputVector = _mm_setzero_si128();//loading not needed
			currentOutputChunk = outputChunk;
		}

		int inputAddress = bitVecAddress((mLUT[infoBit] + offset) % 16);
		int outputAddress = bitVecAddress(infoBit % 16);

		outputChar[outputAddress] = inputChar[inputAddress];
	}
	outputPtr[currentOutputChunk] = _mm_movemask_epi8(outputVector);
}
#endif

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
