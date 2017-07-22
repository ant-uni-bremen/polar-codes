#ifndef PC_ERR_ERRORDETECTOR_H
#define PC_ERR_ERRORDETECTOR_H

namespace PolarCode {
namespace ErrorDetection {

/**
 * \brief The skeleton class for error detecting algorithms
 *
 * The error detector has to check the received information for decoding errors.
 * This class is a purely virtual skeleton class which has to be reimplemented
 * by the derived error detection algorithm class.
 */
class Detector {
public:
	Detector(){}
	virtual ~Detector(){}

	/*!
	 * \brief Replaces the last bytes of data with a checksum.
	 * \param data Memory location to generate the checksum of.
	 * \param bytes Number of bytes of data including the checksum.
	 */
	virtual void generate(void *data, int bytes) = 0;
	virtual bool check(void *data, int bytes) = 0;
	virtual int multiCheck(void **data, int nArrays, int nBytes) = 0;

};

}//namespace ErrorDetection
}//namespace PolarCode

#endif //PC_ERR_ERRORDETECTOR_H
