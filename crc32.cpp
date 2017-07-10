#include "crc32.h"

#include "nmmintrin.h"

CRC32::CRC32() {
}

CRC32::~CRC32() {
}

void CRC32::checkBlockSizeRestriction(int blockCount, int byteCount) {
	if((blockCount<<2) != byteCount) {
		throw "Crc32 failed: Block size does not fit 32 bit restriction!";
	}

}

bool CRC32::check(unsigned char *dataPtr, int bytes) {
	unsigned int toCheck = 0;
	int blocks = bytes>>2;
	unsigned int *data = reinterpret_cast<unsigned int*>(dataPtr);

	checkBlockSizeRestriction(blocks, bytes);

	for(int i=0; i<blocks-1; ++i) {
		toCheck = _mm_crc32_u32(toCheck, data[i]);
	}
	return toCheck == data[blocks-1];
}

void CRC32::generate(unsigned char *dataPtr, int bytes) {
	unsigned int chkSum = 0;
	int blocks = bytes>>2;
	unsigned int *data = reinterpret_cast<unsigned int*>(dataPtr);

	checkBlockSizeRestriction(blocks, bytes);

	for(int i=0; i<blocks-1; ++i) {
		chkSum = _mm_crc32_u32(chkSum, data[i]);
	}
	data[blocks-1] = chkSum;
}



int CRC32::multiCheck(unsigned char **dataPtr, int nArrays, int nBytes)
{
	unsigned int **data = reinterpret_cast<unsigned int**>(dataPtr);
	unsigned int *checksums = new unsigned int[nArrays]();

	int nCheckBlocks = (nBytes-4)>>2;

	checkBlockSizeRestriction(nCheckBlocks, nBytes-4);

	for(int block = 0; block < nCheckBlocks; ++block)
	{
		for(int array = 0; array < nArrays; ++array)
		{
			checksums[array] = _mm_crc32_u32(checksums[array], data[array][block]);
		}
	}

	int firstMatch = -1;
	for(int array = 0; array < nArrays; ++array)
	{
		if(checksums[array] == data[array][nCheckBlocks])
		{
			firstMatch = array;
			break;
		}
	}


	delete [] checksums;
	return firstMatch;
}
