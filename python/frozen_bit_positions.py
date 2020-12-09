#!/usr/bin/env python3
# -*- coding: utf-8 -*-
#
# Copyright 2020 Johannes Demel.
#
# SPDX-License-Identifier: GPL-3.0-or-later
#

from pathlib import Path
import numpy as np
try:
    from pypolar import frozen_bits
except ImportError:
    from .polarcode_python import frozen_bits
import pprint

try:
    from channel_construction import ChannelConstructorGaussianApproximationDai, ChannelConstructorBetaExpansion
except ImportError:
    from .channel_construction import ChannelConstructorGaussianApproximationDai, ChannelConstructorBetaExpansion


class FrozenBitPositions:
    def __init__(self, block_length, info_length):
        self._block_length = block_length
        self._info_length = info_length
        self._frozen_bit_positions = None

    def frozen_bit_positions(self):
        if self._frozen_bit_positions is None:
            self._frozen_bit_positions = self._generate_frozen_bit_positions()
            # ensure frozen bit positions are sorted in ascending order!
            self._frozen_bit_positions = np.sort(self._frozen_bit_positions)
        return self._frozen_bit_positions

    def _generate_frozen_bit_positions(self):
        raise NotImplementedError('Implement this is in a subclass!')


class FrozenBitPositionsBB(FrozenBitPositions):
    '''
    Frozen Bit positions based on the Bhattacharyya Bounds method.
    Basically the method proposed by Arikan and somwhat refined in
    Vangala et al. "A Comparative Study of Polar Code Constructions for the AWGN Channel"
    '''

    def __init__(self, block_length, info_length, dSNR=0.0):
        super().__init__(block_length, info_length)
        self._dSNR = dSNR

    def _generate_frozen_bit_positions(self):
        return np.array(frozen_bits(self._block_length, self._info_length, self._dSNR))


class FrozenBitPositionsGA(FrozenBitPositions):
    '''
    Frozen Bit positions based on Gaussian Approximation
    This specific version is based on
    Dai et al. "Does Gaussian Approximation Work Well for the Long-Length Polar Code Construction?"
    '''

    def __init__(self, block_length, info_length, dSNR=0.0):
        super().__init__(block_length, info_length)
        self._dSNR = dSNR

    def _generate_frozen_bit_positions(self):
        fbp = ChannelConstructorGaussianApproximationDai(
            self._block_length, self._dSNR).frozenBitPositions(self._block_length - self._info_length)
        return np.sort(fbp)


class FrozenBitPositionsPW(FrozenBitPositions):
    '''
    Frozen Bit positions based on Polarization Weights or beta-expansion
    '''

    def __init__(self, block_length, info_length, dSNR=0.0):
        super().__init__(block_length, info_length)
        self._reliabilities = None

    def _generate_frozen_bit_positions(self):
        if self._reliabilities is None:
            self._reliabilities = ChannelConstructorBetaExpansion(
                self._block_length, 0.).getSortedChannels()
        fbp = ChannelConstructorBetaExpansion(
            self._block_length, 0.).frozenBitPositions(self._block_length - self._info_length)
        return np.sort(fbp)


def find_aff3ct_config_path():
    try:
        import aff3ct
        base_path = Path(aff3ct.AFF3CT_CONFIG_SEARCH_PATH)
        while not base_path.exists() and base_path.name:
            base_path = base_path.parent

    except ImportError:
        base_path = Path(
            '~/src/pybind11_aff3ct/lib/aff3ct/conf/cde/awgn_polar_codes/TV/').expanduser()

    if base_path.name == 'share':
        base_path = list(base_path.glob('aff3ct*/conf'))[0]
    base_path = base_path / "cde" / "awgn_polar_codes" / "TV"
    return base_path


class FrozenBitPositionsDE(FrozenBitPositions):
    '''
    Frozen bit positions based on Density Evolution or Tal-Vardy's algorithm.
    The algorithm itself is not implemented. Only the results are importable.
    The details of the file format are part of the aff3ct library.

    FIXME! This is a broken implementation!
    It depends on an inherit assumption where to look for files.
    '''

    def __init__(self, block_length, info_length, dSNR=0.0):
        super().__init__(block_length, info_length)
        self._dSNR = dSNR
        self._log_block_length = int(np.log2(block_length))

        self._base_path = find_aff3ct_config_path()
        print(self._base_path)
        self._path = self._base_path / Path(str(self._log_block_length))
        if not self._path.exists():
            raise ValueError(f"Can not find configuration path '{self._path}' for DE!")

    def _load_file(self, filename):
        result = {}
        with open(filename) as f:
            for i, line in enumerate(f):
                l = line.strip('\n')
                if i == 0:
                    result['block_length'] = int(l)
                    assert result['block_length'] == self._block_length
                elif i == 1:
                    result['channel'] = str(l)
                    assert result['channel'] == 'awgn'
                elif i == 2:
                    result['sigma'] = float(l)
                elif i == 3:
                    result['positions'] = np.array([int(p) for p in l.split()])
                else:
                    raise ValueError('Unexpected File!')
        return result

    def _load_files(self):
        candidates = {}
        bestfile = None
        bestsigma = 10. ** 10
        sigma = 10. ** (-1. * self._dSNR / 10.)
        for f in self._path.iterdir():
            # print(f)
            candidates[f] = self._load_file(f)
            cs = candidates[f]['sigma']
            if np.abs(bestsigma - sigma) > np.abs(cs - sigma):
                bestsigma = cs
                bestfile = f
            # print(cs)
        # print(sigma, bestsigma)
        return candidates[bestfile]['positions']

    def _generate_frozen_bit_positions(self):
        positions = self._load_files()
        return np.sort(positions[self._info_length:])


class FrozenBitPositions5G(FrozenBitPositions):
    '''
    3GPP TS 38.212 version 15.8.0 Release 15
    ETSI TS 138 212 V15.8.0 (2020-01)
    Table 5.3.1.2-1
    5G reliability table based frozen bits.
    '''

    def __init__(self, block_length, info_length, dSNR=0.0):
        assert block_length <= 1024  # Not defined for larger codes!
        super().__init__(block_length, info_length)
        filebase = Path(__file__).resolve().parent
        self._file = filebase / Path('5g_polar_code_reliability_table.csv')
        self._file.resolve()
        assert self._file.exists()
        self._reliabilities = None

    def _load_file(self):
        table = []
        with open(self._file) as f:
            for l in f:
                values = np.array([int(i) for i in l.strip().split(',')])
                table.append(values)
            table = np.vstack(table)
        return table

    def _load_reliabilities(self):
        if self._reliabilities is not None:
            return
        table = self._load_file()
        pos = table[:, 0::2]
        assert np.all(pos.T.flatten() == np.arange(1024, dtype=int))
        reliabilities = table[:, 1::2]
        self._reliabilities = reliabilities.T.flatten()
        assert self._reliabilities.size == 1024

    def _load_frozen_bit_positions(self):
        if self._frozen_bit_positions is not None:
            return self._frozen_bit_positions
        if self._reliabilities is None:
            self._load_reliabilities()

        rels = self._reliabilities[np.where(
            self._reliabilities < self._block_length)]
        assert rels.size == self._block_length
        frozen_length = self._block_length - self._info_length
        self._frozen_bit_positions = np.sort(rels[0:frozen_length])
        return self._frozen_bit_positions

    def _generate_frozen_bit_positions(self):
        return self._load_frozen_bit_positions()


def get_frozen_bit_generator(frozen_bit_generator, block_length, info_length, dSNR):
    generators = {'DE': FrozenBitPositionsDE,
                  'BB': FrozenBitPositionsBB,
                  '5G': FrozenBitPositions5G,
                  'GA': FrozenBitPositionsGA,
                  'BE': FrozenBitPositionsPW,
                  'PW': FrozenBitPositionsPW}
    assert frozen_bit_generator in generators.keys()
    return generators[frozen_bit_generator](block_length, info_length, dSNR)


def main():
    gen = get_frozen_bit_generator('DE', 1024, 512, 0.0)
    return
    fbp = FrozenBitPositions5G(1024, 512, 3.)
    # print(fbp._file)
    # fbp._load_reliabilities()
    # p = fbp._load_frozen_bit_positions()
    frozen_bit_positions = fbp.frozen_bit_positions()
    # print(frozen_bit_positions)

    sorted_channels = fbp._reliabilities

    fbp_be = FrozenBitPositionsPW(256, 64, 0.)
    frozen_bit_positions_be = fbp_be.frozen_bit_positions()
    print(', '.join([str(i) for i in frozen_bit_positions_be]))
    return
    # sorted_channels_be = fbp_be._reliabilities
    # print(sorted_channels)
    # print(sorted_channels_be)
    # print(frozen_bit_positions == frozen_bit_positions_be)
    # print(np.all(sorted_channels == sorted_channels_be))

    for i in (32, 64, 128, ):
        print(i)
        ref = sorted_channels[np.where(sorted_channels < i)]
        print(ref)

        fbp_be = FrozenBitPositionsPW(i, 0, 0.)
        _ = fbp_be.frozen_bit_positions()
        res = fbp_be._reliabilities
        print(res)

        print(res == ref)
        print(np.all(res == ref))

    return

    for g in ('DE', 'GA', 'BB', '5G', 'BE'):
        fbp = get_frozen_bit_generator(g, 512, 256, 3.)
        # print(fbp._file)
        # fbp._load_reliabilities()
        # p = fbp._load_frozen_bit_positions()
        frozen_bit_positions = fbp.frozen_bit_positions()
        print(frozen_bit_positions)


if __name__ == '__main__':
    main()
