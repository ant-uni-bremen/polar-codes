from libcpp.vector cimport vector
from libcpp cimport bool


cdef extern from "polarcode/encoding/encoder.h" namespace "PolarCode::Encoding":
    cdef cppclass Encoder:
        Encoder() except +
        void encode()
        void encode_vector(void*, void*)
        void setInformation(void*)
        void getEncodedData(void*)
        size_t blockLength()
        void setErrorDetection(Detector*)
        void setSystematic(bool)
        bool isSystematic()
        vector[unsigned int] frozenBits()
        size_t duration_ns()


cdef extern from "polarcode/encoding/butterfly_fip_packed.h" namespace "PolarCode::Encoding":
    cdef cppclass ButterflyFipPacked(Encoder):
        ButterflyFipPacked(size_t, vector[unsigned int]) except +


cdef extern from "polarcode/decoding/decoder.h" namespace "PolarCode::Decoding":
    cdef cppclass Decoder:
        Decoder() except +
        bool decode()
        bool decode_vector(const float *pLlr, void* pData)
        bool decode_vector(const char *pLlr, void* pData)
        void setErrorDetection(Detector*)
        size_t blockLength()
        size_t infoLength()
        size_t duration_ns()
        void setSignal(const float*)
        void setSignal(const char*)
        void getDecodedInformationBits(void*)
        vector[unsigned int] frozenBits()
        void getSoftCodeword(void *pData)
        void getSoftInformation(void *pData)
        void setSystematic(bool sys)
        bool isSystematic()

    Decoder* makeDecoder(size_t, size_t, vector[unsigned int], int decoder_impl)

cdef extern from "polarcode/errordetection/errordetector.h" namespace "PolarCode::ErrorDetection":
    cdef cppclass Detector:
        Detector() except +

cdef extern from "polarcode/errordetection/crc8.h" namespace "PolarCode::ErrorDetection":
    cdef cppclass CRC8(Detector):
        CRC8() except +

cdef extern from "polarcode/errordetection/crc32.h" namespace "PolarCode::ErrorDetection":
    cdef cppclass CRC32(Detector):
        CRC32() except +

cdef extern from "polarcode/errordetection/dummy.h" namespace "PolarCode::ErrorDetection":
    cdef cppclass Dummy(Detector):
        Dummy() except +

cdef extern from "polarcode/construction/constructor.h" namespace "PolarCode::Construction":
    vector[unsigned int] frozen_bits(const int blockLength, const int infoLength, const float designSNR)

