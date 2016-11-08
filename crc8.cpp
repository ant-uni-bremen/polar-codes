#include "crc8.h"
#include "ArrayFuncs.h"

#define GP  0x107   /* x^8 + x^2 + x + 1 */
#define DI  0x07

CRC8::CRC8()
{
	int i,j;
	unsigned char crc;

	for (i=0; i<256; i++)
	{
		crc = i;
		for (j=0; j<8; j++)
		{
			crc = (crc << 1) ^ ((crc & 0x80) ? DI : 0);
		}
		table[i] = crc & 0xFF;
	}
}

CRC8::~CRC8()
{
	
}

void CRC8::gen(unsigned char *crc, unsigned char m)
     /*
      * For a byte array whose accumulated crc value is stored in *crc, computes
      * resultant crc obtained by appending m to the byte array
      */
{
  *crc = table[(*crc) ^ m];
  *crc &= 0xFF;
}

bool CRC8::check(unsigned char *data, int bytes, unsigned char checksum)
{
	unsigned char toCheck = 0;
	for(int i=0; i<bytes; ++i)
	{
		gen(&toCheck, data[i]);
	}
	return toCheck == checksum;
}

unsigned char CRC8::generate(unsigned char *data, int bytes)
{
	unsigned char ret = 0;
	for(int i=0; i<bytes; ++i)
	{
		gen(&ret, data[i]);
	}
	return ret;
}

void CRC8::generate(unsigned char *data, int bytes, unsigned char *crc)
{
	*crc = generate(data, bytes);
}

void CRC8::addChecksum(std::vector<bool> &data)
{
	//Now, this function got a bit more complicated by the need to convert
	//between c++-vector and c-array
	int nBits = data.size(), nBytes = nBits>>3;
	unsigned char *bits = new unsigned char[nBits];
	unsigned char *bytes = new unsigned char[nBytes];
	unsigned char crcbits[8], crcbyte;
	
	for(int i=0; i<nBits; ++i)
	{
		bits[i] = data[i];
	}
	Bits2Bytes(bits, bytes, nBytes);
	crcbyte = generate(bytes, nBytes);
	Bytes2Bits(&crcbyte, crcbits, 1);
	for(int i=0; i<8; ++i)
	{
		data.push_back(crcbits[i]);
	}
	delete[] bits;
	delete[] bytes;
}

bool CRC8::check(std::vector<bool> &data)
{
	int nBits = data.size(), nBytes = nBits>>3;
	unsigned char *bits = new unsigned char[nBits];
	unsigned char *bytes = new unsigned char[nBytes];
	
	for(int i=0; i<nBits; ++i)
	{
		bits[i] = data[i];
	}
	Bits2Bytes(bits, bytes, nBytes);
	bool result = check(bytes, nBytes-1, bytes[nBytes-1]);
	delete[] bits;
	delete[] bytes;	
	return result;
}
