#!/usr/bin/env python3
# -*- coding: utf-8 -*-
#
# Copyright 2020 Johannes Demel.
#
# SPDX-License-Identifier: GPL-3.0-or-later
#

import numpy as np


def calc_bb_or(iw0):
    degraded = 2 * iw0
    l2 = np.log(2.)
    upgraded = iw0 + l2 + np.log(1 - np.exp(iw0 - l2))

    iw1 = np.empty(2 * len(iw0), dtype=degraded.dtype)
    iw1[0::2] = degraded
    iw1[1::2] = upgraded
    return iw1


def calculate_ln_bhattacharyya_bounds(initial_channel, block_power):
    # compare [0, Arikan] eq. 6
    # this version is ~ 180 times faster than the loop version with 2**22 synthetic channels
    iw = np.array([np.log(initial_channel), ], dtype=np.float128)
    for i in range(block_power):
        iw = calc_bb_or(iw)
    r = 1. - np.exp(iw)
    return r[::-1]


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
    iw = np.array([initial_channel, ], dtype=np.float128)
    for i in range(block_power):
        iw = calc_vector_capacities_one_recursion(iw)
    return iw


def calculate_bec_channel_capacities(eta, block_size):
    # compare [0, Arikan] eq. 6
    iw = 1 - eta  # holds for BEC as stated in paper
    lw = int(np.log2(block_size))
    return calculate_bec_channel_capacities_vector(iw, lw)


def design_snr_to_bec_eta(design_snr, coderate=1.0):
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


def frozen_indices_to_map(frozen_indices, N):
    frozenBitMap = np.ones(N, dtype=int) * -1
    frozenBitMap[frozen_indices] = 0
    return frozenBitMap


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


def get_polar_generator_matrix(n):
    F = Fk = np.array([[1, 0], [1, 1]]).astype(int)
    for i in range(n - 1):
        F = np.kron(F, Fk)
    # print F
    return F


def get_polar_encoder_matrix_systematic(N, f):
    n = int(np.log2(N))
    G = get_polar_generator_matrix(n)
    ip = np.delete(np.arange(N), f)
    Gi = G[ip, :]
    I = np.identity(N, dtype=np.uint8)
    I[f, :] = 0
    gm = Gi.dot(I) #% 2
    return (gm.dot(G) % 2).astype(np.uint8)


    print(poly_bits[::-1])
    crc_matrix = np.zeros((info_len, crc_len)).astype(np.uint8)
    poly_bits = poly_bits[::-1]
    poly_bits = poly_bits[1:]
    crc_matrix[-1] = poly_bits
    print(crc_matrix)
    for i in np.arange(info_len - 1, 0, -1):
        print(i)
        # for j
    pass


def get_info_indices(frozen_indices, N):
    return np.setdiff1d(np.arange(N, dtype=frozen_indices.dtype),
                        frozen_indices)


def get_expanding_matrix(frozen_indices, N):
    m = np.identity(N)
    info_indices = get_info_indices(frozen_indices, N)
    e = m[info_indices, :]
    return e


def calculate_involution(G, E):
    bpi = np.dot(E, np.dot(G, E.T) % 2) % 2
    return np.dot(bpi, bpi) % 2


def plot_approx():
    import matplotlib.pyplot as plt
    a = np.log(2.)
    s = 0.001
    b0 = np.arange(-5., a, s)

    y0 = np.log(np.exp(a) - np.exp(b0))
    plt.plot(b0, y0)

    plt.show()

    # N = int(2 ** n)
    # capacities = calculate_bec_channel_capacities(eta, N)

    # caps = calculate_ln_bhattacharyya_bounds(eta, n)

    # plt.plot(capacities)
    # plt.plot(caps)
    # plt.show()


def func_phi4_c(t):
    a = 0.1910
    b = 0.7420
    c = 9.2254
    if 0 < t <= a:
        return np.exp(0.1047 * (t ** 2) - 0.4992 * t)
    elif a < t <= b:
        return 0.9981 * np.exp(0.05315 * (t ** 2) - 0.4795 * t)
    elif b < t <= c:
        return np.exp(-0.4527 * (t ** 0.86) + 0.0218)
    else:
        return np.exp(-0.2832 * t - 0.4254)


def func_phi4(t):
    f = np.vectorize(func_phi4_c)
    return f(t)


def func_phi2_c(t):
    a = 7.0633
    if a < t:
        return np.exp(-0.2944 * t - 0.3169)
    else:
        return np.exp(0.0116 * (t ** 2) - 0.4212 * t)


def func_phi2(t):
    f = np.vectorize(func_phi2_c)
    return f(t)


def func_phi_c(t):
    alpha = -0.4527
    beta = 0.0218
    gamma = 0.8600
    phi_pivot = 0.867861
    if t < phi_pivot:
        return np.exp(0.0564 * (t ** 2) - 0.48560 * t)
    else:
        return np.exp(alpha * (t ** gamma) + beta)


def func_phi(t):
    f = np.vectorize(func_phi_c)
    return f(t)


def func_inv_phi_c(t):
    alpha = -0.4527
    beta = 0.0218
    gamma = 0.8600
    a =  1.0  / alpha
    b = -beta / alpha
    c =  1.0  / gamma

    phi_inv_pivot = 0.6845772418

    if t > phi_inv_pivot:
        return 4.304964539 * (1 - np.sqrt(1 + 0.9567131408 * np.log(t)))
    else:
        tt = np.log(t)
        if np.isinf(tt):
            print('log div {}'.format(tt))
            tt = np.sign(tt) * 1e15
        return (a * tt + b) ** c


def func_inv_phi(t):
    f = np.vectorize(func_inv_phi_c)
    return f(t)


def calculate_ga(N, sigma):
    print('GA ({}) {}'.format(N, sigma))
    m = int(np.log2(N))
    z = np.ones(N, dtype=np.float64)
    z *= 2.0 / (sigma ** 2)

    alpha = -0.4527
    gamma = 0.8600

    for l in range(1, m + 1):
        print(l)
        o1 = 2 ** (m - l + 1)
        o2 = 2 ** (m - l)
        for t in range(2 ** (l - 1)):
            T = z[t * o1]
            z[t * o1] = func_inv_phi_c(1. - (1. - func_phi_c(T)) ** 2)
            if np.isinf(z[t * o1]):
                print('HUGE_VAL {}'.format(z[t * o1]))
                z[t * o1] = T + np.log(2.) / (alpha * gamma)
            z[t * o1 + o2] = 2. * T
    return z


def plot_phi():
    import matplotlib.pyplot as plt

    t = np.arange(0, 20, 0.001)
    p0 = func_phi(t)
    p2 = func_phi2(t)
    p4 = func_phi4(t)
    msep2 = np.linalg.norm(p2 - p0)
    msep4 = np.linalg.norm(p4 - p0)
    msep21 = np.sum(np.abs(p2 - p0) ** 2) / len(p0)
    msep41 = np.sum(np.abs(p4 - p0) ** 2) / len(p0)
    print(msep2, msep4)
    print(msep21, msep41)

    plt.plot(t, p0)
    plt.plot(t, p2)
    plt.plot(t, p4)
    # plt.plot(t, ip)
    plt.show()


VALID_2BIT_FROZEN_BIT_POSITION_PATTERN = [
    [0, 0],
    [0, 1],
    [1, 1],
]

def get_valid_frozen_bit_position_patterns(block_power):
    '''
    2bit: 3 valid
    4bit: 6 valid
    8bit: 21 valid (corresponds to maximum AVX float patterns to implement)
    16bit: 230 valid (corresponds to maximum AVX512 float patterns to implement)
    32bit: 26795 valid ((corresponds to maximum AVX2 int8_t patterns to implement))
    '''
    print(f'power={block_power} -> size={2 ** block_power}')
    if block_power == 0:
        return [[0, ], [1, ]]
    elif block_power == 1:
        return VALID_2BIT_FROZEN_BIT_POSITION_PATTERN
    else:
        pattern = np.copy(VALID_2BIT_FROZEN_BIT_POSITION_PATTERN)
        for n in range(1, block_power):
            nextpattern = []
            for i, p in enumerate(pattern):
                for fp in pattern[i:]:
                    nextpattern.append(np.concatenate((p, fp)))
            pattern = np.array(nextpattern)
        return pattern

## There's a total of 16 theoretical options. But domination contiguity restricts mosts.
VALID_4BIT_FROZEN_BIT_POSITION_PATTERN = [
    [0, 0, 0, 0], # Rate-0
    [0, 0, 0, 1], # Repetition
    [0, 0, 1, 1], # ZeroOne (DoubleRep)
    [0, 1, 0, 1], #
    [0, 1, 1, 1], # SPC
    [1, 1, 1, 1], # Rate-1
]

def test_domination_contiguity(block_power):
    N = int(2 ** block_power)
    G = get_polar_generator_matrix(block_power)
    print(G)




def main():
    pattern4 = get_valid_frozen_bit_position_patterns(2)
    for i, p in enumerate(pattern4):
        print(i, p)
    pattern8 = get_valid_frozen_bit_position_patterns(3)
    for i, p in enumerate(pattern8):
        print(i, p)
    pattern16 = get_valid_frozen_bit_position_patterns(4)
    for i, p in enumerate(pattern16):
        print(i, p)
    # pattern32 = get_valid_frozen_bit_position_patterns(5)
    # for i, p in enumerate(pattern32):
    #     print(i, p)
    return
    n = 10
    N = int(2 ** n)
    G = get_polar_generator_matrix(n)
    print(G)
    snr = -1.
    eta = design_snr_to_bec_eta(snr, 1.0)

    # capacities = calculate_bec_channel_capacities(eta, N)
    capacities = calculate_ln_bhattacharyya_bounds(eta, n)
    capacities = calculate_ga(N, np.sqrt(10 ** (-1. / 10.)))


    # for k in (.75, .5, .25, .125, 1. / 16.):
    errs = 0
    for K in np.arange(N, -1, -1):
        # K = int(N * k)

        f = get_frozenBitPositions(capacities, N - K)
        f = np.sort(f)
        # ip = get_info_indices(f, N)
        # print(ip)
        # print(f)

        E = get_expanding_matrix(f, N)
        r = calculate_involution(G, E)
        # print(r)
        pc_correct = np.all(r == np.identity(N - len(f)))
        errs += not pc_correct
        print('Polar Code ({}, {}) -> {}'.format(N, K, pc_correct))

        # print(np.all(r == np.identity(N - len(f))))
    print(capacities)
    print(np.unique(capacities).size)
    print(errs)
    f0 = get_frozenBitPositions(capacities, N - 45)
    f0 = np.sort(f0)
    ip = get_info_indices(f0, N)
    # print(capacities[ip])
    # print(np.unique(capacities[ip]))

    f1 = get_frozenBitPositions(capacities, N - 43)
    f1 = np.sort(f1)
    # ip = get_info_indices(f, N)

    # print(f0)
    # print(f1)
    # print(np.setdiff1d(f1, f0))

    # plt.plot(capacities[200:])
    # plt.show()


if __name__ == '__main__':
    main()
