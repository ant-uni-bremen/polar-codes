/* -*- c++ -*- */
/*
 * Copyright 2018 Florian Lotze
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#ifndef PC_ENC_ENCODER_H
#define PC_ENC_ENCODER_H

#include <cstddef>
#include <vector>

#include <polarcode/bitcontainer.h>
#include <polarcode/errordetection/errordetector.h>

namespace PolarCode {
namespace Encoding {

/*!
 * \brief The skeleton-class for encoders
 */
class Encoder
{
private:
    size_t mEncoderDuration;

protected:
    ErrorDetection::Detector* mErrorDetector; ///< Error detecting object
    size_t mBlockLength;                      ///< Block length of the Polar Code
    bool mSystematic;                         ///< Whether to use systematic coding
    bool mCodewordReady; ///< If true, the data has been copied into mBitContainer before
                         ///< encoding
    unsigned char* xmInputData;        ///< Pointer to memory location of bits to encode
    BitContainer* mBitContainer;       ///< Internal bit memory
    std::vector<unsigned> mFrozenBits; ///< Indices for frozen bits

    size_t informationByteSize()
    {
        const size_t infoBitSize = mBlockLength - mFrozenBits.size();
        return (infoBitSize % 8) ? infoBitSize / 8 : (infoBitSize + 8) / 8;
    }

public:
    Encoder();
    virtual ~Encoder();
    virtual void encode() = 0; ///< Execute the encoding algorithm.

    /*!
     * \brief Encode packed vector
     */
    void encode_vector(void* pInfo, void* pCode);

    /*!
     * \brief Encoder duration
     * \return Number of ticks in nanoseconds for last encoder call.
     */
    size_t duration_ns() { return mEncoderDuration; }

    /*!
     * \brief Set the encoder's parameters.
     * \param blockLength Number of code bits.
     * \param frozenBits A set of frozen channel indices.
     */
    virtual void initialize(size_t blockLength,
                            const std::vector<unsigned>& frozenBits) = 0;

    /*!
     * \brief Query infoword block Length
     */
    size_t infoLength() { return blockLength() - mFrozenBits.size(); }
    /*!
     * \brief Query codeword block Length
     */
    size_t blockLength();

    /*!
     * \brief Query frozenBits
     */
    std::vector<unsigned> frozenBits() { return mFrozenBits; }

    /*!
     * \brief Set an error detection scheme.
     * \param pDetector Pointer to an error detecting object.
     */
    void setErrorDetection(ErrorDetection::Detector* pDetector);

    /*!
     * \brief Get error detection mode
     * \return String indicating error detection mode and length.
     */
    std::string getErrorDetectionMode()
    {
        return std::string(mErrorDetector->getType() + "-" +
                           std::to_string(mErrorDetector->getCheckBitCount()));
    }

    /*!
     * \brief Explicitly call setSystematic(false); to use
     *        non-systematic coding.
     *
     * This function serves the purpose of explicitly deactivating the
     * default systematic coding setting via setSystematic(false),
     * or re-activating it lateron.
     * \param sys Whether coding should be systematic.
     */
    void setSystematic(bool sys);

    /*!
     * \brief Query if Encoder produces systematic codeword.
     *
     * Check if the code is systematic.
     */
    bool isSystematic();

    /*!
     * \brief Copies packed information bits into encoder's memory.
     * \param pData Pointer to memory location of information bytes.
     */
    void setInformation(void* pData);

    /*!
     * \brief Copy packed information bits from encoder.
     * \param pData Pointer to memory location for information bits.
     */
    void getInformation(void* pData);

    /*!
     * \brief Insert packed bits.
     * \param pData Pointer to memory location of packed bits.
     */
    void setCodeword(void* pData);

    /*!
     * \brief Insert char bits.
     * \param cData Pointer to memory location of char bits.
     */
    void setCharCodeword(void* cData);

    /*!
     * \brief Insert float bits.
     * \param fData Pointer to memory location of float bits.
     */
    void setFloatCodeword(void* fData);

    /*!
     * \brief Writes packed code bits into memory.
     * \param pData Memory location for encoded data. Must be pre-allocated.
     */
    void getEncodedData(void* pData);

    /*!
     * \brief Set all frozen bits to 0. This is needed by the recursive encoder.
     */
    void clearFrozenBits();
};

class UndefinedEncoder : public Encoder
{
public:
    UndefinedEncoder();
    ~UndefinedEncoder();
    void initialize(size_t, const std::vector<unsigned>&);
    void encode();
};

} // namespace Encoding
} // namespace PolarCode

#endif // PC_ENC_ENCODER_H
