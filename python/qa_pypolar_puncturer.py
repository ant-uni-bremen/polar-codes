#!/usr/bin/env python3
# -*- coding: utf-8 -*-
#
# Copyright 2020 Johannes Demel.
#
# SPDX-License-Identifier: GPL-3.0-or-later
#

import numpy as np
import unittest

from polar_code_tools import design_snr_to_bec_eta
from polar_code_tools import calculate_bec_channel_capacities
from polar_code_tools import get_frozenBitPositions

import pypolar


def get_diff_positions(block_length, pos):
    return np.setdiff1d(np.arange(block_length), pos).astype(pos.dtype)


class PuncturerTests(unittest.TestCase):
    def setUp(self):
        pass

    def tearDown(self):
        pass

    def test_001_setup(self):
        for n in range(5, 11):
            N = 2 ** n
            K = N // 2
            eta = design_snr_to_bec_eta(0.0, 1.0)
            polar_capacities = calculate_bec_channel_capacities(eta, N)
            f = np.sort(get_frozenBitPositions(polar_capacities, N - K))

            outputPositions0 = get_diff_positions(N, f[0:N//4])

            punc0 = pypolar.Puncturer(N - (N // 4), f)
            self.assertEqual(punc0.parentBlockLength(), N)
            self.assertEqual(punc0.blockLength(), N - (N // 4))
            self.assertListEqual(punc0.blockOutputPositions(),
                                 outputPositions0.tolist())

            outputPositions1 = get_diff_positions(N, f[0:N//8])

            punc1 = pypolar.Puncturer(N - (N // 8), f)
            self.assertEqual(punc1.parentBlockLength(), N)
            self.assertEqual(punc1.blockLength(), N - (N // 8))

            self.assertListEqual(punc1.blockOutputPositions(),
                                 outputPositions1.tolist())

    def test_002_puncture_bits(self):
        N = 2 ** 6
        K = N // 2
        eta = design_snr_to_bec_eta(0.0, 1.0)
        polar_capacities = calculate_bec_channel_capacities(eta, N)
        f = np.sort(get_frozenBitPositions(polar_capacities, N - K))

        outputPositions = get_diff_positions(N, f[0:N//4])

        punc = pypolar.Puncturer(N - (N // 4), f)

        vec = np.random.randint(0, 256, N // 8, dtype=np.uint8)
        unpvec = np.unpackbits(vec)
        unpres = punc.puncture(unpvec)
        res = punc.puncturePacked(vec)
        self.assertListEqual(np.unpackbits(res).tolist(), unpres.tolist())

        ref = unpvec[outputPositions]
        self.assertListEqual(ref.tolist(), unpres.tolist())

        fvec = np.arange(N, dtype=np.float32)
        fres = punc.puncture(fvec)
        fref = fvec[outputPositions]
        self.assertListEqual(fref.tolist(), fres.tolist())

        dvec = np.arange(N, dtype=np.float64)
        dres = punc.puncture(dvec)
        dref = dvec[outputPositions]
        self.assertListEqual(dref.tolist(), dres.tolist())

    def test_003_depuncture_bits(self):
        N = 2 ** 6
        K = N // 2
        eta = design_snr_to_bec_eta(0.0, 1.0)
        polar_capacities = calculate_bec_channel_capacities(eta, N)
        f = np.sort(get_frozenBitPositions(polar_capacities, N - K))

        punc = pypolar.Puncturer(N - (N // 4), f)

        outputPositions = get_diff_positions(N, f[0:N//4])

        vec = np.random.normal(0.0, 1.0, N - (N // 4)).astype(np.float32)
        res = punc.depuncture(vec)
        ref = np.zeros(N, dtype=res.dtype)
        ref[outputPositions] = vec
        self.assertListEqual(ref.tolist(), res.tolist())

        vec = np.random.normal(0.0, 1.0, N - (N // 4)).astype(np.float64)
        res = punc.depuncture(vec)
        ref = np.zeros(N, dtype=res.dtype)
        ref[outputPositions] = vec
        self.assertListEqual(ref.tolist(), res.tolist())

        vec = np.random.randint(0, 256, N - (N // 4), dtype=np.uint8)
        res = punc.depuncture(vec)
        ref = np.zeros(N, dtype=res.dtype)
        ref[outputPositions] = vec
        self.assertListEqual(ref.tolist(), res.tolist())


if __name__ == '__main__':
    unittest.main(failfast=False)
