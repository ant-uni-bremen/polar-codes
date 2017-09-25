#!/usr/bin/env python
# The first few lines make the locally build module available in this python script!
from __future__ import print_function, division
import sys, os
my_dir = os.path.dirname(os.path.realpath(__file__))
print(my_dir)

RESULTS_DIR = os.path.join(my_dir, 'results')

import numpy as np
import time
import datetime
import matplotlib.pyplot as plt
import pysymboldemapper as sd
import channel_simulation as cs
import pypolar

from polar_5g_parameters import get_polar_5g_frozenBitPositions, design_snr_to_bec_eta, calculate_bec_channel_capacities, get_frozenBitPositions


def generate_filename(file_prefix, constellation_order, inv_coderate, scl_list_size, info_length, ebn0):
    ts = '{:%Y-%m-%d-%H:%M:%S}'.format(datetime.datetime.now())
    result_filename = '{:s}_ConstOrder{}_invRate{}_ListSize{}_InfoLength{}_EBN{:.2f}_{:s}'.format(file_prefix, constellation_order, inv_coderate, scl_list_size, info_length, ebn0, ts)
    return os.path.join(RESULTS_DIR, result_filename)


def save_results(filename, constellation_order, inv_coderate, info_length, ebn0, n_iterations, scl_list_size, python_ns,
                 c_ns, results):
    d = {
        'constellation_order': constellation_order,
        'invCoderate': inv_coderate,
        'info_length': info_length,
        'scl_list_size': scl_list_size,
        'EbN0': ebn0,
        'iterations': n_iterations,
        'avgDecoderDurationNs': {'python': python_ns, 'c': c_ns},
        'results': results
    }
    np.save(filename, d)
    return d


def calculate_ber(results, info_length):
    n_errors = (len(results) * info_length) - np.sum(results)
    return n_errors / (len(results) * info_length)


def calculate_fer(results, info_length):
    n_correct_frames = np.sum(results == info_length)
    return 1. * (len(results) - n_correct_frames) / len(results)


def print_time_statistics(datapoint_sim_duration, python_ns, c_ns, n_iterations):
    datapoint_sim_duration_per_iteration = int(datapoint_sim_duration * 1e9 / n_iterations)
    python_ns /= n_iterations
    c_ns /= n_iterations
    print('Simulation Duration: {:6.2f}s,\tper iteration: {:6.2f}us'.format(datapoint_sim_duration, datapoint_sim_duration_per_iteration / 1e3))
    print('Decoder      Python: {:6.2f}us,\tC: {:6.2f}us,\tInterface: {:6.2f}us'.format(python_ns / 1e3, c_ns / 1e3, (python_ns - c_ns) / 1e3))


def lte_subblock_interleaver_indices(block_indices):
    # subblock interleaver permutation pattern. cf. TS36.212 Table 5.1.4-4
    permutation_pattern = np.array([0, 16, 8, 24, 4, 20, 12, 28, 2, 18, 10, 26, 6, 22, 14, 30, 1, 17, 9, 25, 5, 21, 13, 29, 3, 19, 11, 27, 7, 23, 15, 31], dtype=np.int32)
    # assert subblock_index_matrix.shape[0] == 3  # This assures that matrix dimensions are as expected!
    subblock_len = len(block_indices)
    row_len = int(np.ceil(subblock_len / 32))
    dummy_len = 32 * row_len - subblock_len

    # prepend NaNs
    subblock = np.hstack((np.full((dummy_len, ), np.NaN), block_indices))

    subblock_interleaver0 = np.arange(32 * row_len)
    subblock_interleaver0 = np.reshape(subblock_interleaver0, (-1, 32))
    subblock_interleaver0 = subblock_interleaver0[:, permutation_pattern]
    subblock_interleaver0 = subblock_interleaver0.T.flatten()
    indices = subblock[subblock_interleaver0]
    indices = indices[np.where(np.invert(np.isnan(indices)))]
    indices = indices.astype(int)
    assert np.all(np.sort(indices) == np.arange(len(indices), dtype=indices.dtype))
    return indices


def validate_interleaver_padding_combo(bit_interleaver, N):
    codeword_len = len(bit_interleaver)
    bits = np.arange(N)
    tx = np.concatenate((bits, np.zeros(codeword_len - N, dtype=bits.dtype)))
    tx = tx[bit_interleaver]

    llrs = np.zeros(codeword_len, dtype=tx.dtype)
    llrs[bit_interleaver] = tx
    llrs = llrs[0:N]
    assert np.all(bits == llrs)


def simulate_awgn_channel(tx, modulation_constellation, demapper, sigma):
    tx_symbols = sd.map_to_constellation(tx, modulation_constellation)
    noise = cs.get_complex_noise_vector(len(tx_symbols), sigma)
    rx = tx_symbols + noise
    rx_llrs = demapper.demap_symbols(rx, modulation_constellation, 1. / (sigma ** 2))
    return rx_llrs


def simulate_awgn_polar_code_err_frames(constellation_order, inv_coderate, info_length, ebn0, max_frame_errs=8,
                                        scl_list_size=8):
    filename = generate_filename('PolarCodeAWGN', constellation_order, inv_coderate, scl_list_size, info_length, ebn0)

    K = info_length
    N = info_length * inv_coderate

    if N <= 1024:
        f = get_polar_5g_frozenBitPositions(N, N - K)
    else:
        eta = design_snr_to_bec_eta(0.0, 1.0)
        polar_capacities = calculate_bec_channel_capacities(eta, N)
        f = get_frozenBitPositions(polar_capacities, N - K)
    encoder = pypolar.PolarEncoder(N, f)
    decoder = pypolar.PolarDecoder(N, scl_list_size, f)

    sigma = cs.ebn0_to_sigma(ebn0, 1. / inv_coderate, constellation_order)
    print('EbN0 {:.2f}dB -> SNR {:.2f}dB'.format(ebn0, 10. * np.log10(1. / sigma ** 2)))

    codeword_len = int(np.ceil(1. * N / constellation_order) * constellation_order)
    bit_interleaver = lte_subblock_interleaver_indices(np.arange(codeword_len))
    validate_interleaver_padding_combo(bit_interleaver, N)

    mod_constell, bit_representation = sd.generate_gray_constellation(constellation_order)
    demapper = sd.pysymboldemapper()

    python_ns = 0
    c_ns = 0
    err_frames = 0
    n_iterations = 0
    max_iterations = max_frame_errs * (2 ** 10)
    results = np.zeros(max_iterations)
    sim_start_time = time.time()
    while err_frames < max_frame_errs and n_iterations < max_iterations:
        bits = np.random.randint(0, 2, info_length).astype(np.int32)
        d = np.packbits(bits)
        encoded = encoder.encode_vector(d)

        btx = np.unpackbits(encoded)
        tx = np.concatenate((btx, np.zeros(codeword_len - N, dtype=btx.dtype)))
        tx = tx[bit_interleaver]

        rx_llrs = simulate_awgn_channel(tx, mod_constell, demapper, sigma).astype(dtype=np.float32)

        llrs = np.zeros(codeword_len, dtype=rx_llrs.dtype)
        llrs[bit_interleaver] = rx_llrs
        llrs = llrs[0:N]
        # llrs = llrs.astype(dtype=np.float32)

        st = time.time()
        up = decoder.decode_vector(llrs)
        python_ns += int((time.time() - st) * 1e9)
        u = np.unpackbits(up)

        frame_pattern = bits == u
        n_correct = np.sum(frame_pattern)
        results[n_iterations] = n_correct

        n_iterations += 1
        if not n_correct == info_length:
            err_frames += 1
        fer = 1. * err_frames / n_iterations
        if n_iterations % (2 ** 13) == 0:
            sim_perc = 100. * n_iterations / max_iterations
            # fer = calculate_fer(results[0:n_iterations], info_length)
            print('simulating {}/{} frames time {:.2f}s {:.2f}% {}\tFER={:8.2e}'.format(err_frames, max_frame_errs,  time.time() - sim_start_time, sim_perc, n_iterations, fer))
        if n_iterations > 8 * max_frame_errs and fer < 1e-4:
            print('stop simulation for data point')
            break
    datapoint_sim_duration = time.time() - sim_start_time
    results = results[0:n_iterations]
    print_time_statistics(datapoint_sim_duration, python_ns, c_ns, n_iterations)

    ber = calculate_ber(results, info_length)
    fer = calculate_fer(results, info_length)

    print('#frames {} with BER {:8.2e} and FER {:8.2e}'.format(len(results), ber, fer))
    return save_results(filename, constellation_order, inv_coderate, info_length, ebn0, n_iterations, scl_list_size,
                        python_ns, c_ns, results)


def polar_simulation_run():
    info_len_list = np.array([32, 64, 128, 256, 512, 1024, ], dtype=int)
    # k_values = np.array([32, ], dtype=int)
    print(info_len_list)
    inv_coderate = 2
    ebn0_list = np.arange(-4.0, 8.25, .25)  # overall
    # ebn0_list = np.arange(-7.0, -3.25, .25)  # BPSK
    # ebn0_list = np.arange(-2.0, 4.25, .25)  # QPSK
    # ebn0_list = np.arange(4.0, 8.25, .25)  # 8-PSK
    fer_list = np.ones(len(ebn0_list))
    ber_list = np.ones(len(ebn0_list))
    num_err_frames = 2 ** 9
    scl_list_size = 1

    plt.ion()
    plt.semilogy(ebn0_list, fer_list)
    my_figure = plt.gcf()
    plt.draw()
    plt.grid()

    for constellation_order in range(2, 3):
        for info_length in info_len_list:
            for i, ebn0 in enumerate(ebn0_list):
                print('\nConstellationOrder {}, invCoderate {}, SCL List size {}, info length {}, EbN0 {:.2f}'.format(constellation_order, inv_coderate, scl_list_size, info_length, ebn0))
                res = simulate_awgn_polar_code_err_frames(constellation_order, inv_coderate, info_length, ebn0,
                                                          num_err_frames, scl_list_size)
                results = res['results']
                fer_list[i] = calculate_fer(results, info_length)
                ber_list[i] = calculate_ber(results, info_length)

                my_figure.clear()
                plt.semilogy(ebn0_list, fer_list)
                plt.semilogy(ebn0_list, ber_list)
                plt.xlabel('Eb/N0 [dB]')
                plt.title('Polar Code with info length {}, constellation order {}'.format(info_length, constellation_order))
                plt.grid()
                plt.draw()
    plt.ioff()
    plt.show()


def main():
    np.set_printoptions(precision=2, linewidth=150)
    polar_simulation_run()


if __name__ == '__main__':
    main()
