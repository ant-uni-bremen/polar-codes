#include <polarcode/errordetection/crc16.h>

#include "CRC.h"

namespace PolarCode {
namespace ErrorDetection {


CRC16::CRC16() {
}

CRC16::~CRC16() {
}

uint16_t CRC16::gen(uint16_t *data, int byteSize) {
	return CRC::Calculate(data, byteSize, CRC::CRC_16_XMODEM());
}

bool CRC16::check(void *pData, int bytes) {
	uint16_t *data = reinterpret_cast<uint16_t*>(pData);
	uint8_t *p = reinterpret_cast<uint8_t*>(pData);
	const uint16_t rx_checksum = (uint16_t(p[bytes - 2] << 8) |
                                  uint16_t(p[bytes - 1]));
	const auto checksum = gen(data, bytes - 2);
	return checksum == rx_checksum;
}

void CRC16::generate(void *pData, int bytes) {
	uint16_t *data = reinterpret_cast<uint16_t*>(pData);
	auto checksum = gen(data, bytes - 2);

	uint8_t *p = reinterpret_cast<uint8_t*>(pData);
	p[bytes - 2] = (checksum >> 8) & 0xFF;
	p[bytes - 1] = checksum & 0xFF;
}



int CRC16::multiCheck(void **pData, int nArrays, int nBytes) {
	uint16_t **data = reinterpret_cast<uint16_t**>(pData);

	int firstMatch = -1;
	for(int array = 0; array < nArrays; ++array) {
		if(check(data[array], nBytes)){
			firstMatch = array;
			break;
		}
	}

	return firstMatch;
}

}//namespace ErrorDetection
}//namespace PolarCode
