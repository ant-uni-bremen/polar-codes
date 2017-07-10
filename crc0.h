#ifndef CRC0_H
#define CRC0_H

#include "crc.h"

class CRC0 : public CrcGenerator {

public:
	CRC0();
	~CRC0();
	
	void generate(unsigned char *dataPtr, int bytes);
	bool check(unsigned char *dataPtr, int bytes);
	int multiCheck(unsigned char **dataPtr, int nArrays, int nBytes);
};

#endif
