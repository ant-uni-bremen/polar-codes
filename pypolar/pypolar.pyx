cimport polar_interface
import numpy as np
import time
cimport numpy as np
from libcpp cimport bool
from libcpp.vector cimport vector


cdef uint_cppvector_to_ndarray(vector[unsigned int] v):
    cdef unsigned int *vp = &v[0]
    cdef unsigned int[::1] vv = <unsigned int[:v.size()]>vp
    cdef nv = np.asarray(vv)
    cdef res = np.copy(nv)
    return res



def frozen_bits(blockLength, infoLength, designSNR):
    vf = polar_interface.frozen_bits(blockLength, infoLength, designSNR)
    return uint_cppvector_to_ndarray(vf)


cdef class PolarEncoder:
    cdef polar_interface.Encoder* kernel

    def __cinit__(self, block_size, np.ndarray frozen_bit_positions):
        frozen_bit_positions = np.sort(frozen_bit_positions)
        frozen_bit_positions = frozen_bit_positions.astype(np.uint32)
        self.kernel = new polar_interface.ButterflyFipPacked(block_size, frozen_bit_positions)

    def __del__(self):
        del self.kernel

    def setErrorDetection(self):
        detector = new polar_interface.CRC8()
        self.kernel.setErrorDetection(detector)

    def encode(self):
        self.kernel.encode()

    def setInformation(self, np.ndarray[np.uint8_t, ndim=1] info_bytes):
        self.kernel.setInformation(<void*> info_bytes.data)

    def blockLength(self):
        return self.kernel.blockLength()

    def frozenBits(self):
        v = self.kernel.frozenBits()
        return uint_cppvector_to_ndarray(v)

    def setSystematic(self, flag):
        self.kernel.setSystematic(flag)

    def isSystematic(self):
        return self.kernel.isSystematic()

    def getEncodedData(self):
        cdef np.ndarray[np.uint8_t, ndim=1] codeword = np.zeros((self.kernel.blockLength() // 8, ), dtype=np.uint8)
        self.kernel.getEncodedData(<void*> codeword.data)
        return codeword

    def encoder_duration(self):
        return self.kernel.duration_ns()

    def encode_vector(self, np.ndarray[np.uint8_t, ndim=1] info_bytes):
        cdef np.ndarray[np.uint8_t, ndim=1] codeword = np.zeros((self.kernel.blockLength() // 8, ), dtype=np.uint8)
        self.kernel.encode_vector(<void*> info_bytes.data, <void*> codeword.data)
        return codeword


cdef class PolarDecoder:
    cdef polar_interface.Decoder* kernel
    cdef int decoder_impl_flag
    cdef int dec_dur

    def __cinit__(self, block_size, list_size, np.ndarray frozen_bit_positions, decoder_impl="char"):
        frozen_bit_positions = np.sort(frozen_bit_positions)
        frozen_bit_positions = frozen_bit_positions.astype(np.uint32)

        self.decoder_impl_flag = 1 if decoder_impl == "float" else 0
        #print(self.decoder_impl_flag)
        self.kernel = polar_interface.makeDecoder(block_size, list_size, frozen_bit_positions, self.decoder_impl_flag)

    def __del__(self):
        del self.kernel

    def setErrorDetection(self):
        detector = new polar_interface.CRC8()
        self.kernel.setErrorDetection(detector)

    def decode(self):
        self.kernel.decode()

    def frozenBits(self):
        v = self.kernel.frozenBits()
        return uint_cppvector_to_ndarray(v)

    def blockLength(self):
        return self.kernel.blockLength()

    def infoLength(self):
        return self.kernel.infoLength()

    def decode_vector(self, np.ndarray[np.float32_t, ndim=1] llrs):
        #print "float decoder "
        cdef np.ndarray[np.uint8_t, ndim=1] infoword = np.zeros((self.kernel.infoLength() // 8, ), dtype=np.uint8)
        self.kernel.decode_vector(<float*> llrs.data, <void*> infoword.data)
        return infoword

    def decode_vector(self, np.ndarray[np.int8_t, ndim=1] llrs):
        cdef np.ndarray[np.uint8_t, ndim=1] infoword = np.zeros((self.kernel.infoLength() // 8, ), dtype=np.uint8)
        self.kernel.decode_vector(<char*> llrs.data, <void*> infoword.data)
        return infoword

    def decoder_duration(self):
        return self.kernel.duration_ns()

    def enableSoftOutput(self, flag):
        self.kernel.enableSoftOutput(flag)

    def hasSoftOutput(self):
        return self.kernel.hasSoftOutput()

    def getSoftCodeword(self):
        cdef np.ndarray[np.float32_t, ndim=1] llrs = np.zeros((self.kernel.blockLength(), ), dtype=np.float32)
        self.kernel.getSoftCodeword(llrs.data)
        return llrs

    def getSoftInformation(self):
        cdef np.ndarray[np.float32_t, ndim=1] llrs = np.zeros((self.kernel.infoLength(), ), dtype=np.float32)
        self.kernel.getSoftInformation(llrs.data)
        return llrs
