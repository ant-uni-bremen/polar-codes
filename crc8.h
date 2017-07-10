#ifndef CRC8_H
#define CRC8_H

#include "crc.h"

class CRC8 : public CrcGenerator {
	unsigned char table[256];
	void gen(unsigned char *crc, unsigned char m);
	
public:
	CRC8();
	~CRC8();
	
	void generate(unsigned char *data, int bytes);
	bool check(unsigned char *data, int bytes);
	int multiCheck(unsigned char **data, int nArrays, int nBytes);
};

#endif
