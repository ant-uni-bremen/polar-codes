#!/usr/bin/env python
# The first few lines make the locally build module available in this python script!
from __future__ import print_function, division
import sys, os
my_dir = os.path.dirname(os.path.realpath(__file__))
print(my_dir)

RESULTS_DIR = os.path.join(my_dir, 'results')

import numpy as np
import pybcjr
import lte_functions
import time
import datetime
import matplotlib.pyplot as plt
import pysymboldemapper as sd
import channel_simulation as cs
import pypolar


def generate_filename(file_prefix, constellation_order, inv_coderate, constraint_length, info_length, ebn0):
    ts = '{:%Y-%m-%d-%H:%M:%S}'.format(datetime.datetime.now())
    result_filename = '{:s}_ConstOrder{}_invRate{}_ConstraintLen{}_InfoLength{}_EBN{:.2f}_{:s}'.format(file_prefix, constellation_order, inv_coderate, constraint_length, info_length, ebn0, ts)
    return os.path.join(RESULTS_DIR, result_filename)


def save_results(filename, constellation_order, inv_coderate, constraint_length, info_length, ebn0, n_iterations, max_turbo_iterations, decoder_type, python_ns, c_ns, results):
    d = {
        'constellation_order': constellation_order,
        'invCoderate': inv_coderate,
        'contraint_length': constraint_length,
        'info_length': info_length,
        'max_turbo_iterations': max_turbo_iterations,
        'decoder_type': decoder_type,
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
    return 1. * (len(results) - len(np.where(results == info_length)[0].flatten())) / len(results)


def print_time_statistics(datapoint_sim_duration, python_ns, c_ns, n_iterations):
    datapoint_sim_duration_per_iteration = int(datapoint_sim_duration * 1e9 / n_iterations)
    python_ns /= n_iterations
    c_ns /= n_iterations
    print('Simulation Duration: {:6.2f}s,\tper iteration: {:6.2f}us'.format(datapoint_sim_duration, datapoint_sim_duration_per_iteration / 1e3))
    print('Decoder      Python: {:6.2f}us,\tC: {:6.2f}us,\tInterface: {:6.2f}us'.format(python_ns / 1e3, c_ns / 1e3, (python_ns - c_ns) / 1e3))


def simulate_awgn_channel(tx, modulation_constellation, demapper, sigma):
    tx_symbols = sd.map_to_constellation(tx, modulation_constellation)
    noise = cs.get_complex_noise_vector(len(tx_symbols), sigma)
    rx = tx_symbols + noise
    rx_llrs = demapper.demap_symbols(rx, modulation_constellation, 1. / (sigma ** 2))
    return rx_llrs


def simulate_awgn_conv_code_err_frames(constellation_order, inv_coderate, constraint_length, info_length, ebn0, max_frame_errs=8, max_turbo_iterations=8, decoder_type=2):
    filename = generate_filename('ConvolutionalCodeAWGN', constellation_order, inv_coderate, constraint_length, info_length, ebn0)
    code_polynomial = get_code_polynomial(inv_coderate, constraint_length)
    coder = pybcjr.pybcjr(code_polynomial, 0, constraint_length, inv_coderate, 1, 8000, False)
    app_llrs = np.zeros(info_length, dtype=np.float64)

    sigma = cs.ebn0_to_sigma(ebn0, 1. / inv_coderate, constellation_order)

    codeword_len = coder.codeword_len(info_length)
    # subblock_indices = np.reshape(np.arange((info_length + constraint_length) * 3), (-1, 3)).T

    # rate_match_indices = lte_functions.get_lte_rate_match_indices(subblock_indices, codeword_len, ant_compat=True)

    mod_constell, bit_representation = sd.generate_gray_constellation(constellation_order)
    demapper = sd.pysymboldemapper()

    results = np.array([])
    python_ns = 0
    c_ns = 0
    err_frames = 0
    n_iterations = 0
    max_iterations = max_frame_errs * (2 ** 10)
    sim_start_time = time.time()
    while err_frames < max_frame_errs and n_iterations < max_iterations:
        n_iterations += 1
        bits = np.random.randint(0, 2, info_length).astype(np.int32)
        encoded = coder.encode(bits)

        tx = encoded

        rx_llrs = simulate_awgn_channel(tx, mod_constell, demapper, sigma)

        # llrs = np.zeros((info_length + constraint_length) * 3)
        llrs = rx_llrs

        st = time.time()
        info_llrs, code_llrs = coder.decode(llrs, app_llrs)
        python_ns += int((time.time() - st) * 1e9)
        c_ns += coder.decoder_duration()
        u = (info_llrs < 0.0).astype(np.int32)

        frame_pattern = bits == u
        n_correct = np.sum(frame_pattern)
        results = np.append(results, n_correct)
        if not n_correct == info_length:
            err_frames += 1
        if n_iterations % (2 ** 13) == 0:
            sim_perc = 100. * n_iterations / max_iterations
            fer = calculate_fer(results, info_length)
            print('simulating {}/{} frames time {:.2f}s {:.2f}% {}\tFER={:8.2e}'.format(err_frames, max_frame_errs,  time.time() - sim_start_time, sim_perc, n_iterations, fer))
        if n_iterations > 8 * max_frame_errs and calculate_fer(results, info_length) < 1e-4:
            print('stop simulation for data point')
            break

    datapoint_sim_duration = time.time() - sim_start_time
    print_time_statistics(datapoint_sim_duration, python_ns, c_ns, n_iterations)

    ber = calculate_ber(results, info_length)
    fer = calculate_fer(results, info_length)

    print('#frames {} with BER {:8.2e} and FER {:8.2e}'.format(len(results), ber, fer))
    return save_results(filename, constellation_order, inv_coderate, constraint_length, info_length, ebn0, n_iterations, max_turbo_iterations, decoder_type, python_ns, c_ns, results)


def simulate_awgn_turbo_code_err_frames(constellation_order, inv_coderate, constraint_length, info_length, ebn0, max_frame_errs=8, max_turbo_iterations=8, decoder_type=2):
    filename = generate_filename('TurboCodeAWGN', constellation_order, inv_coderate, constraint_length, info_length, ebn0)
    coder = pybcjr.pylte(info_length, max_turbo_iterations=max_turbo_iterations, decoder_type=decoder_type)
    sigma = cs.ebn0_to_sigma(ebn0, 1. / inv_coderate, constellation_order)

    codeword_len = int(np.ceil(1. * inv_coderate * info_length / constellation_order) * constellation_order)
    subblock_indices = np.reshape(np.arange((info_length + constraint_length) * 3), (-1, 3)).T

    rate_match_indices = lte_functions.get_lte_rate_match_indices(subblock_indices, codeword_len, ant_compat=True)

    mod_constell, bit_representation = sd.generate_gray_constellation(constellation_order)
    demapper = sd.pysymboldemapper()

    results = np.array([])
    python_ns = 0
    c_ns = 0
    err_frames = 0
    n_iterations = 0
    max_iterations = max_frame_errs * (2 ** 10)
    sim_start_time = time.time()
    while err_frames < max_frame_errs and n_iterations < max_iterations:
        n_iterations += 1
        bits = np.random.randint(0, 2, info_length).astype(np.int32)
        encoded = coder.encode(bits)

        tx = encoded[rate_match_indices]

        rx_llrs = simulate_awgn_channel(tx, mod_constell, demapper, sigma)

        llrs = np.zeros((info_length + constraint_length) * 3)
        llrs[rate_match_indices] = rx_llrs

        st = time.time()
        u, info_llrs = coder.decode(llrs, bits)

        python_ns += int((time.time() - st) * 1e9)
        c_ns += coder.decoder_duration()

        frame_pattern = bits == u
        n_correct = np.sum(frame_pattern)
        results = np.append(results, n_correct)
        if not n_correct == info_length:
            err_frames += 1
        if n_iterations % (2 ** 13) == 0:
            sim_perc = 100. * n_iterations / max_iterations
            fer = calculate_fer(results, info_length)
            print('simulating {}/{} frames time {:.2f}s {:.2f}% {}\tFER={:8.2e}'.format(err_frames, max_frame_errs,  time.time() - sim_start_time, sim_perc, n_iterations, fer))
        if n_iterations > 8 * max_frame_errs and calculate_fer(results, info_length) < 1e-4:
            print('stop simulation for data point')
            break

    datapoint_sim_duration = time.time() - sim_start_time
    print_time_statistics(datapoint_sim_duration, python_ns, c_ns, n_iterations)

    ber = calculate_ber(results, info_length)
    fer = calculate_fer(results, info_length)

    print('#frames {} with BER {:8.2e} and FER {:8.2e}'.format(len(results), ber, fer))
    return save_results(filename, constellation_order, inv_coderate, constraint_length, info_length, ebn0, n_iterations, max_turbo_iterations, decoder_type, python_ns, c_ns, results)


def lte_simulation_run():
    k_values = np.array([40, 128, 256, 512, 768, 1024, 6144], dtype=int)
    k_values = np.array([40, 512, 1024, ], dtype=int)
    # k_values = np.array([1024, ], dtype=int)
    print(k_values)
    inv_coderate = 2
    constraint_length = 4
    ebn0_list = np.arange(-7.0, 8.25, .25)  # overall
    # ebn0_list = np.arange(-7.0, -3.25, .25)  # BPSK
    # ebn0_list = np.arange(-2.0, 4.25, .25)  # QPSK
    # ebn0_list = np.arange(4.0, 8.25, .25)  # 8-PSK
    fer_list = np.ones(len(ebn0_list))
    ber_list = np.ones(len(ebn0_list))
    # n_iterations = 100000
    num_err_frames = 2 ** 9
    max_turbo_iterations = 8
    decoder_type = 2

    plt.ion()
    plt.semilogy(ebn0_list, fer_list)
    my_figure = plt.gcf()
    plt.draw()

    for constellation_order in range(1, 4):
        for info_length in k_values:
            for i, ebn0 in enumerate(ebn0_list):
                print('\nConstellationOrder {}, invCoderate {}, Contraint length {}, info length {}, EbN0 {:.2f}'.format(constellation_order, inv_coderate, constraint_length, info_length, ebn0))
                res = simulate_awgn_conv_code_err_frames(constellation_order, inv_coderate, constraint_length, info_length, ebn0, num_err_frames, max_turbo_iterations, decoder_type)
                results = res['results']
                fer_list[i] = calculate_fer(results, info_length)
                ber_list[i] = calculate_ber(results, info_length)

                my_figure.clear()
                plt.semilogy(ebn0_list, fer_list)
                plt.semilogy(ebn0_list, ber_list)
                plt.xlabel('Eb/N0 [dB]')
                plt.title('Turbo Code with info length {}, constellation order {}'.format(info_length, constellation_order))
                plt.draw()
    plt.ioff()
    plt.show()


def main():
    np.set_printoptions(precision=2, linewidth=150)
    lte_simulation_run()


if __name__ == '__main__':
    main()
