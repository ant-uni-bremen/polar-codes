#ifndef CRC32_H
#define CRC32_H

#include "crc.h"

class CRC32 : public CrcGenerator {
	void checkBlockSizeRestriction(int blockCount, int byteCount);

public:
	CRC32();
	~CRC32();
	
	void generate(unsigned char *dataPtr, int bytes);
	bool check(unsigned char *dataPtr, int bytes);
	int multiCheck(unsigned char **dataPtr, int nArrays, int nBytes);
};

#endif
