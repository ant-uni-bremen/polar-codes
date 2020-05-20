#ifndef PC_DEC_SCAN_H
#define PC_DEC_SCAN_H

#include <polarcode/decoding/decoder.h>
#include <vector>

namespace PolarCode {
namespace Decoding {

/*!
 * \brief SCAN - Soft-output CANcellation
 *
 * Implementation of the algorithm presented in
 * "Low-Complexity Soft-Output Decoding of Polar Codes"
 * by Ubaid U. Fayyaz and John R. Barry
 *
 */
class Scan : public Decoder {
	std::vector<float> mLlr, mEven, mOdd;
	std::vector<float> mSystematicOutput;
	std::vector<bool> mBooleanFrozen;
	unsigned int mLevelCount, mN, mIterationLimit;

	unsigned int addressToIndex(unsigned level, unsigned group, unsigned bit);

	void updatellrmap(unsigned level, unsigned group);
	void updatebitmap(unsigned level, unsigned group);


public:
	Scan(size_t blockLength, unsigned iterationLimit, const std::vector<unsigned> &frozenBits);
	~Scan();

	bool decode();
	void initialize(size_t blockLength, unsigned iterationLimit, const std::vector<unsigned> &frozenBits);
	void setIterationLimit(unsigned iterationLimit);

	void getExtrinsicChannelInformation(float *);

	/*!
	 * \brief Get decoder list size
	 * \return size_t with Decoder List size.
	 */
	size_t getListSize(){
		return mIterationLimit;
	}
};


}// namespace Decoding
}// namespace PolarCode

#endif
