#!/usr/bin/env python3
import sys, os
my_dir = os.path.dirname(os.path.realpath(__file__))
# print(my_dir)

RESULTS_DIR = os.path.join(my_dir, 'results')

import numpy as np

import pybcjr

import time
import datetime
import matplotlib.pyplot as plt
import symbolmapping as sd

import pybindpolar as pypolar


def is_power_of2(n):
    h = n // 2
    if not 2 * h == n:
        return False
    if h == 1:
        return True
    elif h > 1:
        return is_power_of2(h)
    else:
        return False


def get_next_higher_power_of2(n):
    if n < 2:
        return 1
    return 2 ** (n - 1).bit_length()


def get_next_lower_power_of2(n):
    return get_next_higher_power_of2(n) // 2


def calc_vector_capacities_one_recursion(iw0):
    degraded = iw0 ** 2
    upgraded = 2 * iw0 - degraded
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


def load_polar_5g_channel_reliability_values(filename):
    m = np.genfromtxt(filename, delimiter=',')
    m = m.astype(dtype=np.int)
    z_values = m[:, 0::2]
    z_values = z_values.T.flatten()
    pos_values = m[:, 1::2].T.flatten()
    assert np.all(z_values == np.arange(len(z_values), dtype=z_values.dtype))
    assert np.all(np.sort(pos_values) == np.arange(len(pos_values), dtype=pos_values.dtype))

    # print(z_values)
    # print(pos_values)
    # print(pos_values[-20:])
    return z_values, pos_values


def get_polar_5g_positions(block_length):
    if not is_power_of2(block_length):
        raise ValueError('Block length is not a power of 2!')
    if not block_length >= 0 and block_length <= 1024:
        raise ValueError('Block length is out-of-bounds!')
    csv_file = os.path.join(my_dir, '5g_polar_code_reliability_table.csv')
    z_values, pos_values = load_polar_5g_channel_reliability_values(csv_file)
    if block_length == 1024:
        return pos_values
    sub_pos_values = np.zeros(block_length, dtype=int)
    n_found = 0
    idx = 0
    while n_found < block_length:
        if pos_values[idx] < block_length:
            sub_pos_values[n_found] = pos_values[idx]
            n_found += 1
        idx += 1

    return sub_pos_values


def get_polar_5g_frozenBitPositions(block_length, n_frozen):
    positions = get_polar_5g_positions(block_length)
    assert np.all(np.sort(positions) == np.arange(block_length, dtype=positions.dtype))
    return positions[0:n_frozen]


def polar5g_rate_match(sequence_d):
    # subblock interleaving, bit collection, bit interleaving
    # subblock interleaver pattern Table 5.4.1.1-1
    subblock_interleaver_pattern = np.array(
        [0, 1, 2, 4, 3, 5, 6, 7, 8, 16, 9, 17, 10, 18, 11, 19, 12, 20, 13, 21, 14, 22, 15, 23, 24, 25, 26, 28, 27, 29,
         30, 31, ], dtype=int)
    N = len(sequence_d)
    if not is_power_of2(N):
        raise ValueError('not a power of 2')

    y_sequence = np.zeros(N, dtype=sequence_d.dtype)
    for n in np.arange(N):
        i = int(np.floor(32 * n / N))
        pos = subblock_interleaver_pattern[i] * (N / 32) + (n % (N / 32))
        y_sequence[n] = sequence_d[pos]
    assert np.all(np.sort(y_sequence) == np.arange(N, dtype=y_sequence.dtype))
    return y_sequence


def ga_inv_phi(x):
    # cp. aff3ct library
    phi_inv_pivot = 0.6845772418
    alpha = -0.4527
    beta = 0.0218
    gamma = 0.8600

    a = 1.0 / alpha
    b = -beta / alpha
    c = 1.0 / gamma

    pass
    if x > phi_inv_pivot:
        return 4.304964539 * (1 - np.sqrt(1 + 0.9567131408 * np.log(x)))
    else:
        return (a * np.log(x) + b) ** c


def ga_phi(x):
    # cp. aff3ct library
    phi_pivot = 0.867861
    alpha = -0.4527
    beta = 0.0218
    gamma = 0.8600
    if x < phi_pivot:
        return np.exp(0.0564 * x * x - 0.48560 * x)
    else:
        return np.exp(alpha * (x ** gamma) + beta)


def gaussian_approximation(N, design_snr_db):
    # cp. aff3ct library
    S = 10 ** (design_snr_db / 10.)
    n = int(np.log2(N))
    z = np.zeros(N, dtype=np.float128)
    z[0] = 4 * S
    # print(S, z[0])
    for j in np.arange(n):
        # print(j)
        u = 2 ** (j+1)
        for t in np.arange(u // 2):
            T = z[t]
            z[t] = ga_inv_phi(1.0 - (1.0 - ga_phi(T)) ** 2)
            z[u//2 + t] = 2 * T
    return z


def compare_batthacharyya_bounds_vs_gaussian_approximation():
    info_length = 0
    N = 512
    K = N // 2
    design_snr_db = -1.
    eta = design_snr_to_bec_eta(design_snr_db, 1.0)
    polar_capacities = calculate_bec_channel_capacities(eta, N)

    ga_bers = gaussian_approximation(N, design_snr_db)
    f = get_frozenBitPositions(polar_capacities, N - K)
    f = np.sort(f)
    print(f[0:20])
    fga = get_frozenBitPositions(ga_bers, N - K)
    fga = np.sort(fga)
    print(fga[0:20])

    f_pos = get_polar_5g_frozenBitPositions(N, N // 2)
    f_mask = np.zeros(N, dtype=int)
    f_mask[f_pos] = 1

    fmax = 0
    fmin = N
    d = np.array([])
    d5g = np.array([])
    d5g2 = np.array([])
    snrs = np.arange(-1.5, 4.0, .1)
    for snr in snrs:
        eta = design_snr_to_bec_eta(snr, 1.0)
        polar_capacities = calculate_bec_channel_capacities(eta, N)
        f = get_frozenBitPositions(polar_capacities, N // 2)
        fm = np.zeros(N, dtype=int)
        fm[f] = 1

        ga_bers = gaussian_approximation(N, design_snr_db)
        fg = get_frozenBitPositions(ga_bers, N // 2)
        # fg = np.sort(fg)
        fgm = np.zeros(N, dtype=int)
        fgm[fg] = 1

        n_dev = np.sum(np.abs(fgm - fm))
        d = np.append(d, n_dev)
        d5g = np.append(d5g, np.sum(np.abs(fgm - f_mask)))
        d5g2 = np.append(d5g2, np.sum(np.abs(fm - f_mask)))
        # print(n_dev)
        fmax = np.maximum(fmax, n_dev)
        fmin = np.minimum(fmin, n_dev)
        if .75 < snr < .85:
            plt.plot(np.abs(np.sort(f) - np.sort(fg)))
            plt.plot(np.abs(np.sort(f) - np.sort(f_pos)))
            # plt.plot(fm)
            # plt.plot(f_mask)
            plt.show()
        # plt.plot(np.abs(f_mask - fm))
    print('max {} and min {}'.format(fmax, fmin))
    plt.plot(snrs, d)
    plt.plot(snrs, d5g)
    plt.plot(snrs, d5g2)
    plt.show()


def main():
    np.set_printoptions(precision=2, linewidth=150)
    compare_batthacharyya_bounds_vs_gaussian_approximation()

    return

    z_values, pos_values = load_polar_5g_channel_reliability_values('./5g_polar_code_reliability_table.csv')

    info_length = 0
    K = 1024
    N = 1024
    design_snr_db = -1.
    eta = design_snr_to_bec_eta(design_snr_db, 1.0)
    polar_capacities = calculate_bec_channel_capacities(eta, N)

    ga_bers = gaussian_approximation(N, design_snr_db)

    f = get_frozenBitPositions(polar_capacities, N)
    print(f[0:20])

    print(np.sum(f == pos_values))

    # for i in np.arange(70, dtype=int):
    #     print(i, get_next_higher_power_of2(i), get_next_lower_power_of2(i), is_power_of2(i))
    d_sequence = np.arange(64, dtype=int)
    polar5g_rate_match(d_sequence)
    f_pos = get_polar_5g_frozenBitPositions(N, N // 2)
    f_mask = np.zeros(N, dtype=int)
    f_mask[f_pos] = 1

    fmax = 0
    fmin = N
    d = np.array([])
    snrs = np.arange(-1.5, 4.0, .1)
    for snr in snrs:
        eta = design_snr_to_bec_eta(snr, 1.0)
        polar_capacities = calculate_bec_channel_capacities(eta, N)
        f = get_frozenBitPositions(polar_capacities, N // 2)
        fm = np.zeros(N, dtype=int)
        fm[f] = 1
        # print(snr)
        # print(f[-10:])
        # print(pos_values[-10:])
        n_dev = np.sum(np.abs(f_mask - fm))
        d = np.append(d, n_dev)
        print(n_dev)
        fmax = np.maximum(fmax, n_dev)
        fmin = np.minimum(fmin, n_dev)
        # plt.plot(np.abs(f_mask - fm))
    print('max {} and min {}'.format(fmax, fmin))
    plt.plot(snrs, d)
    plt.show()



if __name__ == '__main__':
    main()
