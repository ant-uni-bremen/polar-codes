#!/usr/bin/env python3
from __future__ import print_function, division
import numpy as np
import unittest
# import os
# import time
from polar_code_tools import design_snr_to_bec_eta, calculate_bec_channel_capacities, get_frozenBitMap, get_frozenBitPositions, get_polar_generator_matrix, get_polar_encoder_matrix_systematic, frozen_indices_to_map
from polar_code_tools import get_info_indices, get_expanding_matrix, calculate_ga
from channel_construction import ChannelConstructorBhattacharyyaBounds, ChannelConstructorGaussianApproximation
import sys
sys.path.insert(0, './build/lib.linux-x86_64-2.7')

import pypolar

'''
EncoderA of the paper:
    Harish Vangala, Yi Hong, and Emanuele Viterbo,
    "Efficient Algorithms for Systematic Polar Encoding",
    IEEE Communication Letters, 2015.
'''


def polar_encode_systematic(u, N, frozenBitMap):
    # print(u.dtype, frozenBitMap.dtype)
    y = frozenBitMap
    x = np.zeros(N, dtype=int)
    x[np.where(frozenBitMap == -1)] = u
    return polar_encode_systematic_algorithm_A(y, x, N, frozenBitMap)


def polar_encode_systematic_algorithm_A(y, x, N, frozenBitMap):
    n = int(np.log2(N))

    X = np.zeros((N, n + 1), dtype=int)
    X[:, 0] = y
    X[:, -1] = x

    for i in np.arange(N - 1, -1, -1):
        bits = tuple(np.binary_repr(i, n))
        # print(i, ' == ', bits)
        if frozenBitMap[i] < 0:
            # print('is info bit', frozenBitMap[i])
            for j in np.arange(n - 1, -1, -1):
                kappa = 2 ** (n - j - 1)
                # print(i, j, bits[j], kappa)
                if bits[j] == '0':
                    X[i, j] = (X[i, j + 1] + X[i + kappa, j + 1]) % 2
                else:
                    X[i, j] = X[i, j + 1]
        else:
            # print('is frozen bit', frozenBitMap[i])
            for j in np.arange(n):
                kappa = 2 ** (n - j - 1)
                # print(i, j, bits[j], kappa)
                if bits[j] == '0':
                    X[i, j + 1] = (X[i, j] + X[i + kappa, j]) % 2
                else:
                    X[i, j + 1] = X[i, j]

    return X[:, 0], X[:, -1]


def encode_systematic_matrix(u, N, frozenBitMap):
    n = int(np.log2(N))
    G = get_polar_generator_matrix(n)
    x = np.copy(frozenBitMap)
    x[np.where(frozenBitMap == -1)] = u
    # print(u, x, np.where(frozenBitMap == -1))
    # print(frozenBitMap[np.where(frozenBitMap > -1)])
    x = x.dot(G) % 2
    x[np.where(frozenBitMap > -1)] = frozenBitMap[np.where(frozenBitMap > -1)]
    x = x.dot(G) % 2
    return x


def encode_matrix(u, N, frozenBitMap):
    n = int(np.log2(N))
    G = get_polar_generator_matrix(n)
    x = np.copy(frozenBitMap)
    x[np.where(frozenBitMap == -1)] = u
    x = x.dot(G) % 2
    return x


class PolarEncoderTests(unittest.TestCase):
    def setUp(self):
        pass

    def tearDown(self):
        pass

    def test_001_encode_systematic(self):
        frozenBitMap = np.array([0, -1, 0, -1, 0, -1, -1, -1], dtype=int)
        for i in range(100):
            u = np.random.randint(0, 2, 5)
            Y, X = polar_encode_systematic(u, 8, frozenBitMap)
            xm = encode_systematic_matrix(u, 8, frozenBitMap)
            self.assertTrue(np.all(X == xm))

        N = 2 ** 6
        K = N // 2
        eta = design_snr_to_bec_eta(-1.59, 1.0)
        polar_capacities = calculate_bec_channel_capacities(eta, N)
        frozenBitMap = get_frozenBitMap(polar_capacities, N - K)
        # print(frozenBitMap)

        for i in range(100):
            u = np.random.randint(0, 2, K)
            Y, X = polar_encode_systematic(u, N, frozenBitMap)
            xm = encode_systematic_matrix(u, N, frozenBitMap)
            self.assertTrue(np.all(X == xm))

    def test_002_frozen_bit_positions(self):
        for snr in np.arange(-1.5, 3.5, .25):
            for inv_coderate in np.array([8, 6, 5, 4, 3, 2, 1.5, 1.2]):
                for n in range(6, 11):
                    N = 2 ** n
                    K = int(N / inv_coderate)
                    # print(N, K, inv_coderate)
                    cf = pypolar.frozen_bits(N, K, snr)
                    eta = design_snr_to_bec_eta(snr, 1. * K / N)
                    polar_capacities = calculate_bec_channel_capacities(eta, N)
                    pf = get_frozenBitPositions(polar_capacities, N - K)
                    pf = np.sort(pf)
                    encoder = pypolar.PolarEncoder(N, cf)
                    decoder = pypolar.PolarDecoder(N, 1, cf)

                    pp = encoder.frozenBits()
                    pd = decoder.frozenBits()

                    if not np.all(pf == cf):
                        print(cf)
                        print(pf)
                        print(cf == pf)
                    self.assertTrue(np.all(pf == cf))
                    self.assertTrue(np.all(pp == cf))
                    self.assertTrue(np.all(pd == cf))

    def test_003_systematic_matrix(self):
        snr = 2.
        for n in range(4, 8):
            N = 2 ** n
            K = N // 2
            self.matrix_validation(N, K, snr)
            self.matrix_validation(N, K // 2, snr)

    def matrix_validation(self, N, K, snr):
        eta = design_snr_to_bec_eta(snr, 1.0)
        polar_capacities = calculate_bec_channel_capacities(eta, N)
        f = np.sort(get_frozenBitPositions(polar_capacities, N - K))
        ip = np.setdiff1d(np.arange(N, dtype=f.dtype), f)
        frozenBitMap = get_frozenBitMap(polar_capacities, N - K)

        n = int(np.log2(N))
        G = get_polar_generator_matrix(n)
        Gs = get_polar_encoder_matrix_systematic(N, f)
        for i in range(10):
            u = np.random.randint(0, 2, K).astype(dtype=np.uint8)
            x = np.zeros(N, dtype=np.uint8)
            x[ip] = u
            xref = np.copy(frozenBitMap)
            xref[np.where(frozenBitMap == -1)] = u
            self.assertTrue(np.all(x == xref))
            x = x.dot(G) % 2
            x[f] = 0
            x = x.dot(G) % 2
            xs = u.dot(Gs) % 2

            self.assertTrue(np.all(x == xs))
        self.matrix_gen_check_validation(Gs, f)

    def matrix_gen_check_validation(self, Gs, f):
        K, N = np.shape(Gs)
        P = Gs[:, f]
        G = np.hstack((np.identity(K, dtype=Gs.dtype), P))
        H = np.hstack((P.T, np.identity(N - K, dtype=Gs.dtype)))
        self.assertEquals(np.linalg.matrix_rank(H), N - K)
        self.assertTrue(np.all(G.dot(H.T) % 2 == 0))

    def check_matrix_domination_contiguity(self, N, f):
        # print('Polar Code({}, {})'.format(N, N - len(f)))

        G = get_polar_generator_matrix(int(np.log2(N)))
        em = get_expanding_matrix(f, N)

        bpi = np.dot(em, np.dot(G, em.T) % 2) % 2
        r = np.dot(bpi, bpi) % 2

        # if not np.all(r == np.identity(N - len(f))):
        #     print(G)
        #     print(r)
        return np.all(r == np.identity(N - len(f)))
        # self.assertTrue(np.all(r == np.identity(N - len(f))))

    def test_004_encoder_config(self):
        print('Test Encoder Configuration')
        snr = -1.
        for n in range(4, 11):
            N = 2 ** n
            self.validate_config(N, int(N * .75), snr)
            self.validate_config(N, N // 2, snr)
            self.validate_config(N, N // 4, snr)
            self.validate_config(N, N // 8, snr)
        # self.assertTrue(False)

    def validate_config(self, N, K, snr):
        eta = design_snr_to_bec_eta(snr, 1.0)
        polar_capacities = calculate_bec_channel_capacities(eta, N)
        f = get_frozenBitPositions(polar_capacities, N - K)
        f = np.sort(f)
        frozenBitMap = get_frozenBitMap(polar_capacities, N - K)
        info_pos = np.setdiff1d(np.arange(N, dtype=f.dtype), f)
        self.assertEquals(info_pos.size, K)
        self.assertEquals(f.size, N - K)
        self.assertEquals(np.sum(frozenBitMap), -K)

        p = pypolar.PolarEncoder(N, f)
        self.assertEquals(p.blockLength(), N)

        self.assertTrue(np.all(f == p.frozenBits()))
        self.assertTrue(np.all(f == np.arange(N)[np.where(frozenBitMap == 0)]))

        self.assertTrue(p.isSystematic())
        p.setSystematic(False)
        self.assertFalse(p.isSystematic())
        p.setSystematic(True)
        self.assertTrue(p.isSystematic())
        self.check_matrix_domination_contiguity(N, p.frozenBits())

    def test_005_cpp_encoder_impls(self):
        snr = -1.
        test_size = np.array([4, 5, 6, 9, 10, 11])
        test_size = np.array([4, 5, 6, 7, 9, 10, 11])
        test_size = np.arange(4, 11)
        for i in test_size:
            N = 2 ** i
            self.validate_encoder(N, int(N * .75), snr)
            self.validate_encoder(N, N // 2, snr)
            self.validate_encoder(N, N // 4, snr)
            self.validate_encoder(N, N // 8, snr)

    def initialize_encoder(self, N, K, snr):
        # print('initialize encoder')
        try:
            np.seterr(invalid='raise')
            cc = ChannelConstructorGaussianApproximation(N, snr)
        except ValueError:
            print('GA is a miserable failure!')
            np.seterr(invalid='warn')
            cc = ChannelConstructorGaussianApproximation(N, snr)
            # print(cc.getCapacities())

            cc = ChannelConstructorBhattacharyyaBounds(N, snr)
            # print(cc.getCapacities())
        # bb = ChannelConstructorBhattacharyyaBounds(N, snr)
        # ga = ChannelConstructorGaussianApproximation(N, snr)
        # bb_caps = bb.getCapacities()
        # ga_caps = ga.getCapacities()
        f = np.sort(cc.getSortedChannels())[0:N - K]
        cc = None
        # eta = design_snr_to_bec_eta(snr, 1.0)
        # polar_capacities = calculate_bec_channel_capacities(eta, N)
        # f = get_frozenBitPositions(polar_capacities, N - K)
        # f = np.sort(f)
        p = pypolar.PolarEncoder(N, f)
        return p

    def validate_encoder(self, N, K, snr):
        print("Encoder CPP test ({}, {}) -> {}dB".format(N, K, snr))
        p = self.initialize_encoder(N, K, snr)
        frozenBitMap = frozen_indices_to_map(p.frozenBits(), N)
        info_pos = get_info_indices(p.frozenBits(), N)
        if not self.check_matrix_domination_contiguity(N, p.frozenBits()):
            print('invalid code parameters!')
            return

        err_ctr = 0
        for i in np.arange(10):
            # print(i)
            u = np.random.randint(0, 2, K).astype(dtype=np.uint8)
            d = np.packbits(u)
            dref = np.copy(d)

            # The 'C++' methods
            p.setInformation(d)
            p.encode()
            codeword = p.getEncodedData()

            # The pythonic method
            cw_pack = p.encode_vector(d)

            # assert input did not change!
            self.assertTrue(np.all(d == dref))
            # assert C++ methods yield same results.
            self.assertTrue(np.all(cw_pack == codeword))

            xm = encode_systematic_matrix(u, N, frozenBitMap)
            xmp = np.packbits(xm)

            # assert code is systematic
            self.assertTrue(np.all(u == xm[info_pos]))
            self.assertTrue(np.all(np.unpackbits(cw_pack)[info_pos] == u))

            # assert equal results!
            if not np.all(xmp == cw_pack):
                err_ctr += 1
                # print('This code is a miserable failure!')
            self.assertTrue(np.all(xmp == cw_pack))
            self.assertTrue(np.all(xm == np.unpackbits(cw_pack)))

        if err_ctr > 0:
            print('Miserable failure! {}'.format(err_ctr))

    def test_006_cpp_decoder_impls(self):
        print('TEST: CPP Decoder')
        snr = -1.
        test_size = np.arange(4, 11, dtype=int)
        # test_size = np.array([4, 5, 6, 8, 9, 10], dtype=int)
        for i in test_size:
            N = 2 ** i
            # self.validate_decoder(N, int(N * .75), snr)
            self.validate_decoder(N, N // 2, snr)
            self.validate_decoder(N, N // 4, snr)
            self.validate_decoder(N, N // 8, snr)

    def validate_decoder(self, N, K, snr, crc=None):
        print("Decoder CPP test ({}, {}) -> {}dB".format(N, K, snr))
        p = self.initialize_encoder(N, K, snr)
        # info_pos = get_info_indices(p.frozenBits(), N)
        # if not self.check_matrix_domination_contiguity(N, p.frozenBits()):
        #     print('invalid code parameters!')
        #     return
        f = p.frozenBits()
        dec0 = pypolar.PolarDecoder(N, 1, f, 'char')
        dec1 = pypolar.PolarDecoder(N, 1, f, 'float')
        dec2 = pypolar.PolarDecoder(N, 4, f, 'float')
        dec3 = pypolar.PolarDecoder(N, 4, f, 'scan')
        # if crc is 'CRC8':
        p.setErrorDetection()
        dec0.setErrorDetection()
        dec1.setErrorDetection()
        dec2.setErrorDetection()
        dec3.setErrorDetection()
        print('Decoder Initialization finished!')

        for i in np.arange(10):
            # print(i)
            u = np.random.randint(0, 2, K).astype(dtype=np.uint8)
            d = np.packbits(u)

            # The pythonic method
            cw_pack = p.encode_vector(d)
            b = np.unpackbits(cw_pack)
            llrs = -2. * b + 1.
            llrs = llrs.astype(dtype=np.float32)
            # llrs += np.random.normal(0.0, .001, len(llrs))

            dhat0 = dec0.decode_vector(llrs)
            self.assertTrue(np.all(d == dhat0))

            dhat1 = dec1.decode_vector(llrs)
            self.assertTrue(np.all(d == dhat1))

            dhat2 = dec2.decode_vector(llrs)
            self.assertTrue(np.all(d == dhat2))

            dhat3 = dec3.decode_vector(llrs)
            self.assertTrue(np.all(d == dhat3))

            si = dec3.getSoftInformation()
            sc = dec3.getSoftCodeword()
            # print(llrs)
            # print(sc)
            # print(np.sign(sc) == np.sign(llrs))
            self.assertTrue(np.all(np.sign(sc) == np.sign(llrs)))


def get_polar_capacities(N, snr):
    eta = design_snr_to_bec_eta(snr, 1.0)
    return calculate_bec_channel_capacities(eta, N)


def matrix_row_weight(G):
    w = np.sum(G, axis=1)
    print(w)
    w = np.sum(G, axis=0)
    print(w)


def calculate_code_properties(N, K, design_snr_db):
    eta = design_snr_to_bec_eta(design_snr_db, 1.0 * K / N)
    polar_capacities = calculate_bec_channel_capacities(eta, N)
    frozenBitMap = get_frozenBitMap(polar_capacities, N - K)

    f = pypolar.frozen_bits(N, K, design_snr_db)
    p = pypolar.PolarEncoder(N, f)
    Gp = get_polar_generator_matrix(int(np.log2(N)))
    print(Gp)

    assert np.all(np.where(frozenBitMap > -1) == f)

    numInfoWords = 2 ** K
    n_prepend_bits = int(8 * np.ceil(K / 8.) - K)
    print(n_prepend_bits)
    weights = {}
    for i in range(numInfoWords):
        # b = np.binary_repr(i, K + n_prepend_bits)
        b = np.binary_repr(i, K)
        u = np.array([int(l) for l in b], dtype=np.uint8)
        # nb = np.concatenate((np.zeros(n_prepend_bits, dtype=nb.dtype), nb))
        nbp = np.packbits(u)
        cw = p.encode_vector(nbp)
        # xm = encode_systematic_matrix(u, N, frozenBitMap)
        c = np.unpackbits(cw)
        # assert np.all(xm == c)
        weight = np.sum(c)
        if weight in weights:
            weights[weight] += 1
        else:
            weights[weight] = 1
        # nb = bin(i)
        # print(i, b, u, nbp, c)
    print(f)
    print(frozenBitMap)
    # print(n_prepend_bits)
    weights.pop(0)
    print(weights)
    dmin_ext_search = np.min(weights.keys())
    print(dmin_ext_search)

    # validate_systematic_matrix(N, f, frozenBitMap)

    Gs = get_polar_encoder_matrix_systematic(N, f)

    P = Gs[:, f]
    # print(P)
    G = np.hstack((np.identity(K, dtype=Gs.dtype), P))
    H = np.hstack((P.T, np.identity(N - K, dtype=Gs.dtype)))
    # print(P)
    print(H)
    # print(G.dot(H.T) % 2)
    #
    # print(Gs.dot(Gs.T) % 2)

    print(np.linalg.matrix_rank(H))
    dmin_H = np.min(np.sum(H, axis=1))
    dmin_P = 1 + np.min(np.sum(P, axis=1))
    print(np.sum(H, axis=1))
    print(np.sum(P, axis=1))
    print('search {} vs {} H, P{}'.format(dmin_ext_search, dmin_H, dmin_P))
    assert dmin_ext_search == dmin_P


if __name__ == '__main__':
    unittest.main(failfast=True)
