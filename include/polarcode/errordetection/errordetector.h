#ifndef PC_ERR_ERRORDETECTOR_H
#define PC_ERR_ERRORDETECTOR_H

#include <string>

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
	 * \brief Get the number of bits that are added by the detector.
	 * \return Number of check bits.
	 */
	virtual unsigned getCheckBitCount() = 0;

	/*!
	 * \brief Replaces the last bytes of data with a checksum.
	 * \param data Memory location to generate the checksum of.
	 * \param bytes Number of bytes of data including the checksum.
	 */
	virtual void generate(void *data, int bytes) = 0;

	/*!
	 * \brief Check validity of the data.
	 * \param data Memory location of data to be checked.
	 * \param bytes Number of bytes, including the size of the checksum.
	 * \return True, if the checksum is correct. False otherwise.
	 */
	virtual bool check(void *data, int bytes) = 0;

	/*!
	 * \brief Check a list of memory locations for checksum validity.
	 *
	 * This function serves for list decoding, where multiple candidate code
	 * words are generated. The first candidate that passes the error detection
	 * test is to be found and its index will be returned.
	 *
	 * \param data Pointer to pointers to memory that will be checked.
	 * \param nArrays Number of locations to check.
	 * \param nBytes Number of bytes per data array, including the size of the checksum.
	 * \return The index of the first array that contains valid data.
	 */
	virtual int multiCheck(void **data, int nArrays, int nBytes) = 0;

};

	/*!
	 * \brief Create new Detector with specified parameters
	 *
	 * Serves as a wrapper to ease Detector creation
	 *
	 * \param size Checksum size in bits, must align to bytes, e.g. 0, 8, 16, ...
	 * \param type Detector type. Currently CRC or CMAC.
	 * \return A new Detector object.
	 */
Detector* create(unsigned size, std::string type);

}//namespace ErrorDetection
}//namespace PolarCode

#endif //PC_ERR_ERRORDETECTOR_H
