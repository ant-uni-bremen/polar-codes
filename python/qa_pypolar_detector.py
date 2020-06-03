#!/usr/bin/env python3
import numpy as np
import unittest
import binascii

import pypolar


class DetectorTests(unittest.TestCase):
    def setUp(self):
        self.cmac_lens = (8, 16, 32, 64, 128, )

    def tearDown(self):
        pass

    def test_001_init(self):
        pypolar.Detector(32, 'cRc')
        crc_sizes = [8, 32, ]
        for s in crc_sizes:
            pypolar.Detector(s, 'crc')

        pypolar.Detector(32, 'cMaC')
        for s in self.cmac_lens:
            pypolar.Detector(s, 'CmAc')

    def test_002_crc8_gen(self):
        # https://crccalc.com/  Use 'CRC-8'
        # Polynomial: 0x07
        # Input reflected: False
        # Output reflected: False
        # Initial value: 0x00
        # Final XOR value: 0x00
        det = pypolar.Detector(8, 'cRc')

        msg = 'TestFooB'
        ref = '0xc2'
        msg = np.array([ord(i) for i in msg])
        res = det.generate(msg)
        self.assertEqual(int(ref, 16), res[-1])

        msg = 'FooBarPolar'
        ref = '0xa1'
        msg = np.array([ord(i) for i in msg])
        res = det.generate(msg)
        self.assertEqual(int(ref, 16), res[-1])

    def test_003_crc8_check(self):
        det = pypolar.Detector(8, 'cRc')

        msg = 'ChaoticLama'
        msg = np.array([ord(i) for i in msg])
        ref = '0x67'
        ref = np.append(msg, int(ref, 16))
        self.assertTrue(det.check(ref))
        refFalse = np.append(msg, 42)
        self.assertFalse(det.check(refFalse))

        msg = 'NeverListenToTheVoid!'
        msg = np.array([ord(i) for i in msg])
        ref = '0x69'
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
        det = pypolar.Detector(32, 'cRc')

        msg = 'Test'
        # 19 E2 2D 8C
        ref = ['0x8c', '0x2d', '0xe2', '0x19', ]
        ref = [int(i, 16) for i in ref]
        msg = np.array([ord(i) for i in msg])
        res = det.generate(msg)
        self.assertListEqual(list(msg), list(res[:-4]))
        self.assertListEqual(ref, list(res[-4:]))

        msg = 'FooBarPolarT'
        # C9 62 AC 38
        ref = ['0x38', '0xac', '0x62', '0xc9', ]
        ref = [int(i, 16) for i in ref]
        msg = np.array([ord(i) for i in msg])
        res = det.generate(msg)
        self.assertListEqual(list(msg), list(res[:-4]))
        self.assertListEqual(ref, list(res[-4:]))

    def test_005_crc32_check(self):
        det = pypolar.Detector(32, 'cRc')

        msg = 'DisgustinRoastedWhip'
        # FE D6 0B D0
        ref = ['0xd0', '0x0b', '0xd6', '0xfe', ]
        ref = [int(i, 16) for i in ref]
        msg = np.array([ord(i) for i in msg])
        ref = np.concatenate((msg, ref))
        self.assertTrue(det.check(ref))

        for i in range(ref.size):
            refFalse = np.copy(ref)
            refFalse[i] = 0
            self.assertFalse(det.check(refFalse))

    def test_006_cmac(self):
        msg = 'ChaoticLama'
        msg = np.array([ord(i) for i in msg])
        for s in self.cmac_lens:
            det = pypolar.Detector(s, 'cmAC')
            res = det.generate(msg)
            print(msg)
            print(res)
            self.assertListEqual(list(msg), list(res[:-(s // 8)]))
            self.assertTrue(det.check(res))

            for i in range(res.size):
                refFalse = np.copy(res)
                refFalse[i] = 42
                self.assertFalse(det.check(refFalse))

    def test_007_crc16_gen(self):
        # http://www.sunshine2k.de/coding/javascript/crc/crc_js.html use: CRC16_CCITT_FALSE
        # https://crccalc.com/
        # Polynimial: 0x1021 (CRC-16/XMODEM) now using: CCITTFALSE
        # Input reflected: false
        # Output reflected: false
        # Initial value: 0x0000 0000
        # Final XOR value: 0x0000 0000
        det = pypolar.Detector(16, 'cRc')
        print('CRC-16 generator test')
        msg = 'Test'
        # 28 88
        ref = ['0x28', '0x88', ]
        ref = [int(i, 16) for i in ref]
        msg = np.array([ord(i) for i in msg])
        print([hex(i) for i in msg])
        res = det.generate(msg)
        print([hex(i) for i in res])
        self.assertListEqual(list(msg), list(res[:-2]))
        self.assertListEqual(ref, list(res[-2:]))

        msg = 'RIPloPTiger'
        # 69 6F
        ref = ['0x69', '0x6f', ]
        ref = [int(i, 16) for i in ref]
        msg = np.array([ord(i) for i in msg])
        print([hex(i) for i in msg])
        res = det.generate(msg)
        print([hex(i) for i in res])
        self.assertListEqual(list(msg), list(res[:-2]))
        self.assertListEqual(ref, list(res[-2:]))

    def test_008_crc16_check(self):
        det = pypolar.Detector(16, 'cRc')

        msg = 'DisgustinRoastedWhip'
        # A3 2B
        ref = ['0xa3', '0x2b', ]
        ref = [int(i, 16) for i in ref]
        msg = np.array([ord(i) for i in msg])
        ref = np.concatenate((msg, ref))
        print(det.generate(msg))
        self.assertTrue(det.check(ref))

        for i in range(ref.size):
            refFalse = np.copy(ref)
            refFalse[i] = 0
            self.assertFalse(det.check(refFalse))


if __name__ == '__main__':
    unittest.main(failfast=False)
