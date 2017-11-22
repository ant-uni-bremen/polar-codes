#ifndef PC_AVX2_TEMPLATES
#define PC_AVX2_TEMPLATES
#include <polarcode/avxconvenience.h>

namespace PolarCode {
namespace Decoding {
namespace Template {

/*****************
 * Genral memory management
 *****************/

constexpr size_t nBit2cvecCount(const size_t blockLength) {
    return (blockLength+31)/32;
}

template<unsigned vecCount>
void WriteFixedValue(char value, __m256i *BitsOut) {
    __m256i Output = _mm256_set1_epi8(value);
    for(unsigned int i=0; i<vecCount; ++i) {
        _mm256_store_si256(BitsOut+i, Output);
    }
}

template<unsigned blockLength>
void ZeroPrepareShortVector(__m256i &vec) {
    union {
        __m256i mask;
        char cMask[32];
        short sMask[16];
        int iMask[8];
        long long lMask[4];
        __m128i vMask[2];
    };
    __m256i zero = _mm256_setzero_si256();
    mask = _mm256_setzero_si256();

    if (blockLength == 1) {
        cMask[0] = -1;
    } else if (blockLength == 2) {
        sMask[0] = -1;
    } else if (blockLength == 4) {
        iMask[0] = -1;
    } else if (blockLength == 8) {
        lMask[0] = -1;
    } else {
        vMask[0] = _mm_set1_epi64x(-1);
    }

    vec = _mm256_blendv_epi8(zero, vec, mask);
}

template<unsigned blockLength>
void SpcPrepareShortVector(__m256i &vec) {
    union {
        __m256i mask;
        char cMask[32];
        short sMask[16];
        int iMask[8];
        long long lMask[4];
        __m128i vMask[2];
    };
    __m256i neutral = _mm256_set1_epi8(127);
    mask = _mm256_setzero_si256();

    if (blockLength == 1) {
        cMask[0] = -1;
    } else if (blockLength == 2) {
        sMask[0] = -1;
    } else if (blockLength == 4) {
        iMask[0] = -1;
    } else if (blockLength == 8) {
        lMask[0] = -1;
    } else {
        vMask[0] = _mm_set1_epi64x(-1);
    }

    vec = _mm256_blendv_epi8(neutral, vec, mask);
}

template<unsigned shift>
__m256i subVectorBackShiftBytes(__m256i x) {
    switch (shift) {
    case 1:
        return _mm256_slli_epi16(x, 8);
    case 2:
        return _mm256_slli_epi32(x, 16);
    case 4:
        return _mm256_slli_epi64(x, 32);
    case 8:
        return _mm256_slli_si256(x, 8);
    case 16:
        return _mm256_permute2x128_si256(x, _mm256_setzero_si256(), 0b01001110);
    }
    return _mm256_setzero_si256();
}

/*****************
 * Polar Transformations
 *****************/



void F_function_calc(__m256i &Left, __m256i &Right, __m256i *Out)
{
    static const __m256i absCorrector = _mm256_set1_epi8(-127);
    static const __m256i one = _mm256_set1_epi8(1);
    __m256i absL = _mm256_abs_epi8(_mm256_max_epi8(Left, absCorrector));
    __m256i absR = _mm256_abs_epi8(_mm256_max_epi8(Right, absCorrector));
    __m256i minV = _mm256_min_epi8(absL, absR);//minimum of absolute values
    __m256i xorV = _mm256_xor_si256(Left, Right);//multiply signs
    xorV = _mm256_or_si256(xorV, one);//prevent zero as sign value
    __m256i outV = _mm256_sign_epi8(minV, xorV);//merge sign and value
    //outV = _mm256_max_epi8(outV, absCorrector);
    _mm256_store_si256(Out, outV);//save
}

void G_function_calc(__m256i &Left, __m256i &Right, __m256i &Bits, __m256i *Out)
{
    __m256i sum  = _mm256_adds_epi8(Right, Left);
    __m256i diff = _mm256_subs_epi8(Right, Left);
    __m256i result = _mm256_blendv_epi8(sum, diff, Bits);
    _mm256_store_si256(Out, result);
}

template<unsigned blockLength>
void F_function(__m256i *LLRin, __m256i *LLRout) {
    __m256i Left, Right;
    if (blockLength < 32) {
        Left = _mm256_load_si256(LLRin);
        Right = _mm256_subVectorShiftBytes_epu8(Left, blockLength);
        F_function_calc(Left, Right, LLRout);
    } else {
        constexpr unsigned vecCount = nBit2cvecCount(blockLength);
        for(unsigned i=0; i<vecCount; i++) {
            Left = _mm256_load_si256(LLRin+i);
            Right = _mm256_load_si256(LLRin+i+vecCount);
            F_function_calc(Left, Right, LLRout+i);
        }
    }
}

template<unsigned blockLength>
void G_function(__m256i *LLRin, __m256i *LLRout, __m256i *BitsIn) {
    __m256i Left, Right, Bits;
    if (blockLength < 32) {
        Left = _mm256_load_si256(LLRin);
        Right = _mm256_subVectorShiftBytes_epu8(Left, blockLength);
        Bits = _mm256_load_si256(BitsIn);
        G_function_calc(Left, Right, Bits, LLRout);
    } else {
        constexpr unsigned vecCount = nBit2cvecCount(blockLength);
        for(unsigned i=0; i<vecCount; i++) {
            Left = _mm256_load_si256(LLRin+i);
            Right = _mm256_load_si256(LLRin+i+vecCount);
            Bits = _mm256_load_si256(BitsIn+i);
            G_function_calc(Left, Right, Bits, LLRout+i);
        }
    }
}

template<unsigned blockLength>
void G_function_0R(__m256i *LLRin, __m256i *LLRout) {
    if (blockLength < 32) {
        __m256i Left, Right, Sum;
        Left = _mm256_load_si256(LLRin);
        Right = _mm256_subVectorShiftBytes_epu8(Left, blockLength);
        Sum = _mm256_adds_epi8(Left, Right);
        _mm256_store_si256(LLRout, Sum);
    } else {
        constexpr unsigned vecCount = nBit2cvecCount(blockLength);
        __m256i Left, Right, Sum;
        for(unsigned i=0; i<vecCount; i++) {
            Left = _mm256_load_si256(LLRin+i);
            Right = _mm256_load_si256(LLRin+i+vecCount);
            Sum = _mm256_adds_epi8(Left, Right);
            _mm256_store_si256(LLRout+i, Sum);
        }
    }
}

template<unsigned blockLength>
void CombineSoftInPlace(__m256i *Bits) {
    constexpr unsigned vecCount = nBit2cvecCount(blockLength);
    for(unsigned i=0; i<vecCount; i++) {
        __m256i tempL = _mm256_load_si256(Bits+i);
        __m256i tempR = _mm256_load_si256(Bits+vecCount+i);
        F_function_calc(tempL, tempR, Bits+i);
    }
}

template<unsigned blockLength>
void CombineSoftBits(__m256i *Left, __m256i *Right, __m256i *Out) {
    __m256i LeftV;
    __m256i RightV;

    if (blockLength < 32) {
        static const __m256i absCorrector = _mm256_set1_epi8(-127);

        __m256i LeftV = _mm256_loadu_si256(Left);
        __m256i RightV = _mm256_loadu_si256(Right);
        __m256i OutV;

        ZeroPrepareShortVector<blockLength>(LeftV);
        ZeroPrepareShortVector<blockLength>(RightV);

        LeftV = _mm256_max_epi8(LeftV, absCorrector);
        RightV = _mm256_max_epi8(RightV, absCorrector);

        //Boxplus operation for upper bits
        F_function_calc(LeftV, RightV, &OutV);

        // Copy operation for lower bits
        OutV = _mm256_or_si256(OutV, subVectorBackShiftBytes<blockLength>(RightV));
        _mm256_store_si256(Out, OutV);
     } else {
        const unsigned vecCount = nBit2cvecCount(blockLength);
        for(unsigned i=0; i<vecCount; ++i) {
            LeftV = _mm256_load_si256(Left+i);
            RightV = _mm256_load_si256(Right+i);

            //Boxplus for upper bits
            F_function_calc(LeftV, RightV, Out+i);

            //Copy lower bits
            _mm256_store_si256(Out+vecCount+i, RightV);
        }
    }
}

template<unsigned blockLength>
void Combine_0R(__m256i *Bits) {
    char* BitPtr = reinterpret_cast<char*>(Bits);
    memcpy(BitPtr, BitPtr+blockLength, blockLength);
}

template<unsigned blockLength>
void Combine_0RShort(__m256i *Bits, __m256i *RightBits) {
    char* BitPtr = reinterpret_cast<char*>(Bits);
    memcpy(BitPtr,             RightBits, blockLength);
    memcpy(BitPtr+blockLength, RightBits, blockLength);
}



/*****************
 * Decoders
 *****************/


template<unsigned blockLength>
void RateZeroDecode(__m256i *LlrIn, __m256i *BitsOut) {
    (void)LlrIn;
    WriteFixedValue<(blockLength+31)/32>(127, BitsOut);
}

template<unsigned blockLength>
void RateOneDecode(__m256i *LlrIn, __m256i *BitsOut) {
    constexpr unsigned vecCount = (blockLength+31)/32;

    for(unsigned i=0; i<vecCount; ++i) {
        _mm256_store_si256(BitsOut+i, _mm256_load_si256(LlrIn+i));
    }
}


template<unsigned blockLength>
void RepetitionDecode(__m256i *LlrIn, __m256i *BitsOut) {
    constexpr unsigned vecCount = (blockLength+31)/32;
    char Bits;

    if (blockLength <= 32) {
        ZeroPrepareShortVector<blockLength>(LlrIn);

        // Get simple sum
        Bits = reduce_adds_epi8(_mm256_load_si256(LlrIn));
    } else {
        __m256i LlrSum = _mm256_setzero_si256();

        // Accumulate vectors
        for(unsigned i=0; i<vecCount; i++) {
            LlrSum = _mm256_adds_epi8(LlrSum, _mm256_load_si256(LlrIn+i));
        }

        // Get final sum and save decoding result
        Bits = reduce_adds_epi8(LlrSum);
    }
    WriteFixedValue<vecCount>(Bits, BitsOut);
}

template<unsigned blockLength>
void SpcDecode(__m256i *LlrIn, __m256i *BitsOut) {
    constexpr unsigned vecCount = (blockLength+31)/32;

    if (vecCount > 1) {
        __m256i parVec = _mm256_setzero_si256();
        unsigned minIdx = 0;
        char testAbs, minAbs = 127;

        // Compute parity and save uncorrected output data
        for(unsigned i=0; i<vecCount; i++) {
            __m256i vecIn = _mm256_load_si256(LlrIn+i);
            parVec = _mm256_xor_si256(parVec, vecIn);
            _mm256_store_si256(BitsOut+i, vecIn);
        }

        unsigned char parity = reduce_xor_si256(parVec) & 0x80;

        // If there was an error, try to correct it
        if(parity) {
            for(unsigned i=0; i<vecCount; i++) {
                __m256i vecIn = _mm256_load_si256(LlrIn+i);

                __m256i abs = _mm256_abs_epi8(vecIn);
                unsigned vecMin = _mm256_minpos_epu8(abs, &testAbs);
                if(testAbs < minAbs) {
                    minIdx = vecMin+i;
                    minAbs = testAbs;
                    if(minAbs == 0) break;
                }
            }

            // Flip least reliable bit
            unsigned char *BitPtr = reinterpret_cast<unsigned char*>(BitsOut);
            BitPtr[minIdx] = -BitPtr[minIdx];
        }
    } else {
        SpcPrepareShortVector<blockLength>(LlrIn);

        __m256i vecIn = _mm256_load_si256(LlrIn);
        _mm256_store_si256(BitsOut, vecIn);

        // Flip least reliable bit, if neccessary
        if(reduce_xor_si256(vecIn) & 0x80) {
            __m256i abs = _mm256_abs_epi8(vecIn);
            unsigned vecMin = _mm256_minpos_epu8(abs);
            unsigned char *BitPtr = reinterpret_cast<unsigned char*>(BitsOut);
            BitPtr[vecMin] = -BitPtr[vecMin];
        }
    }
}

template<unsigned blockLength>
void ZeroSpcDecode(__m256i *LlrIn, __m256i *BitsOut) {
    unsigned char* LlrPtr = reinterpret_cast<unsigned char*>(LlrIn);
    unsigned char* BitPtr = reinterpret_cast<unsigned char*>(BitsOut);
    constexpr size_t subBlockLength = blockLength/2;
    constexpr unsigned vecCount = (subBlockLength+31)/32;
    unsigned minIdx = 0;
    unsigned char parity, testAbs;

    //Generate output
    memcpy(BitPtr,                LlrPtr+subBlockLength, subBlockLength);
    memcpy(BitPtr+subBlockLength, LlrPtr+subBlockLength, subBlockLength);

    if (blockLength >= 32) {
        __m256i parVec = _mm256_setzero_si256();
        char minAbs = 127;

        //Check parity equation
        for(unsigned i=0; i<vecCount; i++) {
            //G-function with only frozen bits
            __m256i left = _mm256_load_si256(LlrIn+i);
            __m256i right = _mm256_load_si256(LlrIn+vecCount+i);
            __m256i llr = _mm256_adds_epi8(left, right);

            //Update parity counter
            parVec = _mm256_xor_si256(parVec, llr);

            // Only search for minimum if there is a chance for smaller absolute value
            if(minAbs > 0) {
                __m256i abs = _mm256_abs_epi8(llr);
                unsigned vecMin = _mm256_minpos_epu8(abs, &testAbs);
                if(testAbs < minAbs) {
                    minIdx = vecMin+i;
                    minAbs = testAbs;
                }
            }
        }
        parity = reduce_xor_si256(parVec) & 0x80;
    } else {
        __m256i left = _mm256_load_si256(LlrIn);
        __m256i right = _mm256_subVectorShiftBytes_epu8(left, subBlockLength);
        __m256i llr = _mm256_adds_epi8(left, right);

        //Set unused bits to SPC-neutral value of 127
        SpcPrepareShortVector<subBlockLength>(llr);

        // Flip least reliable bit, if neccessary
        __m256i abs = _mm256_abs_epi8(llr);
        minIdx = _mm256_minpos_epu8(abs, &testAbs);
        parity = reduce_xor_si256(llr) & 0x80;
    }

    // Flip least reliable bit, if neccessary
    BitPtr[minIdx] = -BitPtr[minIdx];
    BitPtr[minIdx+subBlockLength] = -BitPtr[minIdx+subBlockLength];
}

template<unsigned blockLength>
void ZeroOneDecodeShort(__m256i *LlrIn, __m256i *BitsOut) {
    constexpr size_t subBlockLength = blockLength/2;
    __m256i subLlrLeft, subLlrRight;

    G_function_0R<subBlockLength>(LlrIn, &subLlrLeft);

    subLlrRight = subVectorBackShiftBytes<blockLength>(subLlrLeft);
    ZeroPrepareShortVector<subBlockLength>(subLlrLeft);
    _mm256_store_si256(BitsOut, _mm256_or_si256(subLlrLeft, subLlrRight));
}

template<unsigned blockLength>
void simplifiedRightRateOneDecode(__m256i *LlrIn, __m256i *BitsOut) {
    if (blockLength < 32) {
        __m256i Bits = _mm256_load_si256(BitsOut);//Load left bits
        __m256i Llr_r_subcode = _mm256_setzero_si256();//Destination for right subcode

        G_function<blockLength>(LlrIn, &Llr_r_subcode, BitsOut);//Get right child LLRs
        /*nop*/ //Rate 1 decoder
        __m256i Bits_r = subVectorBackShiftBytes<blockLength>(Llr_r_subcode);
        __m256i Bits_o;
        F_function_calc(Bits, Llr_r_subcode, &Bits_o);//Combine left bits
        ZeroPrepareShortVector<blockLength>(Bits_o);//Clear right bits
        Bits = _mm256_or_si256(Bits_o, Bits_r);//Merge bits into single vector
        _mm256_store_si256(BitsOut, Bits);//Save
    } else {
        constexpr size_t vecCount = nBit2cvecCount(blockLength);
        for(unsigned i = 0; i < vecCount; ++i) {
            __m256i Llr_l = _mm256_load_si256(LlrIn+i);
            __m256i Llr_r = _mm256_load_si256(LlrIn+i+vecCount);
            __m256i Bits = _mm256_load_si256(BitsOut+i);
            __m256i Llr_o;

            G_function_calc(Llr_l, Llr_r, Bits, &Llr_o);
            /*nop*/ //Rate 1 decoder
            F_function_calc(Bits, Llr_o, BitsOut+i);//Combine left bit
            _mm256_store_si256(BitsOut+i+vecCount, Llr_o);//Copy right bit
        }
    }
}

}// namespace Template
}// namespace Decoding
}// namespace PolarCode

#endif
