#!/usr/bin/env python3
# -*- coding: utf-8 -*-
#
# Copyright 2020 Johannes Demel.
#
# SPDX-License-Identifier: GPL-3.0-or-later
#

import numpy as np
import scipy.special as sps
import matplotlib.pyplot as plt

import pypolar
from polar_code_tools import calculate_bec_channel_capacities, design_snr_to_bec_eta


def plot_channel_capacities(capacity, save_file=None):
    block_size = len(capacity)
    # FUN with matplotlib LaTeX fonts! http://matplotlib.org/users/usetex.html
    plt.rc('text', usetex=True)
    # plt.rc('font', family='serif')
    plt.rc('figure', autolayout=True)
    plt.plot(capacity)
    plt.xlim([0, block_size - 1])
    plt.ylim([-0.01, 1.01])
    plt.xlabel('synthetic channel number')
    plt.ylabel('channel capacity')
    # plt.title('BEC channel construction')
    plt.grid()
    plt.gcf().set_size_inches(plt.gcf().get_size_inches() * .5)
    if save_file:
        # save(save_file)
        plt.savefig(save_file)
        pass
    plt.show()


def plot_average_channel_distance(save_file=None):
    eta = 0.5 #  design_snr_to_bec_eta(-1.5917)
    powers = np.arange(4, 26)

    try:
        import matplotlib.pyplot as plt
        import matplotlib
        # FUN with matplotlib LaTeX fonts! http://matplotlib.org/users/usetex.html
        plt.rc('text', usetex=True)
        plt.rc('font', family='serif')
        plt.rc('figure', autolayout=True)

        dist = []
        medians = []
        initial_channel = 1 - eta
        for p in powers:
            bs = int(2 ** p)
            capacities = calculate_bec_channel_capacities(eta, bs)
            avg_capacity = np.repeat(initial_channel, len(capacities))
            averages = np.abs(capacities - avg_capacity)
            avg_distance = np.sum(averages) / float(len(capacities))
            dist.append(avg_distance)
            variance = np.std(averages)
            medians.append(variance)

        plt.errorbar(powers, dist, yerr=medians)
        plt.grid()
        plt.xlabel(r'block size $N$')
        plt.ylabel(r'$\frac{1}{N} \sum_i |I(W_N^{(i)}) - 0.5|$')

        axes = plt.axes()
        tick_values = np.array(axes.get_xticks().tolist())
        tick_labels = np.array(tick_values, dtype=int)
        tick_labels = ['$2^{' + str(i) + '}$' for i in tick_labels]
        plt.xticks(tick_values, tick_labels)
        plt.xlim((powers[0], powers[-1]))
        plt.ylim((0.2, 0.5001))
        plt.gcf().set_size_inches(plt.gcf().get_size_inches() * .5)
        if save_file:
            plt.savefig(save_file)
        plt.show()
    except ImportError:
        pass


def plot_capacity_histogram(design_snr, save_file=None):
    eta = design_snr_to_bec_eta(design_snr)
    # capacities = calculate_bec_channel_capacities(eta, block_size)
    try:
        import matplotlib.pyplot as plt
        # FUN with matplotlib LaTeX fonts! http://matplotlib.org/users/usetex.html
        plt.rc('text', usetex=True)
        plt.rc('font', family='serif')
        plt.rc('figure', autolayout=True)

        block_sizes = [32, 128, 512]
        for b in block_sizes:
            capacities = calculate_bec_channel_capacities(eta, b)
            w = 1. / float(len(capacities))
            weights = [w, ] * b
            plt.hist(capacities, bins=b, weights=weights, range=(0.95, 1.0))
        plt.grid()
        plt.xlabel('synthetic channel capacity')
        plt.ylabel('normalized item count')
        print(plt.gcf().get_size_inches())
        plt.gcf().set_size_inches(plt.gcf().get_size_inches() * .5)
        if save_file:
            plt.savefig(save_file)
        plt.show()
    except ImportError:
        pass


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


def main():
    # plot_channel_coding_bounds()
    # plot_capacity_rate()
    # plot_r0_fer()
    # dsnr_db = 0.0
    # eta = design_snr_to_bec_eta(dsnr_db, .5)
    eta = .5
    capacities = calculate_bec_channel_capacities(eta, 2 ** 5)
    # plot_channel_capacities(capacities)
    plot_channel_capacities(capacities, 'graph_polarized_channel.pgf')
    # plot_average_channel_distance()
    # plot_capacity_histogram(dsnr_db)






if __name__ == '__main__':
    main()
