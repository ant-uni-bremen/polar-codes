/* -*- c++ -*- */
/*
 * Copyright 2018 Florian Lotze
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#ifndef PCDSP_TRANSMITTER_RAYLEIGH_H
#define PCDSP_TRANSMITTER_RAYLEIGH_H

#include <signalprocessing/transmission/transmitter.h>
#include <signalprocessing/random.h>

namespace SignalProcessing {
namespace Transmission {

/*!
 * \brief This class implements the behaviour of a noisy Rayleigh fading channel.
 */
class Rayleigh : public Transmitter {
	Random::Generator *mRandGen;

	float mEsNoLog, mEsNoLin, mNoiseMagnitude;

	void transmit_simple();
	void transmit_vectorized();
public:
	Rayleigh();
	/*!
	 * \brief Construct an Awgn channel with given signal-to-noise ratio per symbol.
	 * \param EsN0_dB Signal-to-noise ratio (symbol energy per noise energy) in dB.
	 */
	Rayleigh(float EsN0_dB);
	~Rayleigh();

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

	void transmit();

};

}//namespace Transmission
}//namespace SignalProcessing

#endif //PCDSP_TRANSMITTER_RAYLEIGH_H
