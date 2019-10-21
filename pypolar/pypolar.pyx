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
    if vf.size() < 1:
        return np.array([], dtype=int)
    return uint_cppvector_to_ndarray(vf)


def get_diff_positions(block_length, pos):
    return np.setdiff1d(np.arange(block_length), pos).astype(pos.dtype)


def calculate_parent_block_length(block_length):
    lp2 = 2 ** int(np.log2(block_length))
    if not lp2 == block_length:
        parent_block_length = 2 ** (int(np.log2(block_length)) + 1)
    else:
        parent_block_length = block_length
    return parent_block_length


class pyPuncturer(object):
    '''
    Puncturer class
    Does only implement QUP puncturing!
    '''
    def __init__(self, block_length, frozen_pos, parent_block_length=None):
        if parent_block_length is None:
            parent_block_length = calculate_parent_block_length(block_length)
        self._block_length = block_length
        self._parent_block_length = parent_block_length
        self._punc_len = punc_len = parent_block_length - block_length
        if parent_block_length == block_length:
            self._puncture_positions = None
        else:
            frozen_pos = frozen_pos.astype(np.intp)
            punced_pos = frozen_pos[0:self._punc_len]
            self._puncture_positions = get_diff_positions(
                self._parent_block_length, punced_pos)

    def parent_block_length(self):
        return self._parent_block_length

    def punctured_block_length(self):
        return self._parent_block_length - self._punc_len

    def puncture_positions(self):
        return self._puncture_positions

    def puncture(self, bits):
        if self._puncture_positions is None:
            return bits
        assert bits.size == self._parent_block_length
        return bits[self._puncture_positions]

    def depuncture(self, bits):
        if self._puncture_positions is None:
            return bits
        assert bits.size == self._puncture_positions.size
        r = np.zeros(self._parent_block_length, dtype=bits.dtype)
        r[self._puncture_positions] = bits
        return r


cdef class Puncturer:
    cdef polar_interface.Puncturer* kernel

    def __cinit__(self, block_size, np.ndarray frozen_bit_positions):
        # print('CYTHON POLAR ENCODER')
        frozen_bit_positions = np.sort(frozen_bit_positions)
        frozen_bit_positions = frozen_bit_positions.astype(np.uint32)
        self.kernel = new polar_interface.Puncturer(block_size,
                                                    frozen_bit_positions)

    def __del__(self):
        del self.kernel

    def blockLength(self):
        return self.kernel.blockLength()

    def parentBlockLength(self):
        return self.kernel.parentBlockLength()

    def blockOutputPositions(self):
        v = self.kernel.blockOutputPositions()
        return uint_cppvector_to_ndarray(v)

    def puncturePacked(self, np.ndarray[np.uint8_t, ndim=1] parent_bytes):
        cdef np.ndarray[np.uint8_t, ndim=1] codeword = np.zeros((self.kernel.blockLength() // 8, ),
                                                                dtype=np.uint8)
        self.kernel.puncturePacked(<unsigned char*> codeword.data,
                                   <unsigned char*> parent_bytes.data)
        return codeword

    def puncture(self, np.ndarray parent_bytes not None):
        if parent_bytes.dtype == np.uint8:
            return self.punctureUInt8(parent_bytes)
        elif parent_bytes.dtype == np.float32:
            return self.punctureFloat(parent_bytes)
        elif parent_bytes.dtype == np.float64:
            return self.punctureDouble(parent_bytes)
        else:
            return self.punctureDouble(parent_bytes.astype(np.float64)).astype(parent_bytes.dtype)

    cdef punctureUInt8(self, np.ndarray[np.uint8_t, ndim=1, mode="c"] parent_bytes):
        cdef np.ndarray[np.uint8_t, ndim=1] codeword = np.zeros((self.kernel.blockLength(), ),
                                                                dtype=np.uint8)
        self.kernel.puncture(<unsigned char*> codeword.data,
                             <unsigned char*> parent_bytes.data)
        return codeword

    cdef punctureFloat(self, np.ndarray[np.float32_t, ndim=1, mode="c"] parent_bytes):
        cdef np.ndarray[np.float32_t, ndim=1] codeword = np.zeros((self.kernel.blockLength(), ),
                                                                  dtype=np.float32)
        self.kernel.puncture(<float*> codeword.data,
                             <float*> parent_bytes.data)
        return codeword

    cdef punctureDouble(self, np.ndarray[np.float64_t, ndim=1, mode="c"] parent_bytes):
        cdef np.ndarray[np.float64_t, ndim=1] codeword = np.zeros((self.kernel.blockLength(), ),
                                                                  dtype=np.float64)
        self.kernel.puncture(<double*> codeword.data,
                             <double*> parent_bytes.data)
        return codeword

    def depuncture(self, np.ndarray parent_bytes not None):
        if parent_bytes.dtype == np.uint8:
            return self.depunctureUInt8(parent_bytes)
        elif parent_bytes.dtype == np.float32:
            return self.depunctureFloat(parent_bytes)
        elif parent_bytes.dtype == np.float64:
            return self.depunctureDouble(parent_bytes)
        else:
            return self.depunctureDouble(parent_bytes.astype(np.float64)).astype(parent_bytes.dtype)

    cdef depunctureUInt8(self, np.ndarray[np.uint8_t, ndim=1, mode="c"] parent_bytes):
        cdef np.ndarray[np.uint8_t, ndim=1] codeword = np.zeros((self.kernel.parentBlockLength(), ),
                                                                dtype=np.uint8)
        self.kernel.depuncture(<unsigned char*> codeword.data,
                               <unsigned char*> parent_bytes.data)
        return codeword

    cdef depunctureFloat(self, np.ndarray[np.float32_t, ndim=1, mode="c"] parent_bytes):
        cdef np.ndarray[np.float32_t, ndim=1] codeword = np.zeros((self.kernel.parentBlockLength(), ),
                                                                  dtype=np.float32)
        self.kernel.depuncture[float](<float*> codeword.data, <float*> parent_bytes.data)
        return codeword

    cdef depunctureDouble(self, np.ndarray[np.float64_t, ndim=1, mode="c"] parent_bytes):
        cdef np.ndarray[np.float64_t, ndim=1] codeword = np.zeros((self.kernel.parentBlockLength(), ),
                                                                  dtype=np.float64)
        self.kernel.depuncture(<double*> codeword.data,
                               <double*> parent_bytes.data)
        return codeword


cdef class PolarEncoder:
    cdef polar_interface.Encoder* kernel

    def __cinit__(self, block_size, np.ndarray frozen_bit_positions):
        # print('CYTHON POLAR ENCODER')
        frozen_bit_positions = np.sort(frozen_bit_positions)
        frozen_bit_positions = frozen_bit_positions.astype(np.uint32)
        self.kernel = new polar_interface.ButterflyFipPacked(block_size, frozen_bit_positions)

    def __del__(self):
        del self.kernel

    def setErrorDetection(self, which):
        if which == 0:
            self.kernel.setErrorDetection(new polar_interface.Dummy())
        elif which == 32:
            self.kernel.setErrorDetection(new polar_interface.CRC32())
        else:
            self.kernel.setErrorDetection(new polar_interface.CRC8())

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

        if decoder_impl == "float":
            self.decoder_impl_flag = 1
        elif decoder_impl == "scan":
            self.decoder_impl_flag = 3
        else:
            self.decoder_impl_flag = 0

        # self.decoder_impl_flag = 1 if decoder_impl == "float" else 0
        #print(self.decoder_impl_flag)
        self.kernel = polar_interface.makeDecoder(block_size, list_size, frozen_bit_positions, self.decoder_impl_flag)

    def __del__(self):
        del self.kernel

    def setErrorDetection(self, which):
        if which == 0:
            self.kernel.setErrorDetection(new polar_interface.Dummy())
        elif which == 32:
            self.kernel.setErrorDetection(new polar_interface.CRC32())
        else:
            self.kernel.setErrorDetection(new polar_interface.CRC8())

    def decode(self):
        self.kernel.decode()

    def frozenBits(self):
        v = self.kernel.frozenBits()
        return uint_cppvector_to_ndarray(v)

    def blockLength(self):
        return self.kernel.blockLength()

    def infoLength(self):
        return self.kernel.infoLength()

    def setSystematic(self, flag):
        self.kernel.setSystematic(flag)

    def isSystematic(self):
        return self.kernel.isSystematic()

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

    def getSoftCodeword(self):
        cdef np.ndarray[np.float32_t, ndim=1] llrs = np.zeros((self.kernel.blockLength(), ), dtype=np.float32)
        self.kernel.getSoftCodeword(llrs.data)
        return llrs

    def getSoftInformation(self):
        cdef np.ndarray[np.float32_t, ndim=1] llrs = np.zeros((self.kernel.infoLength(), ), dtype=np.float32)
        self.kernel.getSoftInformation(llrs.data)
        return llrs
