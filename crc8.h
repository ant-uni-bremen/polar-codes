#ifndef CRC8_H
#define CRC8_H

#include <vector>

class CRC8
{
	unsigned char table[256];
	void gen(unsigned char *crc, unsigned char m);
	
public:
	CRC8();
	~CRC8();
	
	bool check(unsigned char *data, int bytes, unsigned char checksum);
	unsigned char generate(unsigned char *data, int bytes);
	void generate(unsigned char *data, int bytes, unsigned char *checksum);
	void addChecksum(std::vector<bool> &data);
	bool check(std::vector<bool> &data);
};

#endif
