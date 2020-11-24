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

plt.rcParams.update({
    "font.family": "serif",
    "font.serif": [],                    # use latex default serif font
    "font.sans-serif": ["DejaVu Sans"],  # use a specific sans-serif font
    "pgf.texsystem": "pdflatex",
})


def set_size(width, fraction=1):
    """Set figure dimensions to avoid scaling in LaTeX.

    Source: https://jwalton.info/Embed-Publication-Matplotlib-Latex/

    Parameters
    ----------
    width: float
            Document textwidth or columnwidth in pts
    fraction: float, optional
            Fraction of the width which you wish the figure to occupy

    Returns
    -------
    fig_dim: tuple
            Dimensions of figure in inches
    """
    # Width of figure (in pts)
    fig_width_pt = width * fraction

    # Convert from pt to inches
    inches_per_pt = 1 / 72.27

    # Golden ratio to set aesthetic figure height
    # https://disq.us/p/2940ij3
    golden_ratio = (5**.5 - 1) / 2

    # Figure width in inches
    fig_width_in = fig_width_pt * inches_per_pt
    # Figure height in inches
    fig_height_in = fig_width_in * golden_ratio

    fig_dim = (fig_width_in, fig_height_in)

    return fig_dim


def parse_line(line, column_names):
    converters = {'name': str,
                  'real_time': float,
                  'cpu_time': float,
                  'time_unit': str,
                  'CodeThr': float,
                  'InfoThr': float,
                  }
    result = []
    for i, name in enumerate(column_names):
        converter = converters.get(name, int)
        value = line[i]
        if len(value) > 0:
            value = converter(value)
        else:
            value = np.NaN
        result.append(value)
    return result


def extract_columns(results, column_names, interesting_column_names):
    interested = []
    for r in results:
        l = []
        for i, name in enumerate(column_names):
            if name in interesting_column_names:
                l.append(r[i])
        interested.append(l)
    return interested


def filter_rows(results, column_names, name, value):
    if isinstance(name, list):
        assert len(name) == len(value)
        idx = [column_names.index(n) for n in name]
    else:
        idx = [column_names.index(name), ]
        value = [value, ]
    print(idx)
    res = []
    for r in results:
        print([[r[i], value[n]] for n, i in enumerate(idx)])
        if np.all([r[i] == value[n] for n, i in enumerate(idx)]):
            res.append(r)
        # if r[idx] == value:
        #     res.append(r)
    return res


def load_json(filename):
    with open(filename) as file:
        data = json.load(file)
        return data['context'], data['benchmarks']


def load_csv_file(filename):
    results = np.loadtxt(filename, skiprows=9, dtype=str)
    column_names = results[0].split(',')
    column_names = [c.strip('"') for c in column_names]
    results = results[1:]
    results = [r.split(',') for r in results]
    results = [parse_line(r, column_names) for r in results]
    parsed_name = parse_benchmark_name(results[0][0])
    parsed_keys = list(parsed_name.keys())
    # parsed_values = list(parsed_name.values())
    parsed_keys.extend(column_names[1:])
    column_names = parsed_keys
    result_table = []
    for r in results:
        bm_params = parse_benchmark_name(r[0])
        bmvalues = list(bm_params.values())
        bmvalues.extend(r[1:])
        result_table.append(bmvalues)

    return column_names, result_table


def get_cli_configuration():
    parser = argparse.ArgumentParser(
        description='Plot benchmark results')
    parser.add_argument('--file', type=str, nargs='?',
                        default='polar_encoder_benchmark.csv',
                        help='Result CSV file.')
    return parser


def plot_cpu_time_over_info_length(data, column_names):
    block_length = data[0][column_names.index('block_length')]
    dsnr = data[0][column_names.index('dsnr')]
    detector_type = data[0][column_names.index('detector_type')]
    detector_size = data[0][column_names.index('detector_size')]
    is_systematic = data[0][column_names.index('is_systematic')]
    is_systematic = 'syst.' if is_systematic else 'non-syst.'
    label = f'$N_c={block_length}$, {detector_type}{detector_size}, {is_systematic}'
    info_lengths = extract_columns(data, column_names, ['info_length', ])
    cpu_times = extract_columns(data, column_names, ['cpu_time', ])
    plt.plot(info_lengths, cpu_times, label=label)


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
    plt.figure(figsize=set_size(latex_textwidth))
    # for dtype in sorted(values['detector_type']):
    # for dsnr in sorted(values['dsnr']):
    # for is_systematic in sorted(values['is_systematic']):
    # for list_size in sorted(values['list_size'])[0:4]:
    for dsize in sorted(values['detector_size']):
        for bl in sorted(values['block_length'])[3:5]:
            fixed_values = {
                'benchmark_type': benchmark_type,
                'block_length': bl,
                'detector_type': dtype,
                'detector_size': dsize,
                'dsnr': dsnr,
                'is_systematic': is_systematic}
            if benchmark_type == 'polar_decode':
                fixed_values.update({
                    'list_size': list_size,
                    'decoder_type': 'float',
                })
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


if __name__ == '__main__':
    main()
