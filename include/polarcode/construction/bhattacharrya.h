#ifndef PC_CON_BHATTACHARRYA_H
#define PC_CON_BHATTACHARRYA_H

#include <polarcode/construction/constructor.h>
#include <polarcode/arrayfuncs.h>
#include <vector>

namespace PolarCode {
namespace Construction {

/*!
 * \brief Code Construction via Bhattacharrya channel parameter
 *
 * This class implements Polar Code Construction via the intuitively created
 * method of calculating the upper bound on error probability for a given
 * channel.
 *
 * See "Channel Polarization: A Method for Constructing Capacity-Achieving Codes
 * for Symmetric Binary-Input Memoryless Channels" by Erdal Arıkan
 * Published in: IEEE TRANSACTIONS ON INFORMATION THEORY, VOL. 55, NO. 7, JULY 2009
 */
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