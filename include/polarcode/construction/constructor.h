#ifndef PC_CON_CONSTRUCTOR_H
#define PC_CON_CONSTRUCTOR_H

#include <cstddef>
#include <vector>

namespace PolarCode {
namespace Construction {

class InvalidBlockLengthException{};

/**
 * \brief The Constructor class
 */
class Constructor {
protected:
	size_t mBlockLength;///< Number of code bits.
	size_t mInformationLength;///< Number of information bits.

public:
	Constructor();
	virtual ~Constructor();

	/*!
	 * \brief Executes the construction algorithm.
	 * \return The set of frozen bits.
	 */
	virtual std::vector<unsigned> construct() = 0;

	/*!
	 * \brief Set the block length of the code to construct.
	 * \param newBlockLength The desired code word length.
	 */
	void setBlockLength(size_t newBlockLength) throw (InvalidBlockLengthException);

	/*!
	 * \brief Set the number of information bits in the code to construct.
	 * \param newInformationLength The desired amount of information bits.
	 */
	void setInformationLength(size_t newInformationLength);

	/*!
	 * \brief Set the design parameter by passing the design SNR in [dB].
	 * \param designSNR The desired amount of information bits.
	 */
	virtual void setParameterByDesignSNR(float designSNR);
};

		std::vector<unsigned> frozen_bits(const int blockLength, const int infoLength, const float designSNR);

}//namespace Construction
}//namespace PolarCode

#endif //PC_CON_CONSTRUCTOR_H
