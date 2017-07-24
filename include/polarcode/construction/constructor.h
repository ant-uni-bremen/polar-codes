#ifndef PC_CON_CONSTRUCTOR_H
#define PC_CON_CONSTRUCTOR_H

#include <cstddef>
#include <set>

namespace PolarCode {
namespace Construction {

class InvalidBlockLengthException{};

/**
 * \brief The Constructor class
 */
class Constructor {
protected:
	size_t mBlockLength;
	size_t mInformationLength;

public:
	Constructor();
	Constructor(size_t N, size_t K);
	virtual ~Constructor();

	/*!
	 * \brief Executes the construction algorithm.
	 * \return The set of frozen bits.
	 */
	virtual std::set<unsigned> construct() = 0;

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
};

}//namespace Construction
}//namespace PolarCode

#endif //PC_CON_CONSTRUCTOR_H
