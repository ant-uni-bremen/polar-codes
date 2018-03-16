#!/usr/bin/env python3
from __future__ import print_function, division
import numpy as np
import scipy.special as sps
import matplotlib.pyplot as plt
import unittest
import sys
import time
# sys.path.append('./build/lib')
# sys.path.append('./build/lib.linux-x86_64-2.7')
import pypolar
from polar_code_tools import design_snr_to_bec_eta, calculate_bec_channel_capacities, get_frozenBitMap, get_frozenBitPositions, get_polar_generator_matrix


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


def spc_most_efficient(u, N, frozenBitMap):
    '''
    This function is defective!
    '''
    x = np.copy(frozenBitMap)
    x[np.where(frozenBitMap == -1)] = u
    d = np.copy(x)
    n = int(np.log2(N))
    for r in np.arange(N - 1, -1, -1):
        br = tuple(np.binary_repr(r, n))
        print(r, br)

        if frozenBitMap[r] > -1:
            print('frozen_bit')
            # d[r] = frozenBitMap[r]
            if br[0] == '0':
                d[r] = (frozenBitMap[r] + d[r + 2 ** (n - 1)]) % 2
            else:
                d[r] = frozenBitMap[r]
            for j in np.arange(n - 2, -1, -1):
                kappa = 2 ** (n - j - 1)
                # print(i, j, bits[j], kappa)
                if br[j] == '0':
                    d[r] = (d[r] + d[r + kappa]) % 2
                else:
                    d[r] = d[r]

        else:
            print('info_bit')
            if br[-1] == '0':
                d[r] = (x[r] + d[r]) % 2
            else:
                d[r] = x[r]
            for j in np.arange(1, n):
                kappa = 2 ** (n - j - 1)
                # print(i, j, bits[j], kappa)
                if br[j] == '0':
                    d[r] = (d[r] + d[r + kappa]) % 2
                else:
                    d[r] = d[r]
    print(d)
    print(np.where(frozenBitMap > -1), d[np.where(frozenBitMap > -1)])
    print(x)
    x[np.where(frozenBitMap > -1)] = d[np.where(frozenBitMap > -1)]
    return x


def matrix_row_weight(G):
    w = np.sum(G, axis=1)
    print(w)
    w = np.sum(G, axis=0)
    print(w)


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


    def test_002_cpp_encoder_impls(self):
        for i in range(5, 11):
            N = 2 ** i
            K = N // 2
            verify_cpp_encoder_impl(N, K)


def verify_cpp_encoder_impl(N=2 ** 4, K=5):
    eta = design_snr_to_bec_eta(2, 1.0)
    polar_capacities = calculate_bec_channel_capacities(eta, N)
    f = get_frozenBitPositions(polar_capacities, N - K)
    frozenBitMap = get_frozenBitMap(polar_capacities, N - K)

    p = pypolar.PolarEncoder(N, f)
    print("Encoder CPP test ({}, {})".format(N, K))

    ctr = 0
    md = 0
    mp = 0
    cmp = 0
    for i in np.arange(10):
        u = np.random.randint(0, 2, K).astype(dtype=np.uint8)
        d = np.packbits(u)
        # print(N, K, len(u), len(d), u, d)
        print(d)

        sm = time.time()
        xm = encode_systematic_matrix(u, N, frozenBitMap)
        um = time.time()
        pm = time.time()
        cw_pack = p.encode_vector(d)
        em = time.time()
        md += um - sm
        mp += em - pm
        xmp = np.packbits(xm)
        cmp += p.encoder_duration()
        print(xm)
        print(np.unpackbits(cw_pack))

        assert np.all(xmp == cw_pack)
        assert np.all(xm == np.unpackbits(cw_pack))
        ctr += 1

    m = np.array([md, mp]) / ctr
    m *= 1e6
    cmp /= 1.e3 * ctr
    print('N={}, matrix: {:.2f}us, CPP packed: {:.2f}us/{:.2f}us'.format(N, m[0], m[1], cmp))


def verify_cpp_decoder_impl(N=2 ** 6, K=2 ** 5, n_iterations=100, crc=None):
    print('verify CPP decoder implementation with ({}, {}) polar code'.format(N, K))
    eta = design_snr_to_bec_eta(2, float(1. * K / N))
    polar_capacities = calculate_bec_channel_capacities(eta, N)
    f = get_frozenBitPositions(polar_capacities, N - K)
    # f = np.sort(f)
    # print(f)
    # f = pypolar.frozen_bits(N, K, 2)

    p = pypolar.PolarEncoder(N, f)
    dec = pypolar.PolarDecoder(N, 1, f, 'char')

    if crc is 'CRC8':
        p.setErrorDetection()
        dec.setErrorDetection()

    ctr = 0
    num_errors = 0
    for i in np.arange(n_iterations):
        u = np.random.randint(0, 2, K).astype(dtype=np.uint8)
        d = np.packbits(u)
        dc = np.copy(d)

        cw_pack = p.encode_vector(dc)
        b = np.unpackbits(cw_pack)
        llrs = -2. * b + 1.
        llrs = llrs.astype(dtype=np.float32)
        llrs += np.random.normal(0.0, .001, len(llrs))
        dhat = dec.decode_vector(llrs)
        # print(d)
        # print(dhat)
        if not np.all(dhat == d) and crc is None:
            print('Decoder test fails in iteration', i)
            ud = np.unpackbits(d)
            udhat = np.unpackbits(dhat)
            print(d)
            print(dhat)
            print(ud)
            print(udhat)
            print(np.sum(udhat == ud) - len(ud))
            # num_errors += 1

        if crc is 'CRC8':
            assert np.all(dhat[0:-1] == d[0:-1])
        else:
            assert np.all(dhat == d)
        ctr += 1
    if num_errors > 0:
        print('Decoder test failed in {} out of {}'.format(num_errors, n_iterations))
    assert num_errors == 0


def verify_cpp_decoder_impls():
    n_iterations = 100
    inv_coderate = 4 / 3
    for n in range(5, 11):
        N = 2 ** n
        K = int(N // inv_coderate)
        verify_cpp_decoder_impl(N, K, n_iterations)

    inv_coderate = 2
    for n in range(5, 11):
        N = 2 ** n
        K = int(N // inv_coderate)
        verify_cpp_decoder_impl(N, K, n_iterations)
        verify_cpp_decoder_impl(N, K, n_iterations, 'CRC8')

    inv_coderate = 4
    for n in range(5, 9):
        N = 2 ** n
        K = int(N // inv_coderate)
        verify_cpp_decoder_impl(N, K, n_iterations)


def verify_frozen_bit_positions():
    print('verify frozen bit positions')
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
                assert np.all(pf == cf)
                assert np.all(pp == cf)
                assert np.all(pd == cf)


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

    validate_systematic_matrix(N, f, frozenBitMap)

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


def validate_generator_and_check_matrix(Gs, f):
    K, N = np.shape(Gs)
    P = Gs[:, f]
    print(P)
    G = np.hstack((np.identity(K, dtype=Gs.dtype), P))
    H = np.hstack((P.T, np.identity(N - K, dtype=Gs.dtype)))
    print('H Rank', np.linalg.matrix_rank(H))
    assert np.linalg.matrix_rank(H) == N - K
    assert np.all(G.dot(H.T) % 2 == 0)
    return 1 + np.min(np.sum(P, axis=1))


def validate_systematic_matrix(N, f, frozenBitMap):
    K = N - len(f)
    n = int(np.log2(N))
    G = get_polar_generator_matrix(n)
    ip = np.delete(np.arange(N), f)
    Gi = G[ip, :]
    I = np.identity(N, dtype=np.uint8)
    I[f, :] = 0
    gm = Gi.dot(I) #% 2
    Gst = gm.dot(G) % 2
    Gs = get_polar_encoder_matrix_systematic(N, f)
    assert np.all(Gst == Gs)

    for i in range(10):
        u = np.random.randint(0, 2, K).astype(dtype=np.uint8)
        x = np.copy(frozenBitMap)
        x[np.where(frozenBitMap == -1)] = u
        x = x.dot(G) % 2

        xu = (u.dot(Gi) % 2)
        assert np.all(x == xu)

        x[np.where(frozenBitMap > -1)] = frozenBitMap[np.where(frozenBitMap > -1)]
        assert np.all(x == (u.dot(gm) % 2))

        xs = u.dot(Gs) % 2
        x = x.dot(G) % 2

        assert np.all(x == xs)
    validate_generator_and_check_matrix(Gs, f)


def get_polar_encoder_matrix_systematic(N, f):
    n = int(np.log2(N))
    G = get_polar_generator_matrix(n)
    ip = np.delete(np.arange(N), f)
    Gi = G[ip, :]
    I = np.identity(N, dtype=np.uint8)
    I[f, :] = 0
    gm = Gi.dot(I) #% 2
    return gm.dot(G) % 2


def main():
    # calculate_code_properties(32, 16, 0.0)
    verify_frozen_bit_positions()
    verify_encode_systematic()
    verify_cpp_encoder_impls()
    verify_cpp_decoder_impls()


if __name__ == '__main__':
    unittest.main(failfast=True)
    # main()
