#!/usr/bin/env python3
# -*- coding: utf-8 -*-
#
# Copyright 2020 Johannes Demel.
#
# SPDX-License-Identifier: GPL-3.0-or-later
#

import pypolar
from channel_construction import ChannelConstructorBhattacharyyaBounds, ChannelConstructorGaussianApproximationDai
from frozen_bit_positions import FrozenBitPositions5G
from polar_code_tools import get_info_indices, get_expanding_matrix, calculate_ga
from polar_code_tools import design_snr_to_bec_eta, calculate_bec_channel_capacities, get_frozenBitMap, get_frozenBitPositions, get_polar_generator_matrix, get_polar_encoder_matrix_systematic, frozen_indices_to_map
import numpy as np
import unittest
import os

if os.path.dirname(__file__) != os.getcwd():
    os.chdir(os.path.dirname(__file__))


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


def get_diff_positions(block_length, pos):
    return np.setdiff1d(np.arange(block_length), pos).astype(pos.dtype)


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


SUBBLOCK_INTERLEAVER_PATTERN = np.array([0, 1, 2, 4, 3, 5, 6, 7,
                                         8, 16, 9, 17, 10, 18, 11, 19,
                                         12, 20, 13, 21, 14, 22, 15, 23,
                                         24, 25, 26, 28, 27, 29, 30, 31], dtype=int)


def generate_5g_polar_interleaver_pattern(codeword_len):
    # see 38.212 Sec. 5.4.1.1
    # The output is J with J(n) as the nth element value.
    # interleaver_pattern = np.zeros(codeword_len, dtype=int)
    # for n in range(codeword_len):
    #     i = int(np.floor(32 * n / codeword_len))
    #     interleaver_pattern[n] = SUBBLOCK_INTERLEAVER_PATTERN[i] * \
    #         (codeword_len // 32) + (n % (codeword_len // 32))

    p = np.arange(codeword_len, dtype=int)
    mp = np.reshape(p, (32, -1))
    return mp[SUBBLOCK_INTERLEAVER_PATTERN].flatten()


def generate_5g_polar_shortening_indices(mother_codeword_len, codeword_len):
    interleaver_pattern = generate_5g_polar_interleaver_pattern(
        mother_codeword_len)
    return interleaver_pattern[codeword_len:]


def generate_5g_polar_puncturing_indices(mother_codeword_len, codeword_len):
    interleaver_pattern = generate_5g_polar_interleaver_pattern(
        mother_codeword_len)
    punc_pattern = interleaver_pattern[0: mother_codeword_len - codeword_len]
    if codeword_len >= (3 * mother_codeword_len / 4):
        num_frontpositions = int(
            np.ceil(3 * mother_codeword_len / 4 - codeword_len / 2))
    else:
        num_frontpositions = int(
            np.ceil(9 * mother_codeword_len / 16 - codeword_len / 4))
    frontpositions = np.arange(num_frontpositions)
    return np.union1d(punc_pattern, frontpositions)


def generate_5g_polar_ratematching_indices(mother_codeword_len, codeword_len, info_len):
    if codeword_len < mother_codeword_len:
        if info_len / codeword_len <= (7. / 16.):
            print('puncturing')
            return generate_5g_polar_puncturing_indices(mother_codeword_len, codeword_len)
        else:
            print('shortening')
            return generate_5g_polar_shortening_indices(mother_codeword_len, codeword_len)
    print('match')
    return np.array([], dtype=int)


def calculate_5g_polar_mother_code_length(codeword_len, info_len, max_codeword_power=10):
    # cf. TS 38.212 Sec. 5.3.1
    codeword_power1 = int(np.ceil(np.log2(codeword_len)))
    if codeword_len <= (9. / 8.) * (2 ** (np.ceil(np.log2(codeword_len)) - 1)) and \
            info_len / codeword_len < (9. / 16.):
        codeword_power1 -= 1

    rate_min = 1. / 8.
    codeword_power2 = int(np.ceil(np.log2(info_len / rate_min)))
    min_codeword_power = 5
    codeword_power = np.maximum(np.min(
        [codeword_power1, codeword_power2, max_codeword_power]), min_codeword_power)
    mother_codeword_len = int(2 ** codeword_power)
    return mother_codeword_len


def main():
    np.set_printoptions(linewidth=150, precision=3)
    a = np.array([-0.742-0.216j, -0.586-0.325j, -0.077 -
                  0.331j, +0.604-0.537j, ], dtype=np.complex64)
    b = np.array([+0.800-0.569j, +0.621-0.834j, -0.801 +
                  0.775j, +0.344-0.703j, ], dtype=np.complex64)
    print(a)
    print(b)
    print(a / b)

    r = a.real * b.real + 1.j * a.imag * b.real - \
        1.j * a.real * b.imag + a.imag * b.imag
    r /= (b.real ** 2 + b.imag ** 2)
    print(r)

    assert SUBBLOCK_INTERLEAVER_PATTERN.dtype == int
    print(SUBBLOCK_INTERLEAVER_PATTERN)
    assert SUBBLOCK_INTERLEAVER_PATTERN.size == 32
    assert np.unique(SUBBLOCK_INTERLEAVER_PATTERN).size == 32
    gen = pypolar.get_frozen_bit_generator('5G', 32, 0, 0)
    # print(gen.frozen_bit_positions())
    frozens = FrozenBitPositions5G(32, 16)
    frozens.frozen_bit_positions()
    rels = frozens._reliabilities
    print(rels[np.where(rels < 32)])
    # print(frozens._reliabilities)
    intl_pattern = generate_5g_polar_interleaver_pattern(32)
    print(intl_pattern)

    print(generate_5g_polar_shortening_indices(32, 24))

    max_rate = 0.0
    min_rate = 1.0
    dl_codeword_lens = 108 * (2 ** np.arange(5))
    for e in dl_codeword_lens:
        for k in range(12 + 24, min(e, 164)):
            mother_codeword_len = calculate_5g_polar_mother_code_length(e, k, 9)
            max_rate = np.maximum(max_rate, k / mother_codeword_len)
            min_rate = np.minimum(min_rate, k / mother_codeword_len)
            # print(f'E={e}\tK={k}\tN={mother_codeword_len}')
    print(f'max={max_rate:.3f}\tmin={min_rate:.3f}')
    return
    for c in (128, 64, 32, ):
        code_rels = rels[np.where(rels < c)]
        for e in range(12, c + 1):
            k = 12
            indices = generate_5g_polar_ratematching_indices(c, e, k)
            assert indices.dtype == int
            print(f'N={c}\tE={e}\tK={k}')
            # print(indices)
            info_candidates = np.setdiff1d(
                code_rels, indices, assume_unique=True)
            info_positions = info_candidates[-k:]
            frozen_bit_positions = np.setdiff1d(
                code_rels, info_positions, assume_unique=True)
            print(frozen_bit_positions)


if __name__ == '__main__':
    main()
    # unittest.main(failfast=False)
