#include <polarcode/errordetection/crc32.h>

#include "nmmintrin.h"

namespace PolarCode {
namespace ErrorDetection {


CRC32::CRC32() {
}

CRC32::~CRC32() {
}

void CRC32::checkBlockSizeRestriction(int blockCount, int byteCount) {
	if((blockCount<<2) != byteCount) {
		throw "Crc32 failed: Block size does not fit 32 bit restriction!";
	}

}

unsigned int CRC32::gen(unsigned int *data, int blockCount) {
	unsigned int chkSum = 0;

	for(int i=0; i<blockCount; ++i) {
		chkSum = _mm_crc32_u32(chkSum, data[i]);
	}
	return chkSum;
}

bool CRC32::check(void *pData, int bytes) {
	unsigned int chkSum, savedChkSum;
	int reducedBlockCount = (bytes>>2)-1;
	unsigned int *data = reinterpret_cast<unsigned int*>(pData);

	//checkBlockSizeRestriction(reducedBlockCount+1, bytes);
	savedChkSum = data[reducedBlockCount];
	data[reducedBlockCount] = 0;

	chkSum = gen(data, reducedBlockCount);
	data[reducedBlockCount] = savedChkSum;

	return chkSum == data[reducedBlockCount];
}

void CRC32::generate(void *pData, int bytes) {
	unsigned int chkSum;
	int reducedBlockCount = (bytes/4)-1;
	unsigned int *data = reinterpret_cast<unsigned int*>(pData);

	//checkBlockSizeRestriction(reducedBlockCount+1, bytes);
	data[reducedBlockCount] = 0;

	chkSum = gen(data, reducedBlockCount);
	data[reducedBlockCount] = chkSum;
}



int CRC32::multiCheck(void **pData, int nArrays, int nBytes) {
	unsigned int **data = reinterpret_cast<unsigned int**>(pData);
	unsigned int *checksums = new unsigned int[nArrays]();

	int nCheckBlocks = (nBytes-4)/4;

	checkBlockSizeRestriction(nCheckBlocks, nBytes-4);

	for(int block = 0; block < nCheckBlocks; ++block) {
		for(int array = 0; array < nArrays; ++array) {
			checksums[array] = _mm_crc32_u32(checksums[array], data[array][block]);
		}
	}

	int firstMatch = -1;
	for(int array = 0; array < nArrays; ++array) {
		if(checksums[array] == data[array][nCheckBlocks]) {
			firstMatch = array;
			break;
		}
	}


	delete [] checksums;
	return firstMatch;
}


}//namespace ErrorDetection
}//namespace PolarCode
