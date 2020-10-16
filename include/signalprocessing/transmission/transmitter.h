/* -*- c++ -*- */
/*
 * Copyright 2018 Florian Lotze
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#ifndef PCDSP_TRANSMITTER_H
#define PCDSP_TRANSMITTER_H

#include <vector>

namespace SignalProcessing {
namespace Transmission {

/*!
 * \brief A skeleton class for any kind of transmission models, such as AWGN-
 * channels, Rayleigh-fading channels and the like.
 */
class Transmitter
{
protected:
    std::vector<float>* mSignal; ///< Location of a signal to be processed.

public:
    Transmitter();
    virtual ~Transmitter();

    /*!
     * \brief Set the location of a signal vector for transmission.
     */
    void setSignal(std::vector<float>*);

    /*!
     * \brief Call the transmission routine of a Transmitter object.
     */
    virtual void transmit() = 0;
};

} // namespace Transmission
} // namespace SignalProcessing

#endif // PCDSP_TRANSMITTER_H
