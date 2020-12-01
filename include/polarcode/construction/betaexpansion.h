/* -*- c++ -*- */
/*
 * Copyright 2020 Johannes Demel
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#ifndef PC_CONSTRUCTION_BETAEXPANSION_H
#define PC_CONSTRUCTION_BETAEXPANSION_H

// #include <polarcode/arrayfuncs.h>
#include <polarcode/construction/constructor.h>
#include <cmath>
#include <vector>

namespace PolarCode {
namespace Construction {

/*!
 * \brief Code Construction via Beta Expansion
 *
 * This class implements Polar Code Construction via universal partial order properties.
 *
 * He et al. "beta-expansion: A theoretical framework for fast and recursive construction
 * of polar codes" DOI: 10.1109/TSP.2014.2371781
 * https://doi.org/10.1109/GLOCOM.2017.8254146
 */
class BetaExpansion : public Constructor
{
private:
    std::vector<double> mChannelParameters;

    void calculateChannelParameters();

    const double m_beta_value = std::pow(2.0, 1.0 / 4.0);

public:
    BetaExpansion();

    /*!
     * \brief Create the constructor and initialize the length parameters.
     * \param N Code length.
     * \param K Information length.
     */
    BetaExpansion(size_t N, size_t K);

    /*!
     * \brief Create the constructor and initialize all parameters.
     * \param N Code length.
     * \param K Information length.
     * \param designSnr Unused! only here for compatibility reasons!
     */
    BetaExpansion(size_t N, size_t K, float designSnr) : BetaExpansion(N, K){};
    ~BetaExpansion(){};

    /*!
     * \brief Executes the construction algorithm.
     * \return The set of frozen bits.
     */
    std::vector<unsigned> construct();

    /*!
     * \brief Set the Bhattacharrya parameter of the desired channel.
     * \param newInitialParameter The initial parameter.
     */
    void setInitialParameter(float newInitialParameter){};

    /*!
     * \brief Set the Bhattacharrya parameter depending on a design-SNR.
     * \param designSNR The SNR of the channel the code will be designed for.
     */
    void setParameterByDesignSNR(float designSNR){};
};


} // namespace Construction
} // namespace PolarCode

#endif // PC_CONSTRUCTION_BETAEXPANSION_H
