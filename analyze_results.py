#!/usr/bin/env python
# The first few lines make the locally build module available in this python script!
from __future__ import print_function, division
import sys, os
my_dir = os.path.dirname(os.path.realpath(__file__))
# print(my_dir)
test_module_dir = os.path.join(my_dir, 'build/lib.linux-x86_64-2.7/')
test_module_dir = os.path.abspath(test_module_dir)
sys.path.insert(0, test_module_dir)
# print sys.path

import time
import subprocess

SHARE_RESULT_DIR = os.path.join(my_dir, 'results/')
RESULTS_DIR = '/lhome/records/polar_simulation_results/'
print('NETWORK result folder: ', SHARE_RESULT_DIR)
print('LOCAL   result folder: ', RESULTS_DIR)
subt = time.time()
subprocess.call(['rsync', '-ur',  SHARE_RESULT_DIR, RESULTS_DIR])
sube = time.time()
print('RSYNC update local folder duration: ', sube - subt)



import numpy as np
import matplotlib.pyplot as plt
import datetime


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


def main():
    np.set_printoptions(precision=2, linewidth=150)

    fig, ax0 = plt.subplots(1, 1)
    plot_turbo_graph(ax0, ldMlist=np.arange(2, 3, dtype=int))
    # plot_convolutional_graph(ax0, ldMlist=np.arange(2, 3, dtype=int))
    plt.xlabel('Eb/N0')
    plt.ylabel('FER')
    plt.ylim((1e-3, 1.0))
    plt.grid(which='major', color='black', lw=1.4)
    plt.grid(which='minor', color='gray')
    ax0.legend(*ax0.get_legend_handles_labels(), bbox_to_anchor=(0., 1.02, 1., .102), loc=0, mode='expand', borderaxespad=0., handletextpad=0.0, ncol=3)
    plt.xlim((-1, 7.))
    fig.subplots_adjust(top=0.84)
    plt.savefig('Polar_codes_SC_32-1024.pdf')
    plt.show()


if __name__ == '__main__':
    main()
