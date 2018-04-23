#ifndef PCDSP_MODULATION_ASK
#define PCDSP_MODULATION_ASK

#include <signalprocessing/modulation/modem.h>
#include <signalprocessing/modulation/bpsk.h>

namespace SignalProcessing {
namespace Modulation {


/*!
 * \brief The ASK-modulator maps groups of bits to single real-valued symbols.
 *
 * As BPSK is a bottleneck on high-SNR channels, multiple bits can be mapped
 * to a single signal value by using more than two levels.
 * For example, two bits can be mapped as {-3, -1, 1, 3}.
 * This is called Amplitude Shift Keying.
 * Output of this ASK modulator will be normalized to an average signal power
 * of E[x²] = 1.
 * Input to the ASK demodulator is also expected to have a power of 1. If that
 * is not the case, symbol mapping will fail and therefor introduce bit errors
 * even for noise-free input.
 *
 * Edit: Normalization can now be turned off, for use in QAM.
 */
class Ask : public Modem {
	Bpsk *mBpsk;
	unsigned mBitsPerSymbol;
	float mPowerNormalizer, mNormalMagnitude;
	
public:
	Ask();
	/*!
	 * \brief Construct an ASK-(de)modulator with the given amount of bits per symbol.
	 * \param bitsPerSymbol Number of bits per symbol.
	 * \param normalizeOutput Optional: Set to false, to disable power normalization.
	 */
	Ask(unsigned bitsPerSymbol, bool normalizeOutput = true);
	~Ask();
	
	/*!
	 * \brief Set the new bits-per-symbol value.
	 */
	void setBitsPerSymbol(unsigned bps, bool normalizeOutput = true);

	/*!
	 * \brief Get the current bits-per-symbol value.
	 */
	unsigned bitsPerSymbol();
	
	void modulate();
	void demodulate();
};


}//namespace Modulation
}//namespace SignalProcessing

#endif //PCDSP_MODULATION_ASK

 
