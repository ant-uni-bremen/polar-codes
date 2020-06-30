/* -*- c++ -*- */
/*
 * Copyright 2018 Florian Lotze
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#ifndef PCDSP_TRANSMITTER_SCALE_H
#define PCDSP_TRANSMITTER_SCALE_H

#include <signalprocessing/transmission/transmitter.h>
#include <signalprocessing/random.h>

namespace SignalProcessing {
namespace Transmission {

/*!
 * \brief This transmitter multiplies the signal by a given constant.
 */
class Scale : public Transmitter {
	float mFactor;

	void transmit_simple();
	void transmit_vectorized();
public:
	Scale();
	/*!
	 * \brief Construct a scaling channel with given factor.
	 * \param factor The value to multiply a signal with.
	 */
	Scale(float factor);
	~Scale();

	/*!
	 * \brief Set a new scaling factor.
	 */
	void setFactor(float);

	/*!
	 * \brief Get the current factor.
	 * \return The current factor.
	 */
	float Factor();

	void transmit();

};

}//namespace Transmission
}//namespace SignalProcessing

#endif //PCDSP_TRANSMITTER_SCALE_H
