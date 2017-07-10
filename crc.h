#ifndef CRC_H
#define CRC_H

class CrcGenerator {
public:
	CrcGenerator(){}
	virtual ~CrcGenerator(){}

	virtual void generate(unsigned char *data, int bytes) = 0;
	virtual bool check(unsigned char *data, int bytes) = 0;
	virtual int multiCheck(unsigned char **data, int nArrays, int nBytes) = 0;

};

#endif // CRC_H
