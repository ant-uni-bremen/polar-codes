from libcpp.vector cimport vector
from libcpp cimport bool

cdef extern from "polarcode/encoding/butterfly_avx2_packed.h" namespace "PolarCode::Encoding":
    cdef cppclass ButterflyAvx2Packed:
        ButterflyAvx2Packed(size_t, vector[unsigned int]) except +
        void encode()
        void setInformation(void*)
        void getEncodedData(void*)
        size_t blockLength()
        void setSystematic(bool)
        bool isSystematic()


cdef extern from "polarcode/encoding/butterfly_avx2_char.h" namespace "PolarCode::Encoding":
    cdef cppclass ButterflyAvx2Char:
        ButterflyAvx2Char(size_t, vector[unsigned int]) except +
        void encode()
        void setInformation(void*)
        void getEncodedData(void*)
        size_t blockLength()
        void setSystematic(bool)
        bool isSystematic()