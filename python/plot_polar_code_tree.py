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
from channel_construction import ChannelConstructorGaussianApproximationDai


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
            G.add_node(f'p{i:02}{l:04}', pos=(p, -i), layer=i, value=l, nodetype=nodetype)

        # nodes = [f'p{i:02}{l:04}' for l in range(Nlayer)]
        # print(nodes)
        # pos = {f'p{i:02}{l:04}': (i, l) for l in range(Nlayer)}
        # print(pos)
        # G.add_nodes_from(nodes, pos=pos)
        if i > 0:
            edges = [(f'p{i-1:02}{l:04}', f'p{i:02}{2 * l:04}') for l in range(Nlayer//2)]
            G.add_edges_from(edges)
            edges = [(f'p{i-1:02}{l:04}', f'p{i:02}{2 * l + 1:04}') for l in range(Nlayer//2)]
            G.add_edges_from(edges)

    print('nodes')
    print(G.nodes.data())
    print('positions')
    print(nx.get_node_attributes(G, 'pos'))
    print('edges')
    print(G.edges.data())
    return G


def update_node_type(G, node):
    print('update node:', node)
    edges = list(G.out_edges(nbunch=[node, ]))
    n0 = edges[0][1]
    n1 = edges[1][1]
    # print(edges)
    # print(edges[0])
    # print(n0, n1)
    # print(G.nodes[n0]['nodetype'])
    if G.nodes[n0]['nodetype'] == 'NA':
        update_node_type(G, n0)
    if G.nodes[n1]['nodetype'] == 'NA':
        update_node_type(G, n1)
    n0t = G.nodes[n0]['nodetype']
    n1t = G.nodes[n1]['nodetype']
    if n0t == 'rate0' and n1t == 'rate0':
        G.nodes[node]['nodetype'] = 'rate0'
    elif n0t == 'rate1' and n1t == 'rate1':
        G.nodes[node]['nodetype'] = 'rate1'
    elif n0t == 'rate0' and n1t == 'rate1' and len(list(G.out_edges(nbunch=[n0, ]))) == 0:
        G.nodes[node]['nodetype'] = 'REP'
    elif n0t == 'rate0' and n1t == 'REP':
        G.nodes[node]['nodetype'] = 'REP'
    elif n0t == 'REP' and n1t == 'rate1':
        G.nodes[node]['nodetype'] = 'SPC'
    elif n0t == 'SPC' and n1t == 'rate1':
        G.nodes[node]['nodetype'] = 'SPC'
    else:
        G.nodes[node]['nodetype'] = 'rateR'
    return G


def main():
    n = 4
    N = int(2 ** n)
    snr = 0.
    cc = ChannelConstructorGaussianApproximationDai(N, snr)
    capacities = cc.getCapacities()
    frozen_bit_positions = np.argsort(capacities)[0:N//2]
    print(frozen_bit_positions)

    G = create_polar_tree_graph(N, frozen_bit_positions)
    rootnode = [n for n,d in G.in_degree() if d==0][0]
    print(rootnode)
    G = update_node_type(G, rootnode)

    labels = {}
    colors = {}

    colormap = {'rate0': 'black', 'rate1': 'red', 'REP': 'gray', 'SPC': 'cyan', 'rateR': 'blue'}

    for node in list(G.nodes):
        print(node, G.nodes[node])
        G.nodes[node]['label'] = G.nodes[node]['nodetype']
        colors[node] = colormap[G.nodes[node]['nodetype']]
        labels[G.nodes[node]['nodetype']] = labels.get(G.nodes[node]['nodetype'], 0) + 1

    print(labels)
    legend_elements = []
    for label in labels.keys():
        legend_elements.append(Line2D([0], [0], marker='o', color=colormap[label], label=label,
                           markersize=10, linewidth=0))

    nodes = nx.draw_networkx_nodes(G, nx.get_node_attributes(G, 'pos'), node_size=50, node_color=colors.values())
    edges = nx.draw_networkx_edges(G, nx.get_node_attributes(G, 'pos'), arrows=False)
    plt.legend(handles=legend_elements)
    ax = plt.gca()
    ax.set_axis_off()
    plt.tight_layout()

    plt.savefig(f'polar_code_tree_N{N}_SNR{snr:.0f}.pgf')
    plt.show()


if __name__ == '__main__':
    main()
