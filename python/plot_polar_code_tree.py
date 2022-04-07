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
import networkx as nx
from pprint import pprint

from frozen_bit_positions import get_frozen_bit_generator

from latex_plot_magic import set_size


def create_polar_tree_graph(N, frozen_bit_positions):
    n = int(np.log2(N))

    G = nx.DiGraph()
    for i in range(n + 1):
        Nlayer = int(2 ** i)
        offset = N / Nlayer
        spacing = N / Nlayer
        node_size = spacing
        print(node_size)
        for l in range(Nlayer):
            p = spacing * l + spacing / 2.
            nodetype = 'NA'
            if n == i:
                nodetype = 'rate0' if l in frozen_bit_positions else 'rate1'
            G.add_node(f'p{i:02}{l:04}', pos=(p, -i),
                       layer=i, value=l, nodetype=nodetype)

        # nodes = [f'p{i:02}{l:04}' for l in range(Nlayer)]
        # print(nodes)
        # pos = {f'p{i:02}{l:04}': (i, l) for l in range(Nlayer)}
        # print(pos)
        # G.add_nodes_from(nodes, pos=pos)
        if i > 0:
            edges = [(f'p{i-1:02}{l:04}', f'p{i:02}{2 * l:04}')
                     for l in range(Nlayer//2)]
            G.add_edges_from(edges)
            edges = [(f'p{i-1:02}{l:04}', f'p{i:02}{2 * l + 1:04}')
                     for l in range(Nlayer//2)]
            G.add_edges_from(edges)

    print('nodes')
    print(G.nodes.data())
    print('positions')
    print(nx.get_node_attributes(G, 'pos'))
    print('edges')
    print(G.edges.data())
    return G


def update_node_type(G, node):
    print('update node:', node, G.nodes[node])
    edges = list(G.out_edges(nbunch=[node, ]))
    if not edges:
        G.nodes[node]['ctsize'] = 1
        if G.nodes[node]['nodetype'] == 'rate0':
            G.nodes[node]['pattern'] = [1, ]
        else:
            G.nodes[node]['pattern'] = [0, ]
        return G

    n0 = edges[0][1]
    n1 = edges[1][1]
    update_node_type(G, n0)
    update_node_type(G, n1)

    # print('update node:', node, G.nodes[n0])
    # print('update node:', node, G.nodes[n1])
    G.nodes[node]['ctsize'] = G.nodes[n0]['ctsize'] + G.nodes[n1]['ctsize']
    G.nodes[node]['pattern'] = pattern = G.nodes[n0]['pattern'] + \
        G.nodes[n1]['pattern']
    pattern = np.array(pattern, dtype=int)
    ntype = find_node_type(pattern)
    if pattern.size == 8:
        print(f'type={ntype} -> pattern: {pattern}')

    if ntype == 'type1':
        ntype = 'DREP'
    if ntype == 'type2':
        ntype = 'TREP'
    if ntype == 'type3':
        ntype = 'DSPC'
    if ntype == 'type4':
        ntype = 'TSPC'

    if ntype in ('TSPC', ):
        ntype = 'rateR'
    # if not ntype in ('rate0', 'rate1', 'rateR'):
    #     ntype = 'rateR'

    # if not ntype in ('rate0', 'rate1', 'REP', 'DREP', 'SPC', 'DSPC', 'rateR'):
    #     ntype = 'rateR'

    # if not ntype in ('rate0', 'rate1', 'REP', 'DREP', 'TREP', 'SPC', 'DSPC', 'rateR'):
    #     ntype = 'rateR'

    # if not ntype in ('rate0', 'rate1', 'REP', 'DREP', 'TREP', 'SPC', 'DSPC', 'TSPC', 'rateR'):
    #     ntype = 'rateR'
    G.nodes[node]['nodetype'] = ntype

    # print('finish node:', node, ntype, G.nodes[node])
    return G


def find_node_type(frozen_pattern):
    if np.all(frozen_pattern == 1):
        return 'rate0'
    elif np.all(frozen_pattern[0:-1] == 1) and frozen_pattern[-1] == 0:
        return 'REP'
    elif np.all(frozen_pattern[1:] == 0) and frozen_pattern[0] == 1:
        return 'SPC'
    elif np.all(frozen_pattern == 0):
        return 'rate1'
    elif frozen_pattern.size == 8 and np.all(frozen_pattern[0:5] == 1):
        return 'ZeroSPC'
    elif frozen_pattern.size == 8 and np.all(frozen_pattern[0:3] == 1):
        return 'REPONE'
    elif frozen_pattern.size > 2 and np.all(frozen_pattern[0:-2] == 1) and np.all(frozen_pattern[-2:] == 0):
        return 'type1'
    elif frozen_pattern.size > 2 and np.all(frozen_pattern[0:-3] == 1) and np.all(frozen_pattern[-3:] == 0):
        return 'type2'
    elif frozen_pattern.size > 2 and np.all(frozen_pattern[0:2] == 1) and np.all(frozen_pattern[2:] == 0):
        return 'type3'
    elif frozen_pattern.size > 2 and np.all(frozen_pattern[0:3] == 1) and np.all(frozen_pattern[3:] == 0):
        return 'type4'
    elif frozen_pattern.size > 4 and np.all(frozen_pattern[0:-5] == 1) and np.all(frozen_pattern[-5:] == np.array([0, 1, 0, 0, 0])):
        return 'type5'
    else:
        return 'rateR'


def recurse_layer(frozen_pattern, start, stop, layer):
    result = {'start': start, 'stop': stop, 'layer': layer}
    result['type'] = find_node_type(frozen_pattern)
    rN = frozen_pattern.size
    if result['type'] == 'rateR':
        # if rN == 8:
        #     print(f'Nc=8\t{frozen_pattern}, {start}, {stop}')
        rNhalf = rN // 2
        left = recurse_layer(
            frozen_pattern[0:rNhalf], start, start + rNhalf, layer + 1)
        right = recurse_layer(
            frozen_pattern[rNhalf:], start + rNhalf, stop, layer + 1)
        result.update({'left': left, 'right': right})
        # if rN == 8:
        #     print(f'small constituent: {start} - {stop} @{layer}')
        #     print('left')
        #     pprint(left)
        #     print('right')
        #     pprint(right)
    else:
        result['pattern'] = frozen_pattern
        # t = result['type']
        # print(f'N={rN}, ({start}, {stop}), {t}, {frozen_pattern}')
    return result


def analyze_graph_structure(N, frozen_bit_positions):
    frozen_pattern = np.zeros(N, dtype=int)
    frozen_pattern[frozen_bit_positions] = 1
    result = recurse_layer(frozen_pattern, 0, N, 0)
    # pprint(result)
    return result


def merge_patterns(lp, rp):
    for p in rp:
        if p in lp:
            lp[p] += rp[p]
        else:
            lp[p] = rp[p]
    return lp


def extract_patterns(result):
    if 'left' in result and 'right' in result:
        lp = extract_patterns(result['left'])
        rp = extract_patterns(result['right'])
        return merge_patterns(lp, rp)
    else:
        return {tuple(result['pattern']): 1}


def find_all_patterns(nmax, nmin=5, Kmax=164, Kmin=36):
    snr = 1.
    generator = '5G'
    all_patterns = {}
    for n in range(nmin, nmax + 1):
        N = int(2 ** n)
        for K in range(Kmin, Kmax + 1):
            if K <= N:
                # print(f'({N}, {K})')
                frozen_bit_positions = get_frozen_bit_generator(
                    generator, N, K, snr).frozen_bit_positions()
                result = analyze_graph_structure(N, frozen_bit_positions)
                patterns = extract_patterns(result)
                for k in patterns.keys():
                    if len(k) < 8:
                        print(f'({N}, {K}), {k}\tR={K / N}')

                all_patterns = merge_patterns(all_patterns, patterns)
    return all_patterns


def calculate_polar_code_power(E, K, nmax=9):
    RMIN = 1. / 8.
    nMIN = 5
    repthr = (9. / 8.) * (2 ** (np.ceil(np.log2(E)) - 1))
    rate = 1. * K / E
    n1 = int(np.ceil(np.log2(E)))
    if E <= repthr and rate < 9. / 16.:
        n1 -= 1
    n2 = int(np.ceil(np.log2(K / RMIN)))
    return np.maximum(np.min([n1, n2, nmax]), nMIN)


def find_polar_codes():
    sizes = 108 * (2 ** np.arange(5))
    print(sizes)
    maxrate = 0.
    minrate = 1.
    for E in sizes:
        for K in range(36, np.minimum(E, 164) + 1):
            n = calculate_polar_code_power(E, K)
            N = 2 ** n
            print(f'({N}, {K}), rate={K / N}')
            maxrate = np.maximum(maxrate, K / N)
            minrate = np.minimum(minrate, K / N)

    print(f'min={minrate},\tmax={maxrate}')
    # minrate code (512, 36)
    # maxrate code (128, 108)


def prune_nodes(G, node, remove=False):
    edges = list(G.out_edges(nbunch=[node, ]))
    if edges:
        remove_leafs = False
        if G.nodes[node]['nodetype'] != 'rateR':
            remove_leafs = True

        n0 = edges[0][1]
        n1 = edges[1][1]
        prune_nodes(G, n0, remove_leafs)
        prune_nodes(G, n1, remove_leafs)

    if remove:
        G.remove_node(node)
    return G


def main():
    n = 4
    N = int(2 ** n)
    K = int(N * 1 / 2)
    snr = 1.
    generator = 'BB'
    tree_type = 'full' # 'prune
    code_type = 'SSC' # 'Fast-SSC', 'ADVANCED'
    # find_polar_codes()
    # return

    frozen_bit_positions = get_frozen_bit_generator(
        generator, N, K, snr).frozen_bit_positions()
    print(frozen_bit_positions)
    # result = analyze_graph_structure(N, frozen_bit_positions)
    # patterns = extract_patterns(result)
    # # pprint(patterns)
    # all_patterns = find_all_patterns(9, nmin=7, Kmin=12)
    # # print('results!')
    # # pprint(all_patterns)
    # print('num unique patterns: ', len(all_patterns))
    # for nn in range(9):
    #     cN = int(2 ** nn)
    #     for k, v in all_patterns.items():
    #         N = len(k)
    #         if cN == N:
    #             t = find_node_type(np.array(k))
    #             print(f'{t}-{N}: {v} occurences')
    # return

    G = create_polar_tree_graph(N, frozen_bit_positions)
    rootnode = [n for n, d in G.in_degree() if d == 0][0]
    print(rootnode)
    G = update_node_type(G, rootnode)

    if tree_type == 'prune':
        # For pruned trees that end at specialization.
        G = prune_nodes(G, rootnode, remove=False)

    labels = {}
    colors = {}

    # I recall a presentation by Alexios Balatsoukas-Stimming that suggested these colors as a common default.
    colormap = {'rate0': 'white', 'rate1': 'black',
                'REP': 'green', 'DREP': 'cyan', 'TREP': 'blue', 'REPONE': 'blue',
                'SPC': 'orange', 'DSPC': 'yellow', 'TSPC': 'brown', 'ZeroSPC': 'brown',
                'type5': 'red', 'rateR': 'gray'}

    # SSC colormap
    if code_type == 'SSC':
        colormap = {'rate0': 'white', 'rate1': 'black',
                    'REPONE': 'gray',
                    'REP': 'gray',
                    'SPC': 'gray',
                    'rateR': 'gray'}
    elif code_type == 'Fast-SSC':
        # Fast-SSC colormap
        colormap = {'rate0': 'white', 'rate1': 'black',
                    'REPONE': 'blue',
                    'REP': 'green',
                    'SPC': 'orange',
                    'rateR': 'gray'}

    # colormap = {'rate0': 'black', 'rate1': 'red',
    #             'REP': 'gray', 'SPC': 'cyan', 'rateR': 'blue'}

    for node in list(G.nodes):
        # print(node, G.nodes[node])
        G.nodes[node]['label'] = G.nodes[node]['nodetype']
        colors[node] = colormap[G.nodes[node]['nodetype']]
        labels[G.nodes[node]['nodetype']] = labels.get(
            G.nodes[node]['nodetype'], 0) + 1

    print('labels:', labels)

    if code_type == 'SSC':
        # Remove labels for SSC tree
        labels.pop('REP')
        labels.pop('SPC')

    legend_elements = []
    for label in labels.keys():
        legend_elements.append(Line2D([0], [0], marker='o', color=colormap[label], label=label,
                                      markersize=10, linewidth=0))

    nodes = nx.draw_networkx_nodes(G, nx.get_node_attributes(
        G, 'pos'), node_size=50, node_color=colors.values(), edgecolors='black')
    edges = nx.draw_networkx_edges(
        G, nx.get_node_attributes(G, 'pos'), arrows=False)
    plt.legend(handles=legend_elements, fontsize='x-small')
    ax = plt.gca()
    ax.set_axis_off()
    plt.tight_layout()

    fig = plt.gcf()
    fig.set_size_inches(set_size())
    plt.savefig(
        f'polar_code_tree_N{N}_K{K}_{generator}SNR{snr:.0f}_{code_type}_{tree_type}.pgf', bbox_inches='tight', pad_inches=0)
    plt.show()


if __name__ == '__main__':
    main()
