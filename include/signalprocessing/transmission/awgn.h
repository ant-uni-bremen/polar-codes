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
 * Adding noise n to a signal s, their sum gives the input signal x to a
 * detection system.
 * The signal-to-noise power ratio is the single parameter of an AWGN-channel.
 * It leads to the amount of recoverable information per symbol.
 */
class Awgn : public Transmitter {
	Random::Generator *mRandGen;
	Random::LCG<__m256> *mLcg;

	float mEsNoLog, mEsNoLin, mNoiseMagnitude;

	void transmit_simple();
	void transmit_vectorized();
public:
	Awgn();
	/*!
	 * \brief Construct an Awgn channel with given signal-to-noise ratio per symbol.
	 * \param EsNo_dB Signal-to-noise ratio (symbol energy per noise energy) in dB.
	 */
	Awgn(float EsNo_dB);
	~Awgn();
	
	/*!
	 * \brief Set a new SNR.
	 */
	void setEsNo(float);

	/*!
	 * \brief Get the current SNR-setting.
	 * \return The current SNR-setting.
	 */
	float EsNo();

	void transmit();
	
};
    
}//namespace Transmission
}//namespace SignalProcessing

#endif //PCDSP_TRANSMITTER_AWGN_H
