#!/usr/bin/env python3
from __future__ import print_function, division
import numpy as np
import scipy.special as sps
import matplotlib.pyplot as plt

import sys, time
# sys.path.append('./build/lib')
# sys.path.append('./build/lib.linux-x86_64-2.7')
import pypolar



'''
EncoderA of the paper:
    Harish Vangala, Yi Hong, and Emanuele Viterbo,
    "Efficient Algorithms for Systematic Polar Encoding",
    IEEE Communication Letters, 2015.
'''


def odd_rec(iwn):
    return iwn ** 2


def even_rec(iwn):
    return 2 * iwn - iwn ** 2


def calc_vector_capacities_one_recursion(iw0):
    degraded = odd_rec(iw0)
    upgraded = even_rec(iw0)
    iw1 = np.empty(2 * len(iw0), dtype=degraded.dtype)
    iw1[0::2] = degraded
    iw1[1::2] = upgraded
    return iw1


def calculate_bec_channel_capacities_vector(initial_channel, block_power):
    # compare [0, Arikan] eq. 6
    # this version is ~ 180 times faster than the loop version with 2**22 synthetic channels
    iw = np.array([initial_channel, ], dtype=float)
    for i in range(block_power):
        iw = calc_vector_capacities_one_recursion(iw)
    return iw


def calculate_bec_channel_capacities(eta, block_size):
    # compare [0, Arikan] eq. 6
    iw = 1 - eta  # holds for BEC as stated in paper
    lw = int(np.log2(block_size))
    return calculate_bec_channel_capacities_vector(iw, lw)


def design_snr_to_bec_eta(design_snr, coderate):
    # minimum design snr = -1.5917 corresponds to BER = 0.5
    s = 10. ** (design_snr / 10.)
    s *= 2.
    s *= coderate
    # float linearDesignSNR = pow(10.0, designSNR/10.0);
    # mInitialParameter = exp(-2.0 * linearDesignSNR
    #                         * mInformationLength / mBlockLength);
    return np.exp(-s)


def get_frozenBitPositions(capacities, n_frozen):
    indices = np.argsort(capacities)[0:n_frozen]
    return indices


def get_frozenBitMap(capacities, n_frozen):
    frozenBitMap = np.ones(len(capacities), dtype=int) * -1
    indices = get_frozenBitPositions(capacities, n_frozen)
    frozenBitMap[indices] = 0
    return frozenBitMap


def polar_encode_systematic(u, N, frozenBitMap):
    # print(u.dtype, frozenBitMap.dtype)
    y = frozenBitMap
    x = np.zeros(N, dtype=int)
    x[np.where(frozenBitMap == -1)] = u
    return polar_encode_systematic_algorithm_A(y, x, N, frozenBitMap)


def polar_encode_systematic_algorithm_A(y, x, N, frozenBitMap):
    n = int(np.log2(N))

    X = np.zeros((N, n+1), dtype=int)
    X[:,  0] = y
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


def get_polar_generator_matrix(n):
    F = Fk = np.array([[1, 0], [1, 1]]).astype(int)
    for i in range(n - 1):
        F = np.kron(F, Fk)
    # print F
    return F


def verify_encode_systematic():
    frozenBitMap = np.array([0, -1, 0, -1, 0, -1, -1, -1]).astype(dtype=int)
    for i in range(100):
        u = np.random.randint(0, 2, 5)
        Y, X = polar_encode_systematic(u, 8, frozenBitMap)
        xm = encode_systematic_matrix(u, 8, frozenBitMap)
        assert np.all(X == xm)

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
        assert np.all(X == xm)


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


def bit_reverse(val, bitwidth):
    # print(val, bitwidth)
    b = np.binary_repr(val, bitwidth)
    # print(b, b[::-1])
    r = int(b[::-1], 2)
    # print(r)
    return r


def get_bitreversed_vector(bitwidth):
    v = np.zeros(2**bitwidth, dtype=int)
    for i in np.arange(2**bitwidth):
        v[i] = bit_reverse(i, bitwidth)
    return v


def matrix_row_weight(G):
    w = np.sum(G, axis=1)
    print(w)
    w = np.sum(G, axis=0)
    print(w)


def verify_cpp_encoder_impl():
    N = 2 ** 4
    K = N // 2 - 3
    eta = design_snr_to_bec_eta(2, 1.0)
    polar_capacities = calculate_bec_channel_capacities(eta, N)
    f = get_frozenBitPositions(polar_capacities, N - K)
    frozenBitMap = get_frozenBitMap(polar_capacities, N - K)
    print(frozenBitMap)
    print(f)

    p = pypolar.PolarEncoder(N, f)
    pu = pypolar.PolarEncoder(N, f, 'Unpacked')

    ctr = 0
    md = 0
    mu = 0
    mp = 0
    for i in np.arange(10):
        u = np.random.randint(0, 2, K).astype(dtype=np.uint8)
        d = np.packbits(u)
        print(N, K, len(u), len(d), u, d)

        sm = time.time()
        xm = encode_systematic_matrix(u, N, frozenBitMap)
        um = time.time()
        cw_char = pu.encode_vector(d)
        pm = time.time()
        cw_pack = p.encode_vector(d)
        em = time.time()
        md += um - sm
        mu += pm - um
        mp += em - pm
        print(xm, cw_pack)

        assert np.all(cw_char == cw_pack)
        assert np.all(np.packbits(xm) == cw_pack)
        assert np.all(xm == np.unpackbits(cw_pack))
        ctr += 1

    m = np.array([md, mu, mp]) / ctr
    m *= 1e6
    print('N={}, matrix: {:.2f}us, unpacked: {:.2f}us, packed: {:.2f}us'.format(N, m[0], m[1], m[2]))


def verify_cpp_decoder_impl(N=2 ** 6, n_iterations=100):
    print('verify CPP decoder implementation with codeword size: ', N)
    K = N // 4

    eta = design_snr_to_bec_eta(2, 1.0)
    polar_capacities = calculate_bec_channel_capacities(eta, N)
    f = get_frozenBitPositions(polar_capacities, N - K)
    print(f)

    p = pypolar.PolarEncoder(N, f)
    dec = pypolar.PolarDecoder(N, 1, f)
    # u = np.random.randint(0, 2, K).astype(dtype=np.uint8)
    # d = np.packbits(u)

    # cw_pack = p.encode_vector(d)
    # b = np.unpackbits(cw_pack)
    # llrs = -2 * b + 1
    # llrs = llrs.astype(dtype=np.float32)

    # bhat = dec.decode_vector(llrs)

    ctr = 0
    for i in np.arange(n_iterations):
        u = np.random.randint(0, 2, K).astype(dtype=np.uint8)
        d = np.packbits(u)

        cw_pack = p.encode_vector(d)
        b = np.unpackbits(cw_pack)
        llrs = -2 * b + 1
        llrs = llrs.astype(dtype=np.float32)
        dhat = dec.decode_vector(llrs)
        # print(d)
        # print(dhat)
        if not np.all(dhat == d):
            ud = np.unpackbits(d)
            udhat = np.unpackbits(dhat)
            print(ud)
            print(udhat)
            print(np.sum(udhat == ud) - len(ud))

        assert np.all(dhat == d)
        ctr += 1


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


def plot_capacity_rate():
    ebnos_db = np.arange(-1.0, 10., .1)
    ebnos_lin = 10 ** (ebnos_db / 10.)
    R = .5
    # r0 = 1 - np.log2(1 + np.exp(-R * ebnos_lin))
    # plt.plot(r0, ebnos_db)
    r = np.arange(0.01, 1.0, .01)
    en = -1 * (np.log((2 ** (1 - r)) - 1) / r)
    en_db = 10 * np.log10(en)

    ec = ((2 ** r) - 1) / r
    ec_db = 10. * np.log10(ec)

    plt.plot(r, en_db)
    plt.plot(r, ec_db)

    plt.show()


def search_code_weights(N, K, design_snr_db):
    f = pypolar.frozen_bits(N, K, design_snr_db)
    p = pypolar.PolarEncoder(N, f)

    numInfoWords = 2 ** K
    weights = {}
    for i in range(numInfoWords):
        b = np.binary_repr(i, K)
        u = np.array([int(l) for l in b], dtype=np.uint8)
        nbp = np.packbits(u)
        cw = p.encode_vector(nbp)
        c = np.unpackbits(cw)
        weight = np.sum(c)
        if weight in weights:
            weights[weight] += 1
        else:
            weights[weight] = 1
    weights.pop(0)
    print(weights)
    return weights


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


def q_func(x_vals):
    return .5 * sps.erfc(x_vals)


def calculate_awgn_fer_bound(Ar, R, ebnos_lin):
    pw = np.zeros(len(ebnos_lin))
    for r, a in enumerate(Ar):
        pw += a * q_func(np.sqrt(2 * R * r * ebnos_lin))
    return pw


def plot_channel_coding_bounds():
    N = 32
    ebnos_db = np.arange(-1.0, 10., .1)
    ebnos_lin = 10 ** (ebnos_db / 10.)
    design_snr_db = 0.0
    ks = np.array([4, 8, 16], dtype=int)

    for k in ks:
        R = 1. * k / N
        print(N, k, R)
        weights = search_code_weights(N, k, design_snr_db)
        dmin = np.min(weights.keys())
        Ar = np.zeros(N + 1, dtype=int)
        Ar[weights.keys()] = weights.values()
        # Ar = weights.values()
        awgn_fer = calculate_awgn_fer_bound(Ar, R, ebnos_lin)
        print(awgn_fer)
        # f = pypolar.frozen_bits(N, k, design_snr_db)
        # Gs = get_polar_encoder_matrix_systematic(N, f)
        # dmin = validate_generator_and_check_matrix(Gs, f)
        my_plot = plt.semilogy(ebnos_db, awgn_fer, ls='dashed')


        x_vals = np.sqrt(2 * R * dmin * ebnos_lin)
        q_vals = q_func(x_vals)
        pw_vals = (2 ** k - 1) * q_vals
        plt.semilogy(ebnos_db, pw_vals, c=my_plot[0].get_color(), label='K{}, R{:.2f}'.format(k, R))
    # plt.ylim((1e-10, 1))
    plt.legend()
    plt.show()


def calculate_r0_fer(codeword_len, info_len, ebn0s):
    print(ebn0s)
    rb = 1. * info_len / codeword_len
    print(rb)
    ebn0s_lin = 10. ** (ebn0s / 10.)
    print(ebn0s_lin)
    snr = rb * ebn0s_lin
    print(snr)
    r0 = 1 - np.log2(1 + np.exp(-1. * snr))
    print(r0)
    print(r0 - rb)
    fer = 2 ** (-1. * codeword_len * (r0 - rb))
    fer[np.where(r0 < rb)] = 1.
    return fer


def plot_r0_fer():
    ebn0s = np.arange(0.0, 4., .25)
    fer = calculate_r0_fer(512, 256, ebn0s)
    plt.semilogy(ebn0s, fer)
    plt.grid()
    plt.show()


def main():
    calculate_code_properties(32, 17, 0.0)

    plot_channel_coding_bounds()
    plot_capacity_rate()
    # return
    verify_frozen_bit_positions()
    verify_encode_systematic()
    verify_cpp_encoder_impl()
    return
    for n in range(5, 9):
        N = 2 ** n
        verify_cpp_decoder_impl(N, 10000)
    G = get_polar_generator_matrix(3)
    print(G)
    # matrix_row_weight(G)
    # return
    N = 2 ** 3
    # n = int(np.log2(N))
    K = 4
    eta = design_snr_to_bec_eta(2, 1.0)
    polar_capacities = calculate_bec_channel_capacities(eta, N)
    frozenBitMap = get_frozenBitMap(polar_capacities, N - K)

    u = np.random.randint(0, 2, K)
    u = np.ones(K)
    xm = encode_systematic_matrix(u, 8, frozenBitMap)
    # x = spc_most_efficient(u, N, frozenBitMap)
    # print(x)

    print('')
    print(xm)

    N = 2 ** 5
    K = N // 2

    eta = design_snr_to_bec_eta(2, 1.0)
    polar_capacities = calculate_bec_channel_capacities(eta, N)
    f = get_frozenBitPositions(polar_capacities, N - K)
    print(f)

    p = pypolar.PolarEncoder(N, f)
    u = np.random.randint(0, 2, K).astype(dtype=np.uint8)
    d = np.packbits(u)
    print(d)
    cw_pack = p.encode_vector(d)
    b = np.unpackbits(cw_pack)
    llrs = -2 * b + 1
    llrs = llrs.astype(dtype=np.float32)

    dec = pypolar.PolarDecoder(N, 1, f)
    print(dec.infoLength())
    bhat = dec.decode_vector(llrs)
    print(bhat)
    print('Polar Coder Test')
    ctr = 0
    for i in np.arange(100000):
        u = np.random.randint(0, 2, K).astype(dtype=np.uint8)
        d = np.packbits(u)

        cw_pack = p.encode_vector(d)
        b = np.unpackbits(cw_pack)
        llrs = -2 * b + 1
        llrs = llrs.astype(dtype=np.float32)
        dhat = dec.decode_vector(llrs)
        #print d
        #print dhat
        if not np.all(dhat == d):
            print(np.unpackbits(d))
            print(np.unpackbits(dhat))

        assert np.all(dhat == d)
        ctr += 1








if __name__ == '__main__':
    main()
