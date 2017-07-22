#include <polarcode/bitcontainer.h>
#include <polarcode/avxconvenience.h>
#include <immintrin.h>
#include <cassert>

namespace PolarCode {

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

void FloatContainer::insertPacked(const void* pData) {
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

void FloatContainer::insertPackedInformation(const void *pData, std::set<unsigned> &frozenBits) {
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

void FloatContainer::getPacked(void* pData) {
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

void FloatContainer::getPackedInformation(void* pData, std::set<unsigned> &frozenBits) {
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

void CharContainer::insertPacked(const void* pData) {
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

void CharContainer::insertPackedInformation(const void *pData, std::set<unsigned> &frozenBits) {
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

void CharContainer::getPacked(void* pData) {
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

void CharContainer::getPackedInformation(void* pData, std::set<unsigned> &frozenBits) {
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


}//namespace PolarCode
