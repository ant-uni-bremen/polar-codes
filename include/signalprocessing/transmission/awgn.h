#ifndef PCDSP_TRANSMITTER_AWGN_H
#define PCDSP_TRANSMITTER_AWGN_H

#include <signalprocessing/transmission/transmitter.h>
#include <signalprocessing/random.h>

namespace SignalProcessing {
namespace Transmission {

/*!
 * \brief This class implements the behaviour of an AWGN-channel.
 *
 * Additive White Gaussian Noise is a simple-yet-close-to-reality model of
 * general transmission channels. When dealing with fading channels, including
 * enough diversity can help in getting back close to an AWGN-channels (multiple
 * antennas, slower transmission, using more bandwidth.. which result in
 * exploiting space-, time- and frequency diversity). So, if enough diversity
 * can be used, the last thing to deal with is noise. Mostly, the sum of all
 * kinds of noise sources is Gaussian-distributed, with constant power over the
 * entire frequency band, which is called "white" noise.
 *
 * Adding noise to a signal gives the input signal to a detection system.
 * The signal-to-noise power ratio is the single parameter of an AWGN-channel.
 * It leads to the amount of recoverable information per symbol.
 */
class Awgn : public Transmitter {
	Random::Generator *mRandGen;

	float mEsNoLog, mEsNoLin, mNoiseMagnitude;

	void transmit_simple();
	void transmit_vectorized();
public:
	Awgn();
	/*!
	 * \brief Construct an Awgn channel with given signal-to-noise ratio per symbol.
	 * \param EsN0_dB Signal-to-noise ratio (symbol energy per noise energy) in dB.
	 */
	Awgn(float EsN0_dB);
	~Awgn();
	
	/*!
	 * \brief Set a new SNR given in dB.
	 */
	void setEsN0(float);

	/*!
	 * \brief Set SNR by linear ratio.
	 */
	void setEsN0Linear(float);

	/*!
	 * \brief Get the current SNR-setting.
	 * \return The current SNR-setting.
	 */
	float EsNo();

	/*!
	 * \brief Get linear E_S/N_0
	 * \return Current E_S/N_0 in linear domain
	 */
	float EsNoLin();

	void transmit();
	
};
    
}//namespace Transmission
}//namespace SignalProcessing

#endif //PCDSP_TRANSMITTER_AWGN_H
