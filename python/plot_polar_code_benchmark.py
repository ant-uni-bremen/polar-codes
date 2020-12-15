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
    context, results = load_json(filenames[0])
    print(f'initial result size: {len(results)}')

    for f in filenames[1:]:
        fc, fr = load_json(f)
        context, results = merge_results(context, results, fc, fr)
        print(f'new     result size: {len(results)}')

    return context, results


def sanitize_value_set(values, measure_keys=['CodeThr', 'InfoThr', 'real_time',
                                             'cpu_time', 'repetitions', 'iterations', ]):
    for k in measure_keys:
        values.pop(k)
    return values


def main():
    latex_textwidth = 327.20668  # pt
    print(f'CWD: {os.getcwd()}')
    # parser = get_cli_configuration()
    # args = parser.parse_args()
    # print(args)

    # filename = args.file
    ext_filenames = ['../polar_code_benchmarks_encode_crc_trx3970.json',
                     '../polar_code_benchmarks_encode_cmac_trx3970.json',
                     '../polar_code_benchmarks_trx3970.json',
                     ]
    # filename = '../polar_code_benchmarks_encode_crc_trx3970.json'
    # context, crc_results = load_json(filename)
    # pprint(context)
    # results.extend(crc_results)
    # filename = '../polar_code_benchmarks_encode_cmac_trx3970.json'
    # context, cmac_results = load_json(filename)
    # pprint(context)
    # results.extend(cmac_results)

    # filename = '../polar_code_benchmarks_trx3970.json'
    # context, decode_results = load_json(filename)
    # pprint(context)
    # results.extend(decode_results)
    filenames = ['../polar_code_benchmarks_decode_trx3970_1024vs512.json',
                 '../polar_code_benchmarks_decode_trx3970_256vs128.json',
                 '../polar_code_benchmarks_decode_trx3970_1024_generators.json']
    # filenames.extend(ext_filenames)
    context, results = load_results(filenames)
    pprint(context)
    results = extract_result_information(results)

    # pprint(results)
    values = find_all_values(results)
    # measure_keys = ['CodeThr', 'InfoThr', 'real_time',
    #                 'cpu_time', 'repetitions', 'iterations', ]
    # for k in measure_keys:
    #     values.pop(k)
    values = sanitize_value_set(values)
    for k, v in values.items():
        print(f'{k}\t\t{v}')

    all_latencies = []
    dsize = 0
    dtype = 'CRC'
    is_systematic = True
    list_size = 1
    dsnr = 1.0
    benchmark_type = 'polar_decode'
    block_length = 1024
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

    compare_values = ['generator_type', 'dsnr']
    # values['block_length'] = set(sorted(values['block_length'])[4:5])
    # values['list_size'] = set(sorted(values['list_size'])[-3:])
    # print(values['block_length'])

    plt.figure(figsize=set_size(latex_textwidth))

    for lv in itertools.product(*(sorted(values[v]) for v in compare_values)):
        for i, v in enumerate(compare_values):
            fixed_values[v] = lv[i]

        plotset = extract_result_line(results, fixed_values)
        if not plotset:
            continue
        print('all values in plotset')
        pprint(sanitize_value_set(find_all_values(plotset)))
        # info_lens, latencies, label = prepare_latency_over_info_length(
        #     plotset)
        # plt.plot(info_lens, code_thr, label=label)
        info_lens, code_thr, info_thr, label = prepare_throughput_over_info_length(
            plotset)

        code_rate = info_lens / fixed_values['block_length']
        print(label)
        print(info_lens)
        print(code_thr)
        print(info_thr)
        # print(latencies)
        # all_latencies.append(latencies)
        p = plt.plot(code_rate, code_thr, label=label)
        color = p[0].get_color()
        p = plt.plot(code_rate, info_thr, color=color, linestyle='dashed')

    # for l in all_latencies[1:]:
    #     print(l - all_latencies[0])
    plt.xlabel(r'$K$')
    plt.ylabel('execution time [ns]')

    plt.xlabel(r'$R$')
    plt.ylabel('throughput [b/s]')

    plt.legend(fontsize='xx-small')
    plt.tight_layout()
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
