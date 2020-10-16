/* -*- c++ -*- */
/*
 * Copyright 2018 Florian Lotze
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#include <polarcode/construction/constructor.h>
#include <polarcode/decoding/decoder.h>
#include <polarcode/encoding/encoder.h>
#include <polarcode/errordetection/errordetector.h>
#include <set>

namespace PolarCode {

/*!
 * \brief Split up a set of frozen bits for subcodes.
 * \param source The set to split.
 * \param subBlockLength Length of the subcodes.
 * \param left Indices of frozen bits in left subcode.
 * \param right Indices of frozen bits in right subcode.
 */
void splitFrozenBits(const std::vector<unsigned>& source,
                     size_t subBlockLength,
                     std::vector<unsigned>& left,
                     std::vector<unsigned>& right);

/**
 * @brief The PolarCoder class ultimately merges all algorithms
 */
class PolarCoder
{
    unsigned mBlockLength;
    unsigned mInformationLength;
    bool mSystematic;
    float mDesignSnr;

    std::vector<unsigned> mFrozenBits;

    Encoding::Encoder* mEncoder;
    Decoding::Decoder* mDecoder;

public:
    /*!
     * \brief Create an object that collects matching encoder and decoder for a specific
     * Polar Code.
     */
    PolarCoder();
    ~PolarCoder();

    void setCode(unsigned N, unsigned K, bool systematic = true);
    void setDesignSnr(float dSnr);
    void setDecoderType(Decoding::DecoderType type);
};


} // namespace PolarCode
