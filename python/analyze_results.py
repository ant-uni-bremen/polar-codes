#!/usr/bin/env python3
# -*- coding: utf-8 -*-
#
# Copyright 2020 Johannes Demel.
#
# SPDX-License-Identifier: GPL-3.0-or-later
#

import sys, os
my_dir = os.path.dirname(os.path.realpath(__file__))
# print(my_dir)
# test_module_dir = os.path.join(my_dir, 'build/lib.linux-x86_64-2.7/')
# test_module_dir = os.path.abspath(test_module_dir)
# sys.path.insert(0, test_module_dir)
# print sys.path

import time
import subprocess

SHARE_RESULT_DIR = os.path.join(my_dir, 'results/')
RESULTS_DIR = '~/polar_simulation_results/'
print('NETWORK result folder: ', SHARE_RESULT_DIR)
print('LOCAL   result folder: ', RESULTS_DIR)
subt = time.time()
subprocess.call(['rsync', '-ur',  SHARE_RESULT_DIR, RESULTS_DIR])
sube = time.time()
print('RSYNC update local folder duration: ', sube - subt)



import numpy as np
import matplotlib.pyplot as plt
import datetime
import csv
from matplotlib2tikz import save


def load_results(prefix=None):
    s = time.time()
    result_files = os.listdir(RESULTS_DIR)
    r = time.time()
    print('file list load duration: ', r - s)
    # print('\n'.join(result_files))
    # f = np.load(os.path.join(RESULTS_DIR, result_files[0]))
    # g = f.item()
    # print(g)
    s = time.time()
    results = []
    for filename in result_files:
        if prefix is None or prefix in filename:
            # print(filename)
            f = np.load(os.path.join(RESULTS_DIR, filename))
            g = f.item()
            results.append(g)
    l = time.time()
    print('result load duration: ', l - s)
    return results


def find_key_values(myList, key):
    results = {}
    for d in myList:
        if d[key] in results:
            results[d[key]] += 1
        else:
            results[d[key]] = 1
    return np.sort(results.keys())


def filter_dict_list(myList, myKey, value):
    results = []
    for d in myList:
        if d[myKey] == value:
            results.append(d)
    return results


def merge_multi_entries(ebn0s, vals):
    m_ebn0s = np.unique(ebn0s)
    m_vals = np.zeros(len(m_ebn0s), dtype=vals.dtype)
    for i, e in enumerate(m_ebn0s):
        pos = np.where(ebn0s == e)
        v = vals[pos]
        m_vals[i] = np.sum(v) / len(v)
    return m_ebn0s, m_vals


def calculate_ber(results, info_length):
    n_errors = (len(results) * info_length) - np.sum(results)
    return n_errors / (len(results) * info_length)


def calculate_fer(results, info_length):
    return 1. * (len(results) - len(np.where(results == info_length)[0].flatten())) / len(results)


def extract_ebn0_ber(results):
    bers = np.zeros(len(results))
    ebn0s = np.zeros(len(results))
    for i, r in enumerate(results):
        nbits = r['info_length'] * r['iterations']
        ncorrect = np.sum(r['results'])
        ebn0s[i] = r['EbN0']
        bers[i] = (nbits - ncorrect) / nbits
    pos = np.argsort(ebn0s)
    ebn0s = ebn0s[pos]
    bers = bers[pos]
    ebn0s, bers = merge_multi_entries(ebn0s, bers)
    return ebn0s, bers


def extract_ebn0_fer(results):
    fers = np.zeros(len(results))
    ebn0s = np.zeros(len(results))
    for i, r in enumerate(results):
        ebn0s[i] = r['EbN0']
        fers[i] = calculate_fer(r['results'], r['info_length'])
    pos = np.argsort(ebn0s)
    ebn0s = ebn0s[pos]
    fers = fers[pos]
    ebn0s, fers = merge_multi_entries(ebn0s, fers)
    return ebn0s, fers


def plot_turbo_graph(ax, ldMlist=np.arange(8, dtype=int)):
    results = load_results('Polar')
    # ax.set_title('Turbo Codes')
    info_values = find_key_values(results, 'info_length')
    # info_values = np.sort(info_values)
    line_styles = ['solid', 'dashed', 'dotted']
    line_colors = ['b', 'r', 'g']
    for ii, k in enumerate(info_values):
        res = filter_dict_list(results, 'info_length', k)
        constellation_values = find_key_values(res, 'constellation_order')
        constellation_values = np.intersect1d(constellation_values, ldMlist)
        # constellation_values = np.sort(constellation_values)
        # print(constellation_values)
        for ic, c in enumerate(constellation_values):
            cres = filter_dict_list(res, 'constellation_order', c)
            ebn0s, bers = extract_ebn0_fer(cres)
            ll = r'$N_i=${}, $ldM=${}'.format(k, c)
            # ax.semilogy(ebn0s, bers, label=ll, linestyle=line_styles[ii % len(line_styles)], color=line_colors[ic % len(line_colors)])
            ax.semilogy(ebn0s, bers, label=ll)


def plot_convolutional_graph(ax, ldMlist=np.arange(8, dtype=int)):
    results = load_results('Conv')
    print(results[0].keys())
    info_values = find_key_values(results, 'info_length')
    line_styles = ['solid', 'dashed', 'dotted']
    line_colors = ['m', 'gold', 'y']
    for ii, k in enumerate(info_values):
        res = filter_dict_list(results, 'info_length', k)
        constellation_values = find_key_values(res, 'constellation_order')
        constellation_values = np.intersect1d(constellation_values, ldMlist)
        # print(constellation_values)
        for ic, c in enumerate(constellation_values):
            cres = filter_dict_list(res, 'constellation_order', c)
            ebn0s, bers = extract_ebn0_fer(cres)
            ll = r'CC $N_i=${}, $ldM=${}'.format(k, c)
            ax.semilogy(ebn0s, bers, label=ll, linestyle=line_styles[ii % len(line_styles)], color=line_colors[ic % len(line_colors)])


def load_pcs_csv_file(filename, skiphead=1, skipfoot=0):
    sim_res = np.genfromtxt(filename, delimiter=',', skip_header=skiphead, skip_footer=skipfoot)
    return sim_res


def separate_simulation_results(res_mat):
    res_mat[np.isnan(res_mat)] = 0.0
    res = {}
    for row in res_mat:
        if row[0] in res.keys():
            res[row[0]] = np.vstack((res[row[0]], row))
        else:
            res[row[0]] = row
    for k in res.keys():
        res[k] = res[k][:, 1:]
    return res


def separate_dict_simulation_results(res_dict):
    res = {}
    for k in res_dict.keys():
        if isinstance(res_dict[k], dict):
            res[k] = separate_dict_simulation_results(res_dict[k])
        else:
            res[k] = separate_simulation_results(res_dict[k])
    return res


def plot_fer_throughput_combo(results, common_keys):
    fig, ax0 = plt.subplots()
    ax1 = ax0.twinx()
    keys = np.sort(results.keys())
    for k in keys:
        ebno = results[k][:, 0]
        fer = results[k][:, 1]
        thr = results[k][:, 9] / 1e6
        ax0.semilogy(ebno, fer, label='{}'.format(int(k)), ls='--')
        ax1.plot(ebno, thr, label='{}'.format(int(k)))
    plt.legend()
    ax0.set_ylim((1e-3, 1))
    plt.xlim((0.0, 3.6))
    ax0.set_ylabel('FER')
    ax1.set_ylabel('Throughput [Mbps]')
    plt.xlabel(r'$E_b / N_0$ [dB]')
    plt.grid()
    plt.title('List sizes for Polar Code ({}, {}) with dSNR {}dB'.format(int(common_keys[0]), int(common_keys[1]), common_keys[2]))
    # save('polar_code_N{}_K{}_listlength_performance.pgf'.format(int(common_keys[0]), int(common_keys[1])))
    plt.show()


def plot_fer(results, common_keys):
    keys = np.sort(results.keys())
    for k in keys:
        ebno = results[k][:, 0]
        fer = results[k][:, 1]
        plt.semilogy(ebno, fer, label='{}'.format(int(k)))
    plt.legend()
    plt.ylim((1e-3, 1))
    plt.xlim((0.0, 3.6))
    plt.ylabel('FER')
    plt.xlabel(r'$E_b / N_0$ [dB]')
    plt.grid()
    plt.title('List sizes for Polar Code ({}, {}) with dSNR {}dB'.format(int(common_keys[0]), int(common_keys[1]), common_keys[2]))
    # save('polar_code_N{}_K{}_listlength_performance.pgf'.format(int(common_keys[0]), int(common_keys[1])))
    plt.show()


def plot_throughput(results, common_keys):
    keys = np.sort(results.keys())
    for k in keys:
        ebno = results[k][:, 0]
        thr = results[k][:, 9] / 1e6
        plt.plot(ebno, thr, label='{}'.format(int(k)))
    plt.legend(loc='lower right')
    # plt.ylim((1e-3, 1))
    # plt.xlim((0.0, 3.6))
    plt.ylabel('Throughput [Mbps]')
    plt.xlabel(r'$E_b / N_0$ [dB]')
    plt.grid()
    plt.title('Throughput for Polar Code ({}, {}) with dSNR {}dB'.format(int(common_keys[0]), int(common_keys[1]), common_keys[2]))
    # save('polar_code_N{}_K{}_listlength_throughput.pgf'.format(int(common_keys[0]), int(common_keys[1])))
    plt.show()


def calculate_boxplot_point(dataPoints):
    # print(np.percentile(dataPoints, [0, 25, 50, 75, 100]))
    minval, Q01, Q1, median, Q3, Q99, maxval = np.percentile(dataPoints, [0, .1, 25, 50, 75, 99.9, 100])
    IQR = Q3 - Q1
    # print(Q1)
    # print(median)

    loval = Q1 - 1.5 * IQR
    hival = Q3 + 1.5 * IQR

    wiskhi = np.compress(dataPoints <= hival, dataPoints)
    wisklo = np.compress(dataPoints >= loval, dataPoints)
    actual_hival = np.max(wiskhi)
    actual_loval = np.min(wisklo)
    # print(actual_hival, maxval)
    # print(actual_loval, minval)
    # sortedDataPoints = np.sort(dataPoints)
    # low_outliers = sortedDataPoints[0:10]
    # hi_outliers = sortedDataPoints[-10:]
    low_outliers = np.compress(dataPoints < actual_loval, dataPoints)
    hi_outliers = np.compress(dataPoints > actual_hival, dataPoints)
    actual_loval = Q01
    actual_hival = Q99

    return median, Q1, Q3, actual_loval, actual_hival, low_outliers, hi_outliers


def plot_latency_boxwhiskers(results, common_keys):
    keys = np.sort(results.keys())
    for k in keys:
        ebno = results[k][:, 0]
        mean_time = results[k][:, 15] * 1.e-3
        dataPoints = results[k][:, 17:] * 1.e-3
        medians = np.zeros(len(ebno))
        Q1s = np.zeros(len(ebno))
        Q3s = np.zeros(len(ebno))
        lowval = np.zeros(len(ebno))
        hival = np.zeros(len(ebno))
        for i, dp in enumerate(dataPoints):
            medians[i], Q1s[i], Q3s[i], actual_loval, actual_hival, low_outliers, hi_outliers = calculate_boxplot_point(dp)
            print(len(hi_outliers))
            hi_outliers = hi_outliers[::-1][::np.maximum(10, len(hi_outliers) / 10)]
            print(len(hi_outliers))
            plt.scatter(np.ones(len(hi_outliers)) * ebno[i], hi_outliers)
            plt.vlines(x=ebno[i], ymin=Q1s[i], ymax=Q3s[i], lw=8)
            plt.vlines(x=ebno[i], ymin=actual_loval, ymax=actual_hival, color='b')
            plt.scatter(x=[ebno[i], ebno[i]], y=[actual_loval, actual_hival], marker='_')

            # medians[i] = median
            # plt.axvline(x=ebno[i], ymin=Q1, ymax=Q3, lw=3)
        plt.plot(ebno, medians)
        plt.plot(ebno, mean_time)
        # for i in range(len(ebno)):
        #     plt.vlines(x=ebno[i], ymin=Q1s[i], ymax=Q3s[i])
        # plt.plot(ebno, Q1s)
        # plt.plot(ebno, Q3s)


    #     min_time = results[k][:, 13] * 1.e-3
    #     max_time = results[k][:, 14] * 1.e-3
    #     mean_time = results[k][:, 15] * 1.e-3
    #     dev_time = results[k][:, 16] * 1.e-3
    #     plt.errorbar(x=ebno, y=mean_time, yerr=dev_time, label='{}'.format(int(k)))
    #     # plt.boxplot(ebno, mean_time)
    # plt.legend(loc='upper right')
    # # plt.ylim((0, 300))
    # # plt.xlim((0.0, 3.6))
    # plt.ylabel(r'Latency [$\mu$s]')
    # plt.xlabel(r'$E_b / N_0$ [dB]')
    # plt.grid()
    # plt.title('Latency for Polar Code ({}, {}) with dSNR {}dB'.format(int(common_keys[0]), int(common_keys[1]), common_keys[2]))
    # # save('polar_code_N{}_K{}_listlength_latency.pgf'.format(int(common_keys[0]), int(common_keys[1])))
    plt.show()


def plot_latency(results, common_keys):
    keys = np.sort(results.keys())
    for k in keys:
        print(results[k][0, 0:20])
        print(results[k][0, -20:])

        # get_boxplot_data(results[k][:, 17:])

        ebno = results[k][:, 0]
        min_time = results[k][:, 13] * 1.e-3
        max_time = results[k][:, 14] * 1.e-3
        mean_time = results[k][:, 15] * 1.e-3
        dev_time = results[k][:, 16] * 1.e-3
        plt.errorbar(x=ebno, y=mean_time, yerr=dev_time, label='{}'.format(int(k)))
        # plt.boxplot(ebno, mean_time)
    plt.legend(loc='upper right')
    # plt.ylim((0, 300))
    # plt.xlim((0.0, 3.6))
    plt.ylabel(r'Latency [$\mu$s]')
    plt.xlabel(r'$E_b / N_0$ [dB]')
    plt.grid()
    plt.title('Latency for Polar Code ({}, {}) with dSNR {}dB'.format(int(common_keys[0]), int(common_keys[1]), common_keys[2]))
    # # save('polar_code_N{}_K{}_listlength_latency.pgf'.format(int(common_keys[0]), int(common_keys[1])))
    plt.show()


def plot_pcs_rate_results():
    filename = RESULTS_DIR + 'polar_code_N512_K256_compressed_cmac_listlength.csv'
    # filename = 'polar_decoder_8bit_N1024_L4_fixed.csv'

    sim_res = load_pcs_csv_file(filename)
    K = np.copy(sim_res[:, 1])
    print(K)
    sim_res[:, 1] = sim_res[:, 4]
    # sim_res[:, 1] = L
    sim_res[:, 4] = K
    print(sim_res[:, 0:13])
    # sim_res = sim_res[1:, :]
    results = separate_simulation_results(sim_res)
    for i in range(4):
        results = separate_dict_simulation_results(results)

    common_keys = []
    while 0 < len(results.keys()) < 2:
        print(results.keys())
        common_keys.append(results.keys()[0])
        results = results[results.keys()[0]]
    print(common_keys)

    # plot_fer_throughput_combo(results, common_keys)
    plot_fer(results, common_keys)
    plot_throughput(results, common_keys)
    plot_latency(results, common_keys)
    # plot_latency_boxwhiskers(results, common_keys)


def plot_pcs_results():
    # filename = 'polar_list_decoder_N512_listlength.csv'
    filename = 'polar_list_decoder_N512_soft_test8_listlength.csv'
    filename = 'polar_decoder2_N512_K256_L1-8_listlength.csv'
    filename = 'polar_decoder_unlspc_32bit_N512_K256_L1-16_listlength.csv'
    filename = 'polar_decoder_unlspc_32bit_N2048_K1024_L1-16_listlength.csv'
    filename = 'polar_decoder_timeDur_unlspc_32bit_N512_K256_L1-8_listlength.csv'
    filename = RESULTS_DIR + 'polar_code_N512_K256_compressed_cmac_listlength.csv'
    filename = RESULTS_DIR + 'polar_code_N512_K256_cmac8_listlength.csv'
    filename = RESULTS_DIR + 'polar_code_N512_K256_crc8_listlength.csv'
    # filename = 'polar_decoder_32bit_N512_K256_L1-8_listlength.csv'
    # filename = 'polar_decoder_8bit_N512_K256_L1-8_listlength.csv'
    print(filename)
    sim_res = load_pcs_csv_file(filename)
    # print(sim_res[:, 0:6])
    # sim_res = sim_res[1:, :]
    results = separate_simulation_results(sim_res)
    for i in range(4):
        results = separate_dict_simulation_results(results)

    common_keys = []
    while 0 < len(results.keys()) < 2:
        print(results.keys())
        common_keys.append(results.keys()[0])
        results = results[results.keys()[0]]
    print(common_keys)

    # plot_fer_throughput_combo(results, common_keys)
    plot_fer(results, common_keys)
    plot_throughput(results, common_keys)
    plot_latency(results, common_keys)
    # plot_latency_boxwhiskers(results, common_keys)


def plot_coherence_time():
    '''
    https://en.wikipedia.org/wiki/Coherence_time_(communications_systems)
    '''
    fc = 5.8e9
    c = 3e8
    v = np.arange(10.0, 16., 0.1)
    fd = fc * v / c
    factor = 9. / (16 * np.pi)
    plt.plot(v, 1. / fd, label='1/fd')
    plt.plot(v, factor / fd, label='restrictive')
    plt.plot(v, np.sqrt(factor) / fd, label='Clarke')
    plt.legend()
    plt.grid()
    plt.show()


def main():
    np.set_printoptions(precision=2, linewidth=150)
    plot_pcs_results()
    # plot_pcs_rate_results()
    # plot_coherence_time()
    return
    blockLengths = np.array(sim_res[1:, 0]).astype(int)
    blockLengths = np.reshape(blockLengths, (4, -1))
    invRates = sim_res[1:, 1]
    ebn0s = np.array(sim_res[1:, 4]).astype(float)
    ebn0s = np.reshape(ebn0s, (4, -1))
    print(ebn0s)

    fers = np.array(sim_res[1:, 5]).astype(float)
    fers = np.reshape(fers, (4, -1))

    print(blockLengths)
    for i in range(0, 4):
        plt.semilogy(ebn0s[i], fers[i], label=blockLengths[i, 0])
    plt.legend(title='Blocklength')
    plt.ylabel('FER')
    plt.xlabel(r'$E_b / N_0$')
    plt.grid()
    plt.savefig('codelength_vs_fer.pdf')
    plt.show()
    # print(np.reshape(blockLengths, (4, -1)))

    # fig, ax0 = plt.subplots(1, 1)
    # plot_turbo_graph(ax0, ldMlist=np.arange(2, 3, dtype=int))
    # # plot_convolutional_graph(ax0, ldMlist=np.arange(2, 3, dtype=int))
    # plt.xlabel('Eb/N0')
    # plt.ylabel('FER')
    # plt.ylim((1e-3, 1.0))
    # plt.grid(which='major', color='black', lw=1.4)
    # plt.grid(which='minor', color='gray')
    # ax0.legend(*ax0.get_legend_handles_labels(), bbox_to_anchor=(0., 1.02, 1., .102), loc=0, mode='expand', borderaxespad=0., handletextpad=0.0, ncol=3)
    # plt.xlim((-1, 7.))
    # fig.subplots_adjust(top=0.84)
    # plt.savefig('Polar_codes_SC_32-1024.pdf')
    # plt.show()


if __name__ == '__main__':
    main()
