#!/usr/bin/env python3
# -*- coding: utf-8 -*-
#
# Copyright 2020 Johannes Demel.
#
# SPDX-License-Identifier: GPL-3.0-or-later
#

import numpy as np
import unittest
import binascii

import pypolar


class DetectorTests(unittest.TestCase):
    def setUp(self):
        self.cmac_lens = (
            8,
            16,
            32,
            64,
            128,
        )

    def tearDown(self):
        pass

    def test_001_init(self):
        pypolar.Detector(32, "cRc")
        crc_sizes = [
            8,
            32,
        ]
        for s in crc_sizes:
            pypolar.Detector(s, "crc")

        pypolar.Detector(32, "cMaC")
        for s in self.cmac_lens:
            pypolar.Detector(s, "CmAc")

    def test_002_crc8_gen(self):
        # https://crccalc.com/  Use 'CRC-8'
        # Polynomial: 0x07
        # Input reflected: False
        # Output reflected: False
        # Initial value: 0x00
        # Final XOR value: 0x00
        det = pypolar.Detector(8, "cRc")

        msg = "TestFooB"
        ref = "0xc2"
        msg = np.array([ord(i) for i in msg])
        res = det.generate(msg)
        self.assertEqual(int(ref, 16), res[-1])

        msg = "FooBarPolar"
        ref = "0xa1"
        msg = np.array([ord(i) for i in msg])
        res = det.generate(msg)
        self.assertEqual(int(ref, 16), res[-1])

    def test_003_crc8_check(self):
        det = pypolar.Detector(8, "cRc")

        msg = "ChaoticLama"
        msg = np.array([ord(i) for i in msg])
        ref = "0x67"
        ref = np.append(msg, int(ref, 16))
        self.assertTrue(det.check(ref))
        refFalse = np.append(msg, 42)
        self.assertFalse(det.check(refFalse))

        msg = "NeverListenToTheVoid!"
        msg = np.array([ord(i) for i in msg])
        ref = "0x69"
        ref = np.append(msg, int(ref, 16))
        self.assertTrue(det.check(ref))
        refFalse = np.append(msg, 42)
        self.assertFalse(det.check(refFalse))

    def test_004_crc32_gen(self):
        # http://www.sunshine2k.de/coding/javascript/crc/crc_js.html
        # Polynimial: 0x1EDC6F41 (CRC-32C)
        # Input reflected: True
        # Output reflected: True
        # Initial value: 0x0000 0000
        # Final XOR value: 0x0000 0000
        # Input must be a multiple of 4 byte!
        # Read result in reverse!
        det = pypolar.Detector(32, "cRc")

        msg = "Test"
        # 19 E2 2D 8C
        ref = [
            "0x8c",
            "0x2d",
            "0xe2",
            "0x19",
        ]
        ref = [int(i, 16) for i in ref]
        msg = np.array([ord(i) for i in msg])
        res = det.generate(msg)
        self.assertListEqual(list(msg), list(res[:-4]))
        self.assertListEqual(ref, list(res[-4:]))

        msg = "FooBarPolarT"
        # C9 62 AC 38
        ref = [
            "0x38",
            "0xac",
            "0x62",
            "0xc9",
        ]
        ref = [int(i, 16) for i in ref]
        msg = np.array([ord(i) for i in msg])
        res = det.generate(msg)
        self.assertListEqual(list(msg), list(res[:-4]))
        self.assertListEqual(ref, list(res[-4:]))

    def test_005_crc32_check(self):
        det = pypolar.Detector(32, "cRc")

        msg = "DisgustinRoastedWhip"
        # FE D6 0B D0
        ref = [
            "0xd0",
            "0x0b",
            "0xd6",
            "0xfe",
        ]
        ref = [int(i, 16) for i in ref]
        msg = np.array([ord(i) for i in msg])
        ref = np.concatenate((msg, ref))
        self.assertTrue(det.check(ref))

        for i in range(ref.size):
            refFalse = np.copy(ref)
            refFalse[i] = 0
            self.assertFalse(det.check(refFalse))

    def test_006_cmac(self):
        msg = "ChaoticLama"
        msg = np.array([ord(i) for i in msg])
        for s in self.cmac_lens:
            det = pypolar.Detector(s, "cmAC")
            res = det.generate(msg)
            print(msg)
            print(res)
            self.assertListEqual(list(msg), list(res[: -(s // 8)]))
            self.assertTrue(det.check(res))

            # for i in range(res.size):
            #     refFalse = np.copy(res)
            #     refFalse[i] = 424242
            #     self.assertFalse(det.check(refFalse))

    def test_007_crc16_gen(self):
        # http://www.sunshine2k.de/coding/javascript/crc/crc_js.html use: CRC16_CCITT_FALSE
        # https://crccalc.com/
        # Polynimial: 0x1021 (CRC-16/CCITTFALSE)
        # Input reflected: false
        # Output reflected: false
        # Initial value: 0x1111 1111
        # Final XOR value: 0x0000 0000
        det = pypolar.Detector(16, "cRc")
        print("CRC-16 generator test")
        msg = "Test"
        # 28 88
        ref = [
            "0x28",
            "0x88",
        ]
        ref = [int(i, 16) for i in ref]
        msg = np.array([ord(i) for i in msg])
        print([hex(i) for i in msg])
        res = det.generate(msg)
        print([hex(i) for i in res])
        self.assertListEqual(list(msg), list(res[:-2]))
        self.assertListEqual(ref, list(res[-2:]))

        msg = "RIPloPTiger"
        # 69 6F
        ref = [
            "0x69",
            "0x6f",
        ]
        ref = [int(i, 16) for i in ref]
        msg = np.array([ord(i) for i in msg])
        print([hex(i) for i in msg])
        res = det.generate(msg)
        print([hex(i) for i in res])
        self.assertListEqual(list(msg), list(res[:-2]))
        self.assertListEqual(ref, list(res[-2:]))

    def test_007_crc16nr_gen(self):
        # http://www.sunshine2k.de/coding/javascript/crc/crc_js.html use: CRC16_XMODEM
        # https://crccalc.com/
        # Polynimial: 0x1021 (CRC-16/XMODEM)
        # Input reflected: false
        # Output reflected: false
        # Initial value: 0x0000 0000
        # Final XOR value: 0x0000 0000
        print("CRC-16NR generator test")

        det = pypolar.Detector(16, "cRcNr")
        self.assertEqual(det.getCheckBitCount(), 16)
        self.assertEqual(det.getType(), "CRCNR")
        print(f"Test {det.getType()}-{det.getCheckBitCount()}")

        msg = "Test"
        # 28 88
        ref = [
            "0xac",
            "0x48",
        ]
        ref = [int(i, 16) for i in ref]
        msg = np.array([ord(i) for i in msg])
        print([hex(i) for i in msg])
        res = det.generate(msg)
        print([hex(i) for i in res])
        self.assertListEqual(list(msg), list(res[:-2]))
        self.assertListEqual(ref, list(res[-2:]))

        msg = "RIPloPTiger"
        # 69 6F
        ref = [
            "0xbd",
            "0x60",
        ]
        ref = [int(i, 16) for i in ref]
        msg = np.array([ord(i) for i in msg])
        print([hex(i) for i in msg])
        res = det.generate(msg)
        print([hex(i) for i in res])
        self.assertListEqual(list(msg), list(res[:-2]))
        self.assertListEqual(ref, list(res[-2:]))

    def test_008_crc16_check(self):
        det = pypolar.Detector(16, "cRc")

        msg = "DisgustinRoastedWhip"
        # A3 2B
        ref = [
            "0xa3",
            "0x2b",
        ]
        ref = [int(i, 16) for i in ref]
        msg = np.array([ord(i) for i in msg])
        ref = np.concatenate((msg, ref))
        print(det.generate(msg))
        self.assertTrue(det.check(ref))

        for i in range(ref.size):
            refFalse = np.copy(ref)
            refFalse[i] = 0
            self.assertFalse(det.check(refFalse))

    def get_byte_reference(self, msg, expected):
        vec_msg = np.array([ord(i) for i in msg], dtype=np.uint8)
        ref = np.array([int(i, 16) for i in expected], dtype=np.uint8)

        hex_str_msg = " ".join(f"{ord(i):02x}" for i in msg)

        print(f'str: "{msg}" -> 0x {hex_str_msg}')

        return vec_msg, ref

    def test_024_crc24c_gen(self):
        # http://www.ghsi.de/pages/subpages/Online%20CRC%20Calculation/index.php?Polynom=1101100101011000100010111&Message=E100CAFE
        # cf. CRC24C TS 38.212 Sec 5.1
        print("CRC-24C NR generator test")
        det = pypolar.Detector(24, "cRcNr")
        self.assertEqual(det.getCheckBitCount(), 24)
        self.assertEqual(det.getType(), "CRCNR")
        print(f"Test {det.getType()}-{det.getCheckBitCount()}")

        msg = "123456789"
        expected = ["0xf4", "0x82", "0x79"]
        vec_msg, ref = self.get_byte_reference(msg, expected)

        res = det.generate(vec_msg)
        checksum = res[-3:]
        self.assertListEqual(list(vec_msg), list(res[:-3]))
        self.assertListEqual(list(ref), list(res[-3:]))

        # print(vec_msg)
        # print(res)
        # print(checksum)
        # print("\t".join(f"0x{i:02x}" for i in checksum))

        msg = "CommsIsAComplicatedMatter"
        expected = ["0xde", "0xe1", "0xf8"]
        vec_msg, ref = self.get_byte_reference(msg, expected)

        res = det.generate(vec_msg)
        checksum = res[-3:]
        self.assertListEqual(list(vec_msg), list(res[:-3]))
        self.assertListEqual(list(ref), list(res[-3:]))

    def test_025_crc24c_calculate(self):
        print("CRC-24C NR calculator test")
        det = pypolar.Detector(24, "cRcNr")
        self.assertEqual(det.getCheckBitCount(), 24)
        self.assertEqual(det.getType(), "CRCNR")
        print(f"Test {det.getType()}-{det.getCheckBitCount()}")

        data = np.array([0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 0, 1, 1, 1, 1, 0, 0, 1, 0, 0, 1, 0], dtype=np.uint8)
        packed_data = np.packbits(data)
        reference = 0x1fb065
        print(f'0x{reference:06x}')
        checksum = det.calculate(packed_data, data.size)
        print(f'0x{checksum:06x}')
        self.assertEqual(reference, checksum)

        data = np.array([1, 0, 1, 1, 1, 1, 0, 1, 0, 0, 0, 1, 0], dtype=np.uint8)
        packed_data = np.packbits(data)

        reference = 0xf70030
        print(f'0x{reference:06x}')
        checksum = det.calculate(packed_data, data.size)
        print(f'0x{checksum:06x}')
        self.assertEqual(reference, checksum)

    def test_060_crc6_gen(self):
        # http://www.ghsi.de/pages/subpages/Online%20CRC%20Calculation/index.php?Polynom=1100001&Message=31+32+33+34+35+36+37+38+39
        # cf. CRC6 TS 38.212 Sec 5.1
        print("CRC-6 NR generator test")
        det = pypolar.Detector(6, "cRcNr")
        self.assertEqual(det.getCheckBitCount(), 6)
        self.assertEqual(det.getType(), "CRCNR")
        print(f"Test {det.getType()}-{det.getCheckBitCount()}")

        msg = "123456789"
        expected = [
            "0x15",
        ]
        vec_msg, ref = self.get_byte_reference(msg, expected)

        res = det.generate(vec_msg)
        self.assertEqual(res.size, vec_msg.size + 1)
        checksum = res[-1:]
        self.assertListEqual(list(vec_msg), list(res[:-1]))
        self.assertListEqual(list(ref), list(res[-1:]))


        msg = "CommsIsAComplicatedMatter"
        expected = [
            "0x04",
        ]
        vec_msg, ref = self.get_byte_reference(msg, expected)

        res = det.generate(vec_msg)
        checksum = res[-1:]
        self.assertListEqual(list(vec_msg), list(res[:-1]))
        self.assertListEqual(list(ref), list(res[-1:]))

    def test_061_crc6_calculate(self):
        print("CRC-6 NR calculator test")
        det = pypolar.Detector(6, "cRcNr")
        self.assertEqual(det.getCheckBitCount(), 6)
        self.assertEqual(det.getType(), "CRCNR")
        print(f"Test {det.getType()}-{det.getCheckBitCount()}")

        data = np.array([0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 0, 1, 1, 1, 1, 0, 0, 1, 0, 0, 1, 0], dtype=np.uint8)
        packed_data = np.packbits(data)
        reference = 0x02

        print(f'0x{reference:02x}')
        checksum = det.calculate(packed_data, data.size)
        print(f'0x{checksum:02x}')
        self.assertEqual(reference, checksum)

        data = np.array([1, 0, 1, 1, 1, 1, 0, 1, 0, 0, 0, 1, 0], dtype=np.uint8)
        packed_data = np.packbits(data)
        reference = 0x2f

        print(f'0x{reference:02x}')
        checksum = det.calculate(packed_data, data.size)
        print(f'0x{checksum:02x}')
        self.assertEqual(reference, checksum)

    def test_011_crc11_gen(self):
        # http://www.ghsi.de/pages/subpages/Online%20CRC%20Calculation/index.php?Polynom=111000100001&Message=31+32+33+34+35+36+37+38+39
        # cf. CRC11 TS 38.212 Sec 5.1
        print("CRC-11 NR generator test")
        det = pypolar.Detector(11, "cRcNr")
        self.assertEqual(det.getCheckBitCount(), 11)
        self.assertEqual(det.getType(), "CRCNR")
        print(f"Test {det.getType()}-{det.getCheckBitCount()}")

        msg = "123456789"
        expected = [
            "0x05",
            "0xca",
        ]
        vec_msg, ref = self.get_byte_reference(msg, expected)

        res = det.generate(vec_msg)
        self.assertEqual(res.size, vec_msg.size + 2)
        checksum = res[-2:]
        self.assertListEqual(list(vec_msg), list(res[:-2]))
        self.assertListEqual(list(ref), list(res[-2:]))

        # print(vec_msg)
        # print(res)
        # print(checksum)
        # print("\t".join(f"0x{i:02x}" for i in checksum))

        msg = "CommsIsAComplicatedMatter"

        expected = [
            "0x04",
            "0x4e",
        ]
        vec_msg, ref = self.get_byte_reference(msg, expected)

        res = det.generate(vec_msg)
        self.assertEqual(res.size, vec_msg.size + 2)
        checksum = res[-2:]
        self.assertListEqual(list(vec_msg), list(res[:-2]))
        self.assertListEqual(list(ref), list(res[-2:]))

    def test_012_crc11_calculate(self):
        print("CRC-11 NR calculator test")
        det = pypolar.Detector(11, "cRcNr")
        self.assertEqual(det.getCheckBitCount(), 11)
        self.assertEqual(det.getType(), "CRCNR")
        print(f"Test {det.getType()}-{det.getCheckBitCount()}")

        data = np.array([0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 0, 1, 1, 1, 1, 0, 0, 1, 0, 0, 1, 0], dtype=np.uint8)
        packed_data = np.packbits(data)
        reference = 0x0608

        print(f'0x{reference:03x}')
        checksum = det.calculate(packed_data, data.size)
        print(f'0x{checksum:03x}')
        self.assertEqual(reference, checksum)

        data = np.array([1, 0, 1, 1, 1, 1, 0, 1, 0, 0, 0, 1, 0], dtype=np.uint8)
        packed_data = np.packbits(data)
        reference = 0x06c8

        print(f'0x{reference:03x}')
        checksum = det.calculate(packed_data, data.size)
        print(f'0x{checksum:03x}')
        self.assertEqual(reference, checksum)

if __name__ == "__main__":
    unittest.main(failfast=False)
