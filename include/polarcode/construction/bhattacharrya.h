#ifndef PC_CON_BHATTACHARRYA_H
#define PC_CON_BHATTACHARRYA_H

#include <polarcode/construction/constructor.h>
#include <polarcode/arrayfuncs.h>
#include <vector>

namespace PolarCode {
namespace Construction {


class Bhattacharrya : public Constructor {
	float mInitialParameter;
	std::vector<float> mChannelParameters;
	trackingSorter mSorter;

	void calculateChannelParameters();

public:
	Bhattacharrya();
	Bhattacharrya(size_t N, size_t K);
	Bhattacharrya(size_t N, size_t K, float param);
	~Bhattacharrya();

	/*!
	 * \brief Executes the construction algorithm.
	 * \return The set of frozen bits.
	 */
	std::set<unsigned> construct();

	/*!
	 * \brief Set the Bhattacharrya parameter of the desired channel.
	 * \param newInitialParameter The initial parameter.
	 */
	void setInitialParameter(float newInitialParameter);

	/*!
	 * \brief Set the Bhattacharrya parameter depending on a design-SNR.
	 * \param designSNR The SNR of the channel the code will be designed for.
	 */
	void setParameterByDesignSNR(float designSNR);

};


}//namespace Construction
}//namespace PolarCode

#endif //PC_CON_BHATTACHARRYA_H
