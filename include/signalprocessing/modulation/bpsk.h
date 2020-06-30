/* -*- c++ -*- */
/*
 * Copyright 2018 Florian Lotze
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#ifndef PCDSP_MODULATION_BPSK
#define PCDSP_MODULATION_BPSK

#include <signalprocessing/modulation/modem.h>

namespace SignalProcessing {
namespace Modulation {

/*!
 * \brief The very simple Binary Phase Shift Keying modulator and demodulator.
 *
 * BPSK maps a zero bit to a signal value of 1 and a one bit to a signal value
 * of -1. The modulator simply applies this mapping to input data.
 * The demodulator detects the sign of input symbols and thereby unmaps signal
 * values back to data bits.
 */
class Bpsk : public Modem {
	void modulate_vectorized();
	void modulate_simple();

	void demodulate_vectorized();
	void demodulate_simple();

public:
	Bpsk();
	~Bpsk();

	void modulate();
	void demodulate();
};


}//namespace Modulation
}//namespace SignalProcessing

#endif //PCDSP_MODULATION_BPSK
