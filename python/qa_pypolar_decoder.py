#!/usr/bin/env python3
# -*- coding: utf-8 -*-
#
# Copyright 2020 Johannes Demel.
#
# SPDX-License-Identifier: GPL-3.0-or-later
#

import numpy as np
import unittest
from polar_code_tools import (
    design_snr_to_bec_eta,
    calculate_bec_channel_capacities,
    get_frozenBitPositions,
    get_polar_generator_matrix,
)
from polar_code_tools import get_expanding_matrix
from channel_construction import ChannelConstructorBetaExpansion
import pypolar


class PolarDecoderTests(unittest.TestCase):
    def setUp(self):
        pass

    def tearDown(self):
        pass

    def test_002_frozen_bit_positions(self):
        for snr in np.arange(-1.5, 3.5, 0.25):
            for inv_coderate in np.array([8, 6, 5, 4, 3, 2, 1.5, 1.2]):
                for n in range(6, 11):
                    N = 2 ** n
                    K = int(N / inv_coderate)
                    # print(N, K, inv_coderate)
                    cf = pypolar.frozen_bits(N, K, snr)
                    eta = design_snr_to_bec_eta(snr, 1.0 * K / N)
                    polar_capacities = calculate_bec_channel_capacities(eta, N)
                    pf = get_frozenBitPositions(polar_capacities, N - K)
                    pf = np.sort(pf)
                    decoder = pypolar.PolarDecoder(N, 1, cf, "mixed")

                    pd = decoder.frozenBits()

                    self.assertListEqual(cf, pd)
                    self.assertListEqual(cf, list(pf))

    def check_matrix_domination_contiguity(self, N, f):
        f = np.array(f)
        G = get_polar_generator_matrix(int(np.log2(N)))
        em = get_expanding_matrix(f, N)

        bpi = np.dot(em, np.dot(G, em.T) % 2) % 2
        r = np.dot(bpi, bpi) % 2

        return np.all(r == np.identity(N - len(f)))

    def initialize_encoder(self, N, K, snr):
        try:
            np.seterr(invalid="raise")
            cc = ChannelConstructorBetaExpansion(N, snr)
        except FloatingPointError:
            np.seterr(invalid="warn")
            cc = ChannelConstructorBetaExpansion(N, snr)

        f = np.sort(cc.getSortedChannels())[0 : N - K]
        p = pypolar.PolarEncoder(N, f)
        return p

    def test_006_cpp_decoder_impls(self):
        print("TEST: CPP Decoder")
        snr = -1.0
        test_size = 2 ** np.arange(7, 11, dtype=int)
        # test_size = np.array([4, 5, 6, 8, 9, 10], dtype=int)
        for N in test_size:
            self.validate_decoder(N, int(N * 0.75), snr)
            self.validate_decoder(N, N // 2, snr)
            self.validate_decoder(N, N // 4, snr)
            self.validate_decoder(N, N // 8, snr)

    def validate_decoder(self, N, K, snr):
        self.run_decoder(N, K, 1, snr, "char")
        self.run_decoder(N, K, 1, snr, "float")
        self.run_decoder(N, K, 4, snr, "float")
        self.run_decoder(N, K, 8, snr, "float")
        self.run_decoder(N, K, 4, snr, "mixed")

    def test_007_scan_decoder(self):
        snr = -1.0
        test_size = 2 ** np.arange(7, 10, dtype=int)
        for N in test_size:
            self.run_decoder(N, N // 2, 8, snr, "scan", 16, 3)

    def run_decoder(self, N, K, L, snr, decType, detector_size=8, iterations=10):
        with self.subTest(
            N=N,
            K=K,
            L=L,
            snr=snr,
            decType=decType,
            detector_size=detector_size,
            iterations=iterations,
        ):
            print(
                f"Decoder CPP test ({N=:4}, {K=:3}, {L=}, {decType=:5}, CRC{detector_size:2}) -> {snr}dB"
            )
            p = self.initialize_encoder(N, K, snr)

            if not self.check_matrix_domination_contiguity(N, p.frozenBits()):
                print("invalid code parameters!")
                return

            f = p.frozenBits()
            dec0 = pypolar.PolarDecoder(N, L, f, decType)

            p.setErrorDetection(detector_size)
            dec0.setErrorDetection(detector_size)
            self.assertListEqual(f, dec0.frozenBits())

            for i in np.arange(iterations):
                u = np.random.randint(0, 2, K).astype(dtype=np.uint8)
                d = np.packbits(u)

                # The pythonic method
                cw_pack = p.encode_vector(d)
                b = np.unpackbits(cw_pack)
                llrs = -2.0 * b + 1.0
                llrs += np.random.uniform(-0.2, 0.2, size=llrs.size)
                llrs = llrs.astype(dtype=np.float32)

                dhat0 = dec0.decode_vector(llrs)
                np.testing.assert_equal(dhat0, d)


if __name__ == "__main__":
    unittest.main(failfast=False)
