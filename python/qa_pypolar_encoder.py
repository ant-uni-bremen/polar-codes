#!/usr/bin/env python3
# -*- coding: utf-8 -*-
#
# Copyright 2020 Johannes Demel.
#
# SPDX-License-Identifier: GPL-3.0-or-later
#

import sys
print('sys.path')
print(sys.path)
print('end sys.path')

import numpy as np
import unittest
from polar_code_tools import design_snr_to_bec_eta, calculate_bec_channel_capacities, get_frozenBitMap, get_frozenBitPositions, get_polar_generator_matrix, get_polar_encoder_matrix_systematic, frozen_indices_to_map
from polar_code_tools import get_info_indices, get_expanding_matrix, calculate_ga
from channel_construction import ChannelConstructorBhattacharyyaBounds, ChannelConstructorGaussianApproximationDai
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

                    pp = encoder.frozenBits()

                    if not np.all(pf == cf):
                        print(cf)
                        print(pf)
                        print(cf == pf)
                    self.assertListEqual(cf, pp)
                    self.assertListEqual(cf, list(pf))

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
        self.assertEqual(np.linalg.matrix_rank(H), N - K)
        self.assertTrue(np.all(G.dot(H.T) % 2 == 0))

    def check_matrix_domination_contiguity(self, N, f):
        f = np.array(f)
        G = get_polar_generator_matrix(int(np.log2(N)))
        em = get_expanding_matrix(f, N)

        bpi = np.dot(em, np.dot(G, em.T) % 2) % 2
        r = np.dot(bpi, bpi) % 2

        return np.all(r == np.identity(N - len(f)))

    def test_004_encoder_config(self):
        print('Test Encoder Configuration')
        snr = -1.
        for n in range(4, 11):
            N = 2 ** n
            self.validate_config(N, int(N * .75), snr)
            self.validate_config(N, N // 2, snr)
            self.validate_config(N, N // 4, snr)
            self.validate_config(N, N // 8, snr)

    def validate_config(self, N, K, snr):
        eta = design_snr_to_bec_eta(snr, 1.0)
        polar_capacities = calculate_bec_channel_capacities(eta, N)
        f = get_frozenBitPositions(polar_capacities, N - K)
        f = np.sort(f)
        frozenBitMap = get_frozenBitMap(polar_capacities, N - K)
        info_pos = np.setdiff1d(np.arange(N, dtype=f.dtype), f)
        self.assertEqual(info_pos.size, K)
        self.assertEqual(f.size, N - K)
        self.assertEqual(np.sum(frozenBitMap), -K)

        p = pypolar.PolarEncoder(N, f)
        self.assertEqual(p.blockLength(), N)

        self.assertTrue(np.all(f == p.frozenBits()))
        self.assertTrue(np.all(f == np.arange(N)[np.where(frozenBitMap == 0)]))

        self.assertTrue(p.isSystematic())
        p.setSystematic(False)
        self.assertFalse(p.isSystematic())
        p.setSystematic(True)
        self.assertTrue(p.isSystematic())
        self.check_matrix_domination_contiguity(N, p.frozenBits())

    def test_005_cpp_encoder_impls(self):
        return
        snr = -1.
        test_size = np.array([4, 5, 6, 9, 10, 11])
        test_size = np.array([4, 5, 6, 7, 9, 10, 11])
        test_size = np.arange(8, 11)
        for i in test_size:
            N = 2 ** i
            self.validate_encoder(N, int(N * .75), snr)
            self.validate_encoder(N, N // 2, snr)
            self.validate_encoder(N, N // 4, snr)
            self.validate_encoder(N, N // 8, snr)

    def initialize_encoder(self, N, K, snr):
        try:
            np.seterr(invalid='raise')
            cc = ChannelConstructorGaussianApproximationDai(N, snr)
        except ValueError:
            np.seterr(invalid='warn')
            cc = ChannelConstructorGaussianApproximationDai(N, snr)

            cc = ChannelConstructorBhattacharyyaBounds(N, snr)

        f = np.sort(cc.getSortedChannels()[0:N - K])

        p = pypolar.PolarEncoder(N, f)
        return p

    def validate_encoder(self, N, K, snr):
        print("Encoder CPP test ({}, {}) -> {}dB".format(N, K, snr))
        p = self.initialize_encoder(N, K, snr)
        frozenBitMap = frozen_indices_to_map(p.frozenBits(), N)
        info_pos = get_info_indices(np.array(p.frozenBits()), N)
        if not self.check_matrix_domination_contiguity(N, p.frozenBits()):
            print('invalid code parameters!')
            return

        err_ctr = 0
        for i in np.arange(10):
            u = np.random.randint(0, 2, K).astype(dtype=np.uint8)
            d = np.packbits(u)
            dref = np.copy(d)

            # The pythonic method
            cw_pack = p.encode_vector(d)

            # assert input did not change!
            self.assertTrue(np.all(d == dref))

            xm = encode_systematic_matrix(u, N, frozenBitMap)
            xmp = np.packbits(xm)

            # assert code is systematic
            self.assertTrue(np.all(u == xm[info_pos]))
            self.assertTrue(np.all(np.unpackbits(cw_pack)[info_pos] == u))

            # assert equal results!
            if not np.all(xmp == cw_pack):
                err_ctr += 1
            self.assertTrue(np.all(xmp == cw_pack))
            self.assertTrue(np.all(xm == np.unpackbits(cw_pack)))

        if err_ctr > 0:
            print('Miserable failure! {}'.format(err_ctr))


# import aff3ct
# class ComparePolarEncoderTests(unittest.TestCase):
#     def setUp(self):
#         pass

#     def tearDown(self):
#         pass

#     def initialize_encoder(self, N, K, snr):
#         try:
#             np.seterr(invalid='raise')
#             cc = ChannelConstructorGaussianApproximationDai(N, snr)
#         except ValueError:
#             np.seterr(invalid='warn')
#             cc = ChannelConstructorGaussianApproximationDai(N, snr)

#             cc = ChannelConstructorBhattacharyyaBounds(N, snr)

#         f = np.sort(cc.getSortedChannels()[0:N - K])

#         p = pypolar.PolarEncoder(N, f)
#         return p

#     def test_001_evaluate(self):
#         N = 128
#         K = N // 2
#         snr = -1.
#         penc = self.initialize_encoder(N, K, snr)
#         frozen_bits = penc.frozenBits()
#         print(frozen_bits)
#         # setErrorDetection

#         frozen_bit_mask = np.zeros(N, dtype=bool)
#         # print(frozen_bit_mask)
#         frozen_bit_mask[frozen_bits] = True
#         # print(frozen_bit_mask)
#         aenc = aff3ct.PolarEncoder(N, K, frozen_bit_mask)

#         pdet = pypolar.Detector(8, "CRC")
#         adet = aff3ct.CRC(K, "8-CCITT")

#         u = np.random.randint(0, 2, K).astype(dtype=np.uint8)
#         d = np.packbits(u)
#         dref = np.copy(d)

#         pc = penc.encode_vector(d)
#         ac = aenc.encode(u.astype(dtype=np.int32))

#         self.assertTupleEqual(tuple(pc), tuple(np.packbits(ac)))

#         print(pc)
#         print(np.packbits(ac))

#         print(d)
#         print(pdet.generate(d))
#         print(np.packbits(adet.generate(u.astype(np.int32))))


if __name__ == '__main__':
    unittest.main(failfast=False)
