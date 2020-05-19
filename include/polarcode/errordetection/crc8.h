#ifndef PC_ERR_CRC8_H
#define PC_ERR_CRC8_H

#include <polarcode/errordetection/errordetector.h>

namespace PolarCode {
namespace ErrorDetection {

/*!
 * \brief Error detection via eight bit Cyclic Redundancy Check
 *
 * Polynom: 0x07
 * Init: 0x00
 * XorOut: 0x00
 * RefIn: false
 * RefOut: false
 */
class CRC8 : public Detector {
	unsigned char table[256];
	unsigned char gen(unsigned char *data, int bytes);

public:
	CRC8();
	~CRC8();

	std::string getType(){return std::string("CRC");}
	unsigned getCheckBitCount(){return 8;}
	void generate(void *pData, int bytes);
	bool check(void *pData, int bytes);
	int multiCheck(void **pData, int nArrays, int nBytes);
};

}//namespace ErrorDetection
}//namespace PolarCode

#endif //PC_ERR_CRC8_H
