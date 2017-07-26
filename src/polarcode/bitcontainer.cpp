#include <polarcode/bitcontainer.h>
#include <polarcode/avxconvenience.h>
#include <immintrin.h>
#include <cassert>
#include <cmath>
#include <cstring>

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
	: mElementCount(0) {
}

BitContainer::~BitContainer() {
}

FloatContainer::FloatContainer()
	: mData(nullptr) {
	static_assert(sizeof(float) == 4, "sizeof(float) must be 4 bytes!");
}

FloatContainer::FloatContainer(size_t size)
	: mData(nullptr) {
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
	if(newSize == mElementCount) return;

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

void FloatContainer::insertPackedInformationBits(const void *pData, std::set<unsigned> &frozenBits) {
	const unsigned char *charPtr = static_cast<const unsigned char*>(pData);
	unsigned int bitPool = charPtr[0]<<24;
	unsigned int bitCounter = 0, byteCounter = 1;
	unsigned int currentBit = 0;
	float *fBit = reinterpret_cast<float*>(&currentBit);

	for(unsigned int bit=0; bit<mElementCount; ++bit) {
		if(frozenBits.find(bit) == frozenBits.end()) {
			currentBit = bitPool&0x80000000;
			bitPool <<= 1;
			mData[bit] = *fBit;
			if(++bitCounter == 8 && bit+1 != mElementCount) {
				bitPool = charPtr[byteCounter++]<<24;
				bitCounter = 0;
			}
		} else {
			mData[bit] = 0.0;
		}
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

void FloatContainer::getPackedInformationBits(void* pData, std::set<unsigned> &frozenBits) {
	unsigned int bit=0;
	unsigned char *charPtr = static_cast<unsigned char*>(pData);
	unsigned char currentByte = 0, currentBit = 0;
	unsigned int *iBit = reinterpret_cast<unsigned int*>(mData);

	while(bit < mElementCount) {
		if(frozenBits.find(bit) == frozenBits.end()) {
			currentByte |= static_cast<unsigned char>(iBit[bit]>>(24+(currentBit++)));
			if(currentBit == 8) {
				*charPtr = currentByte;
				currentByte = 0;
				currentBit = 0;
				++charPtr;
			}
		}
		++bit;
	}
}

void FloatContainer::resetFrozenBits(std::set<unsigned> &frozenBits) {
	std::set<unsigned>::iterator it = frozenBits.begin();
	while(it != frozenBits.end()) {
		mData[*(it++)] = 0.0;
	}
}

float* FloatContainer::data() {
	return mData;
}




CharContainer::CharContainer()
	: mData(nullptr) {
}

CharContainer::CharContainer(size_t size)
	: mData(nullptr) {
	setSize(size);
}

CharContainer::~CharContainer() {
	if(mData != nullptr) {
		_mm_free(mData);
	}
}

void CharContainer::setSize(size_t newSize) {
	//Precautions
	assert(newSize%8 == 0);
	if(newSize == mElementCount) return;

	mElementCount = newSize;

	// Free previously allocated memory, if neccessary
	if(mData != nullptr) {
		_mm_free(mData);
	}

	// Allocate new memory
	mData = static_cast<char*>(_mm_malloc(mElementCount, BYTESPERVECTOR));
	if(mData == nullptr) {
		throw "Allocating memory for char-container failed.";
	}
}

void CharContainer::insertPackedBits(const void* pData) {
	// Char bits are mapped as follows: 0 => 0, 1 => 1
	unsigned int nBytes = mElementCount/8;
	const unsigned char *charPtr = static_cast<const unsigned char*>(pData);
	unsigned char bitPool;

	for(unsigned int byte=0; byte < nBytes; ++byte) {
		bitPool = charPtr[byte];
		for(unsigned int bit=0; bit < 8; ++bit) {
			mData[byte*8+bit] = (bitPool&0x80)>>7;
			bitPool <<= 1;
		}
	}
}

void CharContainer::insertPackedInformationBits(const void *pData, std::set<unsigned> &frozenBits) {
	const unsigned char *charPtr = static_cast<const unsigned char*>(pData);
	unsigned char bitPool = charPtr[0];
	unsigned int bitCounter = 0, byteCounter = 1;

	for(unsigned int bit=0; bit<mElementCount; ++bit) {
		if(frozenBits.find(bit) == frozenBits.end()) {
			mData[bit] = (bitPool&0x80)>>7;
			bitPool <<= 1;
			if(++bitCounter == 8 && bit+1 != mElementCount) {
				bitPool = charPtr[byteCounter++];
				bitCounter = 0;
			}
		} else {
			mData[bit] = 0;
		}
	}
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
	unsigned char *charPtr = static_cast<unsigned char*>(pData);
	unsigned char currentByte;

	for(unsigned int byte=0; byte<nBytes; ++byte) {
		currentByte = 0;
		for(unsigned int bit=0; bit<8; ++bit) {
			currentByte |= (mData[byte*8+bit]<<(7-bit));
		}
		charPtr[byte] = currentByte;
	}
}

void CharContainer::getPackedInformationBits(void* pData, std::set<unsigned> &frozenBits) {
	unsigned int bit=0;
	unsigned char *charPtr = static_cast<unsigned char*>(pData);
	unsigned char currentByte = 0, currentBit = 0;

	while(bit < mElementCount) {
		if(frozenBits.find(bit) == frozenBits.end()) {
			currentByte |= mData[bit]<<(7-(currentBit++));
			if(currentBit == 8) {
				*charPtr = currentByte;
				currentByte = 0;
				currentBit = 0;
				++charPtr;
			}
		}
		++bit;
	}
}

void CharContainer::resetFrozenBits(std::set<unsigned> &frozenBits) {
	std::set<unsigned>::iterator it = frozenBits.begin();
	while(it != frozenBits.end()) {
		mData[*(it++)] = 0;
	}
}

char* CharContainer::data() {
	return mData;
}


PackedContainer::PackedContainer()
	: mData(nullptr) {
}

PackedContainer::PackedContainer(size_t size)
	: mData(nullptr) {
	setSize(size);
}

PackedContainer::~PackedContainer() {
	if(mData != nullptr) {
		_mm_free(mData);
	}
}

void PackedContainer::setSize(size_t newSize) {
	//Precautions
	assert(newSize%8 == 0);
	if(newSize == mElementCount) return;

	mElementCount = newSize;
	fakeSize = std::max((size_t)256, mElementCount);

	// Free previously allocated memory, if neccessary
	if(mData != nullptr) {
		_mm_free(mData);
	}

	// Allocate new memory
	mData = static_cast<char*>(_mm_malloc(fakeSize/8, 32));
	if(mData == nullptr) {
		throw "Allocating memory for packed bit container failed.";
	}
}

void PackedContainer::insertPackedBits(const void* pData) {
	unsigned int nBytes = mElementCount/8;

	if(nBytes < fakeSize/8) {
		memcpy(mData+(32-nBytes), pData, nBytes);
	} else {
		memcpy(mData, pData, nBytes);
	}
}

void PackedContainer::insertPackedInformationBits(const void *pData, std::set<unsigned> &frozenBits) {
	const unsigned char *charPtr = static_cast<const unsigned char*>(pData);
	unsigned char bitPool = charPtr[0];
	unsigned int bitCounter = 0, byteCounter = 1;

	memset(mData, 0, fakeSize/8);

	for(unsigned int bit=0; bit<mElementCount; ++bit) {
		if(frozenBits.find(bit) == frozenBits.end()) {
			insertBit(bit, (bitPool&0x80)>>7);
			bitPool <<= 1;
			if(++bitCounter == 8 && bit+1 != mElementCount) {
				bitPool = charPtr[byteCounter++];
				bitCounter = 0;
			}
		}
	}
}

void PackedContainer::getPackedBits(void* pData) {
	unsigned int nBytes = mElementCount/8;
	memcpy(pData, mData+(fakeSize-mElementCount)/8, nBytes);
}

void PackedContainer::resetFrozenBits(std::set<unsigned> &frozenBits) {
	std::set<unsigned>::iterator it = frozenBits.begin();
	while(it != frozenBits.end()) {
		clearBit(*(it++));
	}
}

char* PackedContainer::data() {
	return mData;
}

//Dummy
void PackedContainer::insertLlr(const float *pLlr){}
void PackedContainer::insertLlr(const char  *pLlr){}
void PackedContainer::getPackedInformationBits(void* pData, std::set<unsigned> &frozenBits){}


void PackedContainer::insertBit(unsigned int bit, char value) {
	unsigned byteAddress;

	if(mElementCount<fakeSize) {
		byteAddress = (fakeSize-mElementCount+bit)/8;
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

	if(mElementCount<fakeSize) {
		byteAddress = (fakeSize-mElementCount+bit)/8;
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
