#ifndef PC_CON_CONSTRUCTOR_H
#define PC_CON_CONSTRUCTOR_H

#include <cstddef>
#include <set>

namespace PolarCode {
namespace Construction {

/**
 * \brief The Constructor class
 */
class Constructor {
protected:
	size_t mBlockLength;
	size_t mInformationLength;

public:
	Constructor();
	virtual ~Constructor() = 0;

	/**
	 * \brief Executes the construction algorithm.
	 * \return The set of frozen bits.
	 */
	virtual std::set<unsigned> construct() = 0;
};

}//namespace Construction
}//namespace PolarCode

#endif //PC_CON_CONSTRUCTOR_H
