#!/usr/bin/env python3

import numpy as np
import matplotlib.pyplot as plt

import sys
sys.path.append('./build/lib')
sys.path.append('./build/lib.linux-x86_64-2.7')
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
    s = 10. ** (coderate * design_snr / 10.)
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
    print(frozenBitMap)

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


def main():
    verify_encode_systematic()
    G = get_polar_generator_matrix(3)
    print(G)
    matrix_row_weight(G)
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
    x = spc_most_efficient(u, N, frozenBitMap)
    print(x)

    print('')
    print(xm)

    N = 2 ** 6
    n = int(np.log2(N))
    rv = get_bitreversed_vector(n)
    K = N // 2
    eta = design_snr_to_bec_eta(2, 1.0)
    polar_capacities = calculate_bec_channel_capacities(eta, N)
    f = get_frozenBitPositions(polar_capacities, N - K)
    frozenBitMap = get_frozenBitMap(polar_capacities, N - K)

    p = pypolar.EncoderPacked(N, f)
    pu = pypolar.EncoderUnpacked(N, f)


    d = np.random.randint(0, 256, K / 8).astype(dtype=np.uint8)
    print(d)
    u = np.unpackbits(d)
    print(u)
    print()
    xm = encode_systematic_matrix(u, N, frozenBitMap)
    print(np.packbits(xm))
    print(xm)

    # print('unpacked')
    # pu.setInformation(u)
    # pu.encode()
    # c = pu.getEncodedData()
    # print(c)
    print('unpacked')
    cu = pu.encode_vector(d)
    print(cu)
    # p.setInformation(d)
    # p.encode()
    # cw = p.getEncodedData()
    print('packed')
    cw = p.encode_vector(d)

    cwu = np.unpackbits(cw)
    print(np.packbits(xm))
    print(cu)
    print(cw)

    print(xm)

    print(cwu)

    print(np.sum(cwu), np.sum(xm))






if __name__ == '__main__':
    main()
