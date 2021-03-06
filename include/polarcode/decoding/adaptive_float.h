/* -*- c++ -*- */
/*
 * Copyright 2018 Florian Lotze
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#ifndef PC_DEC_ADPT_FLOAT_H
#define PC_DEC_ADPT_FLOAT_H

#include <polarcode/decoding/fastssc_avx_float.h>
#include <polarcode/decoding/scl_avx_float.h>
#include <memory>

namespace PolarCode {
namespace Decoding {

/*!
 * \brief A wrapper class for Fast-SSC and SCL decoding, in case the former fails.
 *
 * The list decoder has a high latency, but achieves up to around 2 dB
 * Eb/N0 more than the fast decoder. As the Fast-SSC decoder indeed is fast,
 * it tries to decode the given signal and only upon failure, the signal is
 * decoded a second time, using the list decoder.
 */
class AdaptiveFloat : public Decoder
{
    std::unique_ptr<FastSscAvxFloat> mFastDecoder;
    std::unique_ptr<SclAvxFloat> mListDecoder;
    size_t mListSize;

public:
    /*!
     * \brief Create an adaptive decoder.
     * \param blockLength Block length of the Polar Code.
     * \param listSize Path limit of the list decoder.
     * \param frozenBits The set of frozen bits.
     */
    AdaptiveFloat(size_t blockLength,
                  size_t listSize,
                  const std::vector<unsigned>& frozenBits);
    ~AdaptiveFloat();
    bool decode();

    void setSystematic(bool sys);
    void setErrorDetection(ErrorDetection::Detector* pDetector);
    void setSignal(const float* pLlr);

    /*!
     * \brief Get decoder list size
     * \return size_t with Decoder List size.
     */
    size_t getListSize() { return mListSize; }
};


} // namespace Decoding
} // namespace PolarCode

#endif // PC_DEC_ADPT_FLOAT_H
