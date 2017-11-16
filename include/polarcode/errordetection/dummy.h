#ifndef PC_ERR_DUMMY_H
#define PC_ERR_DUMMY_H

#include <polarcode/errordetection/errordetector.h>

namespace PolarCode {
namespace ErrorDetection {

/**
 * \brief This class defines the no-op error detector
 *
 * The dummy error detector does absolutely nothing, aside from removing the
 * need to check if error detection should be done in a particular
 * encoder/decoder.
 * The dummy does not alter input bytes, nor does it do any kind of error
 * detection. It simply assumes no error has occured or will occur.
 */
class Dummy : public Detector {

public:
	Dummy();
	~Dummy();

	/*!
	 * \brief Get another Dummy error detection object.
	 * \return A new Dummy object.
	 */
	Dummy* clone();

	unsigned getCheckBitCount();

	/*!
	 * \brief Does not do anything.
	 * \param dataPtr Memory location that might hold data.
	 * \param bytes Number of bytes there might be.
	 */
	void generate(void *dataPtr, int bytes);

	/*!
	 * \brief Does not perform any kind of error detection.
	 * \param dataPtr Memory location not to be checked.
	 * \param bytes Number of bytes there might be.
	 * \return True.
	 */
	bool check(void *dataPtr, int bytes);

	/*!
	 * \brief Does not check any of the arrays.
	 * \param dataPtr May point anywhere.
	 * \param nArrays Number of arrays there might be.
	 * \param nBytes Number of bytes there might be per array.
	 * \return 0.
	 */
	int multiCheck(void **dataPtr, int nArrays, int nBytes);
};

extern Dummy globalDummyDetector;

}//namespace ErrorDetection
}//namespace PolarCode

#endif //PC_ERR_DUMMY_H
