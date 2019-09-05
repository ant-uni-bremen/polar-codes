#ifndef PCDSP_MODULATOR_H
#define PCDSP_MODULATOR_H

#include <polarcode/bitcontainer.h>

namespace SignalProcessing {
namespace Modulation {


/*!
 * \brief A skeleton class for any kind of data-to-signal converters (modulator)
 *        and data detectors (demodulator).
 */
class Modem {
protected:
	std::vector<float> *mInputSignal;///< Input to the (de-)modulator
	std::vector<float> *mOutputSignal;///< Output of the (de-)modulator

	bool mExternalInput;///< Helper to differentiate between external reference and internally allocated mInputSignal

public:
	Modem();
	virtual ~Modem();
	
	/*!
	 * \brief Tell the modem to get binary input data from any kind of BitContainer.
	 *
	 * When this function is called, it allocates its own mInputSignal vector.
	 * Otherwise the external vector is used, to minimize data copy operations.
	 */
	void setInputData(PolarCode::BitContainer *);

	/*!
	 * \brief For demodulators, set the location of an input signal.
	 */
	void setInputSignal(std::vector<float> *);
	

	/*!
	 * \brief Get a pointer to the output signal vector.
	 */
	std::vector<float>* outputSignal();

	/*!
	 * \brief Insert detected bits of the demodulated signal into a BitContainer.
	 */
	void getDataOutput(PolarCode::BitContainer *);
	

	/*!
	 * \brief Call the modulation routine.
	 */
	virtual void modulate() = 0;

	/*!
	 * \brief Call the demodulation routine.
	 */
	virtual void demodulate() = 0;

};

}//namespace Modulation
}//namespace SignalProcessing

#endif //PCDSP_MODULATOR_H
