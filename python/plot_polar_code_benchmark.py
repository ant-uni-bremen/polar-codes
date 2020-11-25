#!/usr/bin/env python3
# -*- coding: utf-8 -*-
#
# Copyright 2020 Johannes Demel.
#
# SPDX-License-Identifier: GPL-3.0-or-later
#


import numpy as np
from scipy import special as sps
import matplotlib as mpl
import matplotlib.pyplot as plt
from matplotlib.lines import Line2D
from pprint import pprint
import argparse
import re
import string
import os
import json
import itertools

from latex_plot_magic import set_size


def load_json(filename):
    with open(filename) as file:
        data = json.load(file)
        return data['context'], data['benchmarks']


def get_cli_configuration():
    parser = argparse.ArgumentParser(
        description='Plot benchmark results')
    parser.add_argument('--file', type=str, nargs='?',
                        default='polar_encoder_benchmark.csv',
                        help='Result CSV file.')
    return parser


def parse_benchmark_name(name):
    # print(name)
    parts = name.split('/')
    values = [int(v) for v in parts[2:]]
    detector_type = parts[1].split('_')
    decoder_type = ''
    if len(detector_type) > 1:
        decoder_type = detector_type[1]
    detector_type = detector_type[0]

    results = {
        'benchmark_type': parts[0].strip('BM_'),
        'detector_type': detector_type,
        'block_length': values[0],
        'info_length': values[1],
        'detector_size': values[-3],
        'is_systematic': bool(values[-2]),
        'dsnr': float(values[-1]) / 100.,
    }
    if len(values) > 5:
        results['list_size'] = values[2]
    if decoder_type:
        results['decoder_type'] = decoder_type
    return results


def update_result(result):
    converters = {'name': str,
                  'run_name': str,
                  'run_type': str,
                  'real_time': float,
                  'cpu_time': float,
                  'time_unit': str,
                  'CodeThr': float,
                  'InfoThr': float,
                  }

    name = result.pop('name')
    run_name = result.pop('run_name')
    res = parse_benchmark_name(name)

    for k, v in result.items():
        res[k] = converters.get(k, int)(v)

    return res


def extract_result_information(results):
    return [update_result(r) for r in results]


def find_all_values(results):
    values = {}
    for r in results:
        for k, v in r.items():
            s = values.get(k, set())
            s.add(v)
            values[k] = s
    return values


def extract_result_line(results, fixed_values):
    line = []
    for r in results:
        if np.all([r[k] == v for k, v in fixed_values.items()]):
            # pprint(r)
            line.append(r)
    return line


def prepare_latency_over_info_length(results):
    infos = []
    latencies = []
    for r in results:
        infos.append(r['info_length'])
        latencies.append(r['cpu_time'])
    indices = np.argsort(infos)
    infos = np.array(infos, dtype=int)[indices]
    latencies = np.array(latencies, dtype=float)[indices]

    # print(results)
    p = results[0]
    label = f'N={p["block_length"]}, {p["detector_type"]}{p["detector_size"]}, dSNR={p["dsnr"]:.1f}'
    if 'list_size' in p:
        label += f', L={p["list_size"]}'
    if 'decoder_type' in p:
        label += f', {p["decoder_type"]}'
    label += ', syst' if p['is_systematic'] else ''
    return infos, latencies, label


def main():
    latex_textwidth = 327.20668  # pt
    print(f'CWD: {os.getcwd()}')
    # parser = get_cli_configuration()
    # args = parser.parse_args()
    # print(args)

    # filename = args.file
    # filename = '../polar_code_benchmarks_encode_crc_trx3970.json'
    # context, results = load_json(filename)
    # pprint(context)
    # filename = '../polar_code_benchmarks_encode_cmac_trx3970.json'
    # context, cmac_results = load_json(filename)
    # pprint(context)
    # results.extend(cmac_results)

    filename = '../polar_code_benchmarks_trx3970.json'
    context, results = load_json(filename)
    pprint(context)

    results = extract_result_information(results)

    values = find_all_values(results)
    measure_keys = ['CodeThr', 'InfoThr', 'real_time',
                    'cpu_time', 'repetitions', 'iterations', ]
    for k in measure_keys:
        values.pop(k)
    for k, v in values.items():
        print(f'{k}\t\t{v}')

    all_latencies = []
    dsize = 32
    dtype = 'CRC'
    is_systematic = False
    list_size = 1
    dsnr = 1.0
    benchmark_type = 'polar_decode'
    block_length = 1024
    decoder_type = 'float'
    fixed_values = {
        'benchmark_type': benchmark_type,
        'block_length': block_length,
        'detector_type': dtype,
        'detector_size': dsize,
        'dsnr': dsnr,
        'is_systematic': is_systematic}
    if benchmark_type == 'polar_decode':
        fixed_values.update({
            'list_size': list_size,
            'decoder_type': decoder_type,
        })

    compare_values = ['decoder_type', 'is_systematic', 'block_length']
    values['block_length'] = set(sorted(values['block_length'])[4:5])
    values['list_size'] = set(sorted(values['list_size'])[1:-1])
    # print(values['block_length'])

    plt.figure(figsize=set_size(latex_textwidth))

    for lv in itertools.product(*(sorted(values[v]) for v in compare_values)):
        for i, v in enumerate(compare_values):
            fixed_values[v] = lv[i]

        plotset = extract_result_line(results, fixed_values)
        if not plotset:
            continue
        info_lens, latencies, label = prepare_latency_over_info_length(
            plotset)
        print(label)
        print(info_lens)
        print(latencies)
        all_latencies.append(latencies)
        plt.plot(info_lens, latencies, label=label)

    # for l in all_latencies[1:]:
    #     print(l - all_latencies[0])
    plt.xlabel(r'$K$')
    plt.ylabel('execution time [ns]')
    plt.legend(fontsize='x-small')
    plt.tight_layout()
    # plt.savefig('polar_encoder_N1024_CRCs.pgf')
    # plt.savefig('polar_encoder_N1024_CMACs.pgf')
    # plt.savefig('polar_encoder_N1024_systematic.pgf')
    # plt.savefig('polar_encoder_N1024_dSNRs.pgf')
    # plt.savefig('polar_encoder_N1024_CRCvsCMAC.pgf')
    # plt.savefig('polar_decoder_N1024_dSNRs.pgf')
    plt.savefig('polar_decoder_N1024_systematic.pgf')


if __name__ == '__main__':
    main()
