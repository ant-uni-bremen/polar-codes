/* -*- c++ -*- */
/*
 * Copyright 2018 Florian Lotze
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#ifndef PC_CON_CONSTRUCTOR_H
#define PC_CON_CONSTRUCTOR_H

#include <cstddef>
#include <string>
#include <vector>

namespace PolarCode {
namespace Construction {

/**
 * \brief The Constructor class
 */
class Constructor
{
protected:
    size_t mBlockLength;       ///< Number of code bits.
    size_t mInformationLength; ///< Number of information bits.
    float mDesignSnr;          ///< Construction parameter for most algorithms.

public:
    Constructor();
    virtual ~Constructor();

    /*!
     * \brief Executes the construction algorithm.
     * \return The set of frozen bits.
     */
    virtual std::vector<unsigned> construct() = 0;

    /*!
     * \brief Set the block length of the code to construct.
     * \param newBlockLength The desired code word length.
     */
    void setBlockLength(size_t newBlockLength);

    /*!
     * \brief Set the number of information bits in the code to construct.
     * \param newInformationLength The desired amount of information bits.
     */
    void setInformationLength(size_t newInformationLength);

    /*!
     * \brief Set the design parameter by passing the design SNR in dB.
     * \param designSnr The SNR (Eb/No) the code should be designed for.
     */
    void setDesignSnr(float designSnr);
};

std::vector<unsigned>
frozen_bits(const int blockLength,
            const int infoLength,
            const float designSNR,
            const std::string& constructor_type = std::string("BB"));

} // namespace Construction
} // namespace PolarCode

#endif // PC_CON_CONSTRUCTOR_H
