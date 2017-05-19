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
		//gen(&toCheck, data[i]);
		toCheck = table[toCheck ^ data[i]];
	}
	return toCheck == checksum;
}

unsigned char CRC8::generate(unsigned char *data, int bytes)
{
	unsigned char ret = 0;
	for(int i=0; i<bytes; ++i)
	{
		//gen(&ret, data[i]);
		ret = table[ret ^ data[i]];
	}
	return ret;
}

void CRC8::generate(unsigned char *data, int bytes, unsigned char *crc)
{
	*crc = generate(data, bytes);
}

void CRC8::addChecksum(std::vector<float> &data)
{
	int nBits = data.size(), nBytes = nBits>>3;
	tmpBytes.resize(nBytes);
	unsigned char crcbits[8], crcbyte;

	Bits2Bytes(data, tmpBytes.data(), nBytes);
	crcbyte = generate(tmpBytes.data(), nBytes);
	Bytes2Bits(&crcbyte, crcbits, 1);

	data.resize(nBits+8);
	unsigned int* iData = reinterpret_cast<unsigned int*>(data.data());
	for(int i=0; i<8; ++i)
	{
		iData[nBits+i] = ((unsigned int)crcbits[i]) << 31;
	}
}

void CRC8::addChecksum(float* data, int nBits)
{
	int nBytes = nBits>>3;
	tmpBytes.resize(nBytes);
	unsigned char crcbits[8], crcbyte;

	Bits2Bytes(data, tmpBytes.data(), nBytes);
	crcbyte = generate(tmpBytes.data(), nBytes);
	Bytes2Bits(&crcbyte, crcbits, 1);

	unsigned int* iData = reinterpret_cast<unsigned int*>(data);
	for(int i=0; i<8; ++i)
	{
		iData[nBits+i] = ((unsigned int)crcbits[i]) << 31;
	}
}

bool CRC8::check(std::vector<float> &data)
{
	int nBits = data.size(), nBytes = nBits>>3;
	tmpBytes.resize(nBytes);

	Bits2Bytes(data, tmpBytes.data(), nBytes);
	bool result = check(tmpBytes.data(), nBytes-1, tmpBytes[nBytes-1]);

	return result;
}

bool CRC8::check(float* data, int nBits)
{
	int nBytes = nBits>>3;
	tmpBytes.resize(nBytes);

	Bits2Bytes(data, tmpBytes.data(), nBytes);
	bool result = check(tmpBytes.data(), nBytes-1, tmpBytes[nBytes-1]);

	return result;
}

int CRC8::multiCheck(unsigned char **data, int nArrays, int nBytes)
{
	unsigned char *checksums = new unsigned char[nArrays]();
	int nCheckBytes = nBytes-1;

	for(int byte = 0; byte < nCheckBytes; ++byte)
	{
		for(int array = 0; array < nArrays; ++array)
		{
			checksums[array] = table[checksums[array] ^ data[array][byte]];
		}
	}

	int firstMatch = -1;
	for(int array = 0; array < nArrays; ++array)
	{
		if(checksums[array] == data[array][nBytes-1])
		{
			firstMatch = array;
			break;
		}
	}


	delete [] checksums;
	return firstMatch;
}
