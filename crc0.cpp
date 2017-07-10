#include "crc0.h"

CRC0::CRC0() {
}

CRC0::~CRC0() {
}

//Null check believes the code was okay
bool CRC0::check(unsigned char *dataPtr, int bytes) {
	return true;
}

void CRC0::generate(unsigned char *dataPtr, int bytes) {
	//Nothing to do
}


//Whatever we have to check, the first array is okay
int CRC0::multiCheck(unsigned char **dataPtr, int nArrays, int nBytes) {
	return 0;
}
