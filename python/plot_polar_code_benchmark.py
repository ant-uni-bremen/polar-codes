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
import datetime

from latex_plot_magic import set_size


def load_json(filename, cpu_name):
    with open(filename) as file:
        data = json.load(file)
        benchmarks = data['benchmarks']
        for b in benchmarks:
            b['cpu_name'] = cpu_name
        context = data['context']
        context['cpu_name'] = cpu_name
        return context, benchmarks


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
    generator_type = 'BB'
    if len(detector_type) > 1:
        decoder_type = detector_type[-1]
    if len(detector_type) > 2:
        generator_type = detector_type[1]
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
    if generator_type:
        results['generator_type'] = generator_type
    return results


def update_result(result):
    converters = {'name': str,
                  'run_name': str,
                  'run_type': str,
                  'cpu_name': str,
                  'real_time': float,
                  'cpu_time': float,
                  'time_unit': str,
                  'CodeThr': float,
                  'InfoThr': float,
                  }

    name = result.pop('name')
    run_name = result.pop('run_name')
    # print(f'{name}, "{run_name}')
    res = parse_benchmark_name(name)
    # print(f'{res}')

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
    # print('\n\n\n')
    # print(fixed_values)
    for r in results:
        if np.all([r.get(k, None) == v for k, v in fixed_values.items()]):
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


def prepare_throughput_over_info_length(results):
    infos = []
    code_thr = []
    info_thr = []
    for r in results:
        infos.append(r['info_length'])
        code_thr.append(r['CodeThr'])
        info_thr.append(r['InfoThr'])
    indices = np.argsort(infos)
    infos = np.array(infos, dtype=int)[indices]
    code_thr = np.array(code_thr, dtype=float)[indices]
    info_thr = np.array(info_thr, dtype=float)[indices]

    # print(results)
    p = results[0]
    label = f'N={p["block_length"]}, {p["detector_type"]}{p["detector_size"]}'
    if 'generator_type' in p:
        label += f', {p["generator_type"]}'
    label += f', dSNR={p["dsnr"]:.1f}'
    if 'list_size' in p:
        label += f', L={p["list_size"]}'
    if 'decoder_type' in p:
        label += f', {p["decoder_type"]}'
    label += ', syst' if p['is_systematic'] else ''
    return infos, code_thr, info_thr, label


def parse_date(date_str):
    date_str = date_str.split('+', 1)[0]
    return datetime.datetime.strptime(date_str, '%Y-%m-%dT%H:%M:%S')


def merge_results(context, results, next_context, next_results):
    assert context['host_name'] == next_context['host_name']
    assert context['mhz_per_cpu'] == next_context['mhz_per_cpu']
    assert context['num_cpus'] == next_context['num_cpus']

    contextdate = parse_date(context['date'])
    print(context['date'])
    print(contextdate)
    nextcontextdate = parse_date(next_context['date'])
    print(next_context['date'])
    print(nextcontextdate)
    print(contextdate < nextcontextdate)
    print(contextdate > nextcontextdate)
    c = context if contextdate > nextcontextdate else next_context
    r = results if contextdate > nextcontextdate else next_results

    # oldc = next_context if contextdate > nextcontextdate else context
    oldr = next_results if contextdate > nextcontextdate else results

    rnames = [i['name'] for i in r]
    for i in oldr:
        if 'BE' in i['name']:
            print(i['name'])
        if i in rnames:
            print('duplicate')
        else:
            # print(f'append: {i["name"]}')
            r.append(i)

    return c, r


def load_results(filenames):
    assert len(filenames) > 0
    # choose a random reference
    res = []
    for f, cpu in filenames:
        fc, fr = load_json(f, cpu)
        res.append([fc, fr])
    return res


def sanitize_value_set(values, measure_keys=['CodeThr', 'InfoThr', 'real_time',
                                             'cpu_time', 'repetitions', 'iterations', ]):
    for k in measure_keys:
        values.pop(k)
    return values


def exlcude_pattern(plot_data, pattern):
    res = []
    for d in plot_data:
        if np.all([d[0].get(k, None) in v for k, v in pattern.items()]):
            continue
        res.append(d)
    return res


def main():
    latex_textwidth = 327.20668  # pt
    print(f'CWD: {os.getcwd()}')

    filenames = ['../polar_code_benchmarks_decode_trx3970_1024vs512.json',
                 '../polar_code_benchmarks_decode_trx3970_256vs128.json',
                 '../polar_code_benchmarks_decode_trx3970_1024_generators.json']
    filenames = ['../polar_code_benchmarks_encode_r5900_1024.json', ]
    filenames = [['../polar_code_benchmarks_encode_trx3970.json', "Ryzen Threadripper 3970X"],
                 ['../polar_code_benchmarks_encode_xeon6254.json', "Xeon Gold 6254"],
                 ['../polar_code_benchmarks_encode_7700t.json', "Core i7-7700T"],
                 ['../polar_code_benchmarks_encode_r5800.json', "Ryzen 7 5800X"],
                 ['../polar_code_benchmarks_encode_r5900.json', "Ryzen 9 5900X"], ]
    # filenames.extend(ext_filenames)
    raw_results = load_results(filenames)
    results = []
    for r in raw_results:
        pprint(r[0])
        print(np.shape(r[1]))
        results.extend(r[1])
    print(np.shape(results))
    # pprint(context)

    results = extract_result_information(results)

    # pprint(results)
    values = find_all_values(results)

    values = sanitize_value_set(values)
    for k, v in values.items():
        print(f'{k}\t\t{v}')
    # return
    all_latencies = []
    benchmark_plot_type = 'throughput'
    dsize = 0
    dtype = 'CRC'
    is_systematic = False
    list_size = 1
    dsnr = 1.0
    benchmark_type = 'polar_encode'
    block_length = 131072
    decoder_type = 'float'
    generator_type = 'BB'
    fixed_values = {
        'benchmark_type': benchmark_type,
        'block_length': block_length,
        'detector_type': dtype,
        'detector_size': dsize,
        'dsnr': dsnr,
        'is_systematic': is_systematic,
        'generator_type': generator_type}
    if benchmark_type == 'polar_decode':
        fixed_values.update({
            'list_size': list_size,
            'decoder_type': decoder_type,
        })

    # compare_values = ['generator_type', 'dsnr']
    compare_values = ['detector_size', 'detector_type', ]
    compare_values = ['cpu_name',]
    compare_values = ['cpu_name', 'block_length']
    # values['block_length'] = set(sorted(values['block_length'])[4:5])
    # values['list_size'] = set(sorted(values['list_size'])[-3:])
    # print(values['block_length'])

    plot_data = []
    for lv in itertools.product(*(sorted(values[v]) for v in compare_values)):
        for i, v in enumerate(compare_values):
            fixed_values[v] = lv[i]

        plotset = extract_result_line(results, fixed_values)
        if not plotset:
            continue
        plot_data.append(plotset)

    pattern = {'detector_type': ['CMAC', ],
                'detector_size': [16, 64]}
    plot_data = exlcude_pattern(plot_data, pattern)

    pattern = {'is_systematic': [True, ],
                'detector_size': [0, 8, 16, 64]}
    plot_data = exlcude_pattern(plot_data, pattern)

    pattern = {'detector_type': ['CRC', ],
                'block_length': [16, 64, 128, 512, 2048, 32768]}
    plot_data = exlcude_pattern(plot_data, pattern)
    # pattern = {'is_systematic': [True, ],
    #             'detector_type': ['CRC', ],
    #             'detector_size': [32, ]}
    # plot_data = exlcude_pattern(plot_data, pattern)

    plt.figure(figsize=set_size(latex_textwidth))

    if benchmark_plot_type == 'latency':
        for plotset in plot_data:
            info_lens, latencies, label = prepare_latency_over_info_length(
                plotset)
            print(label)
            if 'cpu_name' in compare_values and len(compare_values) < 2:
                label = plotset[0]['cpu_name']
            print(info_lens)
            print(latencies)
            plt.plot(info_lens, latencies, label=label)
        plt.xlabel(r'$K$')
        plt.ylabel('execution time [ns]')
    elif benchmark_plot_type == 'throughput':
        for plotset in plot_data:
            info_lens, code_thr, info_thr, label = prepare_throughput_over_info_length(
                plotset)
            res_block_length = plotset[0]['block_length']
            code_rate = info_lens / res_block_length
            print(label)
            print(info_lens)
            print(code_thr)
            print(info_thr)
            if 'cpu_name' in compare_values:
                label = ', '.join([f'{plotset[0][v]}' for v in compare_values])

            p = plt.plot(code_rate, code_thr, label=label)
            color = p[0].get_color()
            p = plt.plot(code_rate, info_thr, color=color, linestyle='dashed')
        plt.xlabel(r'$R$')
        plt.ylabel('throughput [b/s]')
    else:
        print('unknown plot type!')
        return

    plt.legend(fontsize='xx-small')
    plt.grid()
    plt.tight_layout()
    # plt.savefig('polar_encoder_N1024_CRCs.pgf')
    # plt.savefig('polar_encoder_blocksizes.pgf')
    # plt.savefig('polar_encoder_N1024_CRCs.pgf')
    # plt.savefig('polar_encoder_N1024_CMACs.pgf')
    # plt.savefig('polar_encoder_N1024_systematic.pgf')
    # plt.savefig('polar_encoder_N1024_dSNRs.pgf')
    # plt.savefig('polar_encoder_N1024_CRCvsCMAC.pgf')
    # plt.savefig('polar_decoder_blocksizes.pgf')
    # plt.savefig('polar_decoder_N1024_dSNRs.pgf')
    # plt.savefig('polar_decoder_N1024_systematic.pgf')
    # plt.savefig('polar_decoder_N1024_CRCs.pgf')
    # plt.savefig('polar_decoder_N1024_listsizes_small.pgf')
    # plt.savefig('polar_decoder_N1024_listsizes_large.pgf')
    # plt.savefig('polar_encoder_throughput.pgf')
    # plt.savefig('polar_decoder_throughput_short.pgf')
    # plt.savefig('polar_decoder_throughput_list.pgf')


if __name__ == '__main__':
    main()
