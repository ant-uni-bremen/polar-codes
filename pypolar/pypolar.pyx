cimport polar_interface
import numpy as np
import time
cimport numpy as np
from libcpp cimport bool


cdef class PolarEncoder:
    cdef polar_interface.Encoder* kernel

    def __cinit__(self, block_size, np.ndarray frozen_bit_positions, encoder_impl='Packed'):
        frozen_bit_positions = np.sort(frozen_bit_positions)
        if encoder_impl is 'Unpacked':
            self.kernel = new polar_interface.ButterflyAvx2Char(block_size, frozen_bit_positions)
        else:
            self.kernel = new polar_interface.ButterflyAvx2Packed(block_size, frozen_bit_positions)

    def __del__(self):
        del self.kernel

    def encode(self):
        self.kernel.encode()

    def setInformation(self, np.ndarray[np.uint8_t, ndim=1] info_bytes):
        self.kernel.setInformation(<void*> info_bytes.data)

    def blockLength(self):
        return self.kernel.blockLength()

    def setSystematic(self, flag):
        self.kernel.setSystematic(flag)

    def isSystematic(self):
        return self.kernel.isSystematic()

    def getEncodedData(self):
        cdef np.ndarray[np.uint8_t, ndim=1] codeword = np.zeros((self.kernel.blockLength() // 8, ), dtype=np.uint8)
        self.kernel.getEncodedData(<void*> codeword.data)
        return codeword

    def encode_vector(self, np.ndarray[np.uint8_t, ndim=1] info_bytes):
        self.kernel.setInformation(<void*> info_bytes.data)
        self.kernel.encode()
        return self.getEncodedData()


cdef class PolarDecoder:
    cdef polar_interface.Decoder* kernel

    def __cinit__(self, block_size, np.ndarray frozen_bit_positions):
        frozen_bit_positions = np.sort(frozen_bit_positions)
        #self.kernel = polar_interface.Decoder.make(block_size, 1, frozen_bit_positions)

    def __del__(self):
        del self.kernel

    def decode(self):
        self.kernel.decode()

    def blockLength(self):
        return self.kernel.blockLength()

    def decode_vector(self, np.ndarray[np.float32_t, ndim=1] llrs):
        self.kernel.setSignal(<float*> llrs.data)
        #self.kernel.decode()
        cdef np.ndarray[np.uint8_t, ndim=1] infoword = np.zeros((self.kernel.blockLength() // 8, ), dtype=np.uint8)
        return infoword

