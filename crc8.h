#ifndef CRC8_H
#define CRC8_H

#include <vector>

class CRC8
{
	unsigned char table[256];
	void gen(unsigned char *crc, unsigned char m);
	
	std::vector<unsigned char> tmpBytes;
	
public:
	CRC8();
	~CRC8();
	
	bool check(unsigned char *data, int bytes, unsigned char checksum);
	unsigned char generate(unsigned char *data, int bytes);
	void generate(unsigned char *data, int bytes, unsigned char *checksum);
	void addChecksum(std::vector<float> &data);
	void addChecksum(float* data, int nBits);
	bool check(std::vector<float> &data);
	bool check(float* data, int nBits);
	int multiCheck(unsigned char **data, int nArrays, int nBytes);
};

#endif
