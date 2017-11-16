#!/usr/bin/env python3

from __future__ import print_function, division
import numpy as np


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


def main():
    G = get_polar_generator_matrix(3)
    print(G)
    capacities = calculate_bec_channel_capacities(.5, 8)
    f = get_frozenBitPositions(capacities, 4)
    f = np.sort(f)
    ip = np.setdiff1d(np.arange(8), f)
    print(ip)
    print(f)

    ui = np.random.randint(0, 2, 4)
    print(ui)

    u = np.zeros(8)
    u[ip] = ui
    xs = u.dot(G) % 2
    xs[f] = 0
    xs = xs.dot(G) % 2

    print(xs)


if __name__ == '__main__':
    main()
