#!/usr/bin/env python3
# -*- coding: utf-8 -*-
#
# Copyright 2020 Johannes Demel.
#
# SPDX-License-Identifier: GPL-3.0-or-later
#

import numpy as np
from scipy import special as sps
import warnings


def db2lin(snr_db):
    return 10. ** (snr_db / 10.)


class ChannelConstructor:
    """docstring for ChannelConstructor"""

    def __init__(self, N, design_snr):
        self._block_power = int(np.log2(N))
        self._block_size = int(2 ** self._block_power)
        self._eta = self._design_snr2eta(design_snr)
        self._sigma_sq = 1. * 10. ** (design_snr / 10.)
        self._sortedChannels = np.arange(self._block_size)
        self._capacities = np.zeros(self._block_size, dtype=np.float64)

    def _design_snr2eta(self, design_snr, coderate=1.0):
        # minimum design snr = -1.5917 corresponds to BER = 0.5
        s = db2lin(design_snr)
        # s = 10. ** (design_snr / 10.)
        # s *= 2. * coderate
        return np.exp(-s)

    def sortedChannels(self, capacities):
        return np.argsort(capacities)

    def frozenBitPositions(self, capacities, n_frozen):
        return self.sortedChannels(capacities)[0:n_frozen]

    def evaluate(self):
        capacities = self.calculate_capacities()
        self._sortedChannels = self.sortedChannels(capacities)

    def getSortedChannels(self):
        return self._sortedChannels

    def getCapacities(self):
        return self._capacities


class ChannelConstructorBhattacharyyaBounds(ChannelConstructor):
    """docstring for ChannelConstructorBhattacharyyaBounds"""

    def __init__(self, N, snr):
        ChannelConstructor.__init__(self, N, snr)
        self._cc_lin = ChannelConstructorBhattacharyyaBoundsLinear(N, snr)
        self._cc_log = ChannelConstructorBhattacharyyaBoundsLog(N, snr)
        self.evaluate()

    def calculate_capacities(self):
        v0 = self._cc_lin.getCapacities()
        v1 = self._cc_log.getCapacities()
        uv0 = np.unique(v0).size
        uv1 = np.unique(v1).size
        print('unique elements lin: {}'.format(uv0))
        # print(np.size(np.where(uv0 == 1.0)))
        # print('unique elements lin: {} vs ln: {}'.format(uv0, uv1))
        if uv0 > uv1:
            self._capacities = v0
            return v0
        else:
            self._capacities = v1
            return v1


class ChannelConstructorBhattacharyyaBoundsLog(ChannelConstructor):
    def __init__(self, N, snr):
        ChannelConstructor.__init__(self, N, snr)
        self.evaluate()

    def calculate_capacities(self):
        self._capacities = v0 = self._calculate_capacities_ln()
        return v0

    def _degrade_ln(self, vals):
        return 2 * vals

    def _upgrade_ln(self, vals):
        l2 = np.log(2.)
        # print(vals, l2, vals - l2)
        return vals + l2 + np.log(1 - np.exp(vals - l2))
        # return vals + l2 + np.logaddexp(np.log(1.), 1. / (vals - l2))
        # return vals + np.logaddexp(-vals, l2)

    def _calculate_channels_ln(self, vals):
        res = np.empty(2 * len(vals), dtype=vals.dtype)
        res[0::2] = self._degrade_ln(vals)
        res[1::2] = self._upgrade_ln(vals)
        return res

    def _calculate_capacities_ln(self):
        vals = np.array([np.log(1. - self._eta), ], dtype=np.float128)
        for i in range(self._block_power):
            # print(vals)
            vals = self._calculate_channels_ln(vals)
        # print('ln unique {}'.format(np.unique(vals).size))
        uv0 = np.unique(vals).size
        print('unique elements ln: {}'.format(uv0))
        r = 1. - np.exp(vals)
        r = np.maximum(r, np.zeros_like(r))
        return r[::-1]


class ChannelConstructorBhattacharyyaBoundsLinear(ChannelConstructor):
    def __init__(self, N, snr):
        ChannelConstructor.__init__(self, N, snr)
        self.evaluate()

    def calculate_capacities(self):
        self._capacities = v0 = self._calculate_capacities_lin()
        return v0

    def _degrade_lin(self, vals):
        return vals ** 2

    def _upgrade_lin(self, vals):
        return 2 * vals - vals ** 2

    def _calculate_channels_lin(self, vals):
        res = np.empty(2 * len(vals), dtype=vals.dtype)
        res[0::2] = self._degrade_lin(vals)
        res[1::2] = self._upgrade_lin(vals)
        return res

    def _calculate_capacities_lin(self):
        vals = np.array([1. - self._eta, ], dtype=np.float128)
        # print(vals)
        for i in range(self._block_power):
            # print(vals)
            vals = self._calculate_channels_lin(vals)
        return vals


class ChannelConstructorGaussianApproximation(ChannelConstructor):
    """docstring for ChannelConstructorBhattacharyyaBounds"""

    def __init__(self, N, snr):
        ChannelConstructor.__init__(self, N, snr)

        self._alpha = -0.4527
        self._beta = 0.0218
        self._gamma = 0.8600

        self._a = 1.0 / self._alpha
        self._b = -self._beta / self._alpha
        self._c = 1.0 / self._gamma

        self.evaluate()

    def calculate_capacities(self):
        v0 = self._calculate_capacities_llr()
        self._capacities = sps.erf(np.sqrt(v0.astype(np.float64) / 2.))
        return v0

    def _phi(self, t):
        phi_pivot = 0.867861
        if t < phi_pivot:
            # print('phi {} ** 2'.format(t))
            sq = t ** 2.
            return np.exp(0.0564 * sq - 0.48560 * t)
        else:
            try:
                # t2 = t ** self._gamma
                # print('phi {} ** {}'.format(t, self._gamma))
                t2 = np.power(t, self._gamma)
                # print(t2)
                ex = self._alpha * t2 + self._beta
                # print(ex)
                res = np.exp(ex)
                # print(res)
                return res
                # res = np.exp(self._alpha * (t ** self._gamma) + self._beta)
            except FloatingPointError as e:
                print('FloatingPointError: {}'.format(e.args[0]))
                print('{} ** {}'.format(t, self._gamma))
                t = np.array([t, ], dtype=np.float128)
                t = t[0]
            finally:
                t2 = np.power(t, self._gamma)
                # print(t2)
                ex = self._alpha * t2 + self._beta
                # print(ex)
                res = np.exp(ex)
                # print(res)
                return res

    def _inv_phi(self, t):
        phi_inv_pivot = 0.6845772418
        if t > phi_inv_pivot:
            return 4.304964539 * (1 - np.sqrt(1 + 0.9567131408 * np.log(t)))
        else:
            if t == 0.0:
                t += np.finfo(t.dtype).resolution
                # print('log div {} -> {}'.format(t, np.log(t)))
            tt = np.log(t)
            # if np.isinf(tt):
            #     print('log div {} -> {}'.format(t, tt))
            #     # tt = np.sign(tt) * 1e15
            #     e = np.finfo(t.dtype).resolution
            #     tt = np.log(t + e)
            #     print('log div {} -> {}'.format(t, tt))
            return (self._a * tt + self._b) ** self._c

    def _calculate_capacities_llr(self):
        m = self._block_power
        initial_val = 2.0 / self._sigma_sq
        # print('ChannelConstructor {}, {}'.format(self._sigma_sq, initial_val))
        z = np.full(self._block_size, initial_val, dtype=np.float128)
        assert z.dtype == np.float128
        # print('GA vector datatype: ', type(z))

        if np.any(np.isnan(z)):
            raise ValueError('Fuck the system!')
            print('Reassign correct value')
            z[:] = initial_val
            z = z.astype(np.float128)
        if np.any(np.isnan(z)):
            print('Reinstantiate vector')
            z = np.ones(self._block_size)
            z *= initial_val
            z = z.astype(np.float128)
        if np.any(np.isnan(z)):
            raise ValueError('Moronic NumPy tries to fuck the system!')
        # print(z)
        for l in range(1, m + 1):
            # print(l)
            o1 = 2 ** (m - l + 1)
            o2 = 2 ** (m - l)
            for t in range(2 ** (l - 1)):
                T = z[t * o1]
                # print(l, t, T, type(T))
                z[t * o1] = self._inv_phi(1. - (1. - self._phi(T)) ** 2)
                if np.isinf(z[t * o1]):
                    print('HUGE_VAL: {}'.format(z[t * o1]))
                    z[t * o1] = T + np.log(2.) / (self._alpha * self._gamma)
                z[t * o1 + o2] = 2. * T
        return z


class ChannelConstructorGaussianApproximationDai(ChannelConstructor):
    """docstring for ChannelConstructorBhattacharyyaBounds"""

    def __init__(self, N, snr):
        ChannelConstructor.__init__(self, N, snr)

        self._alpha = -0.4527
        self._beta = 0.0218
        self._gamma = 0.8600

        self._a = 1.0 / self._alpha
        self._b = -self._beta / self._alpha
        self._c = 1.0 / self._gamma

        self.evaluate()

    def calculate_capacities(self):
        v0 = self._calculate_capacities_llr()
        uv0 = np.unique(v0).size
        print('unique elements llr: {}'.format(uv0))
        self._capacities = sps.erf(np.sqrt(v0.astype(np.float64) / 2.))
        return v0

    def _phi4inv(self, t):
        assert t >= 0.0
        ainv = 0.9125360939445893
        binv = 0.7200545321883631
        cinv = 0.047929057387273905
        if t >= 1.0:
            return 0.0
        elif t > ainv:
            return self._inverse_quadratic_exponential(t, a=0.1047, b=0.4992, r=1.0)
        elif t > binv:
            return self._inverse_quadratic_exponential(t, a=0.05315, b=0.4795, r=0.9981)
        elif t > cinv:
            return ((0.0218 - np.log(t)) / 0.4527) ** (1. / 0.86)
        else:
            return -1. * (np.log(t) + 0.4254) / 0.2832

    def _phi4(self, t):
        assert t >= 0.0
        a = 0.1910
        b = 0.7420
        c = 9.2254
        if t <= a:
            return np.exp(0.1047 * (t ** 2) - 0.4992 * t)
        elif t <= b:
            return 0.9981 * np.exp(0.05315 * (t ** 2) - 0.4795 * t)
        elif t <= c:
            return np.exp(-0.4527 * (t ** 0.86) + 0.0218)
        else:
            return np.exp(-0.2832 * t - 0.4254)

    def _fc(self, t):
        assert t >= 0.0
        tau = 11.673
        if t > tau:
            return t - 2.4476
        else:
            return self._phi4inv(1 - (1 - self._phi4(t)) ** 2)

    def _fv(self, t):
        return 2 * t

    def _inverse_quadratic_exponential(self, y, a=0.05315, b=0.4795, r=0.9981):
        return (b - np.sqrt(4 * a * np.log(y / r) + b ** 2)) / (2. * a)

    def _calculate_capacities_llr(self):
        m = self._block_power
        initial_val = 2. * self._sigma_sq
        z = np.full(self._block_size, initial_val, dtype=np.float128)
        assert z.dtype == np.float128

        for l in range(1, m + 1):
            o1 = 2 ** (m - l + 1)
            o2 = 2 ** (m - l)
            for t in range(2 ** (l - 1)):
                T = z[t * o1]
                z[t * o1] = self._fc(T)
                z[t * o1 + o2] = self._fv(T)
        return z


def plot_capacity_approx(N, snr):
    import matplotlib.pyplot as plt
    bb = ChannelConstructorBhattacharyyaBounds(N, snr)
    ga = ChannelConstructorGaussianApproximation(N, snr)
    plt.figure(figsize=(4, 2))
    for snr in (-1.5917, ):
        bb = ChannelConstructorBhattacharyyaBoundsLog(N, snr)
        ga = ChannelConstructorGaussianApproximation(N, snr)
        ga4 = ChannelConstructorGaussianApproximationDai(N, snr)
        print(f'BB snr={snr}, capacity={np.sum(bb.getCapacities())}')
        print(f'GA snr={snr}, capacity={np.sum(ga4.getCapacities())}')
        plt.plot(bb.getCapacities(), label=f'BB')
        # plt.plot(ga.getCapacities(), label=f'GA, dSNR={snr}')
        plt.plot(ga4.getCapacities(), label=f'GA')

    # for n in range(6, 9):
    #     nn = 2 ** n
    #     for tr in range(1000):
    #         gat = ChannelConstructorGaussianApproximation(nn, snr)

    # bb_caps = bb.getCapacities()
    # ga_caps = ga.getCapacities()

    # plt.plot(bb_caps)
    # plt.plot(ga_caps)
    plt.xlabel('virtual bit channel')
    plt.ylabel('capacity')
    plt.legend(loc='lower right')
    plt.xlim((0, N))
    plt.tight_layout()
    plt.savefig(f'polar_channel_capacity_graph_dSNR-1.5917_N{N}.pgf')
    plt.show()


def main():
    n = 10
    N = int(2 ** n)
    snr = 1.
    plot_capacity_approx(N, snr)
    # plot_dai_functions(N, snr)


if __name__ == '__main__':
    main()
