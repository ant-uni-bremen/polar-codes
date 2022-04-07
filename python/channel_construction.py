#!/usr/bin/env python3
# -*- coding: utf-8 -*-
#
# Copyright 2020 Johannes Demel.
#
# SPDX-License-Identifier: GPL-3.0-or-later
#

import numpy as np
from scipy import special as sps

from latex_plot_magic import set_size


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
        return np.exp(-s)

    def sortedChannels(self, capacities):
        return np.argsort(capacities)

    def frozenBitPositions(self, n_frozen):
        return self._sortedChannels[0:n_frozen]

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
        # the last part with log 1-exp(vals - l2) needs a log domain approximation!
        return vals + l2 + np.log(1 - np.exp(vals - l2))

    def _calculate_channels_ln(self, vals):
        res = np.empty(2 * len(vals), dtype=vals.dtype)
        res[0::2] = self._degrade_ln(vals)
        res[1::2] = self._upgrade_ln(vals)
        return res

    def _calculate_capacities_ln(self):
        vals = np.array([np.log(1. - self._eta), ], dtype=np.float128)
        for i in range(self._block_power):
            vals = self._calculate_channels_ln(vals)
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
        for i in range(self._block_power):
            vals = self._calculate_channels_lin(vals)
        return vals


class ChannelConstructorGaussianApproximationDai(ChannelConstructor):
    """ChannelConstructorGaussianApproximationDai

    Perform channel polarization via Gaussian Approximation.
    This algorithm is form:
    Dai et al. 'Does Gaussian Approximation Work Well for the Long-Length Polar Code Construction?'
    """

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


class ChannelConstructorBetaExpansion(ChannelConstructor):
    """ChannelConstructorBetaExpansion
    cf. He et al. "beta-expansion: A theoretical framework for fast and recursive construction of polar codes"
    DOI: 10.1109/TSP.2014.2371781
    https://doi.org/10.1109/GLOCOM.2017.8254146
    """
    beta_value = 2.0 ** (1.0 / 4.0)

    def __init__(self, N, snr):
        ChannelConstructor.__init__(self, N, snr)
        self.evaluate()

    def calculate_capacities(self):
        channels = np.arange(self._block_size, dtype=np.uint64)
        betas = np.zeros(self._block_size, dtype=np.float64)
        beta_weights = self.beta_value ** np.arange(
            self._block_power - 1, -1, -1)
        for i, idx in enumerate(channels):
            bin_digits = [int(b)
                          for b in np.binary_repr(idx, self._block_power)]
            # print(bin_digits)
            weight = np.sum(beta_weights * bin_digits)
            # print(beta_weights * bin_digits, weight)
            betas[i] = weight
        # This is incorrect strictly speaking. But let's keep this value.
        self._capacities = betas
        return betas


def plot_capacity_approx(N, snr):
    import matplotlib.pyplot as plt
    bb = ChannelConstructorBhattacharyyaBounds(N, snr)
    static_size = (4, 2)
    latex_size = set_size()
    fig_size = (latex_size[0], static_size[1] * (latex_size[0] / static_size[0]))
    plt.figure(figsize=fig_size)
    min_dsnr = -1.5917
    for snr in (0, ):
        bb = ChannelConstructorBhattacharyyaBoundsLog(N, snr)
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
    plt.savefig(f'polar_channel_capacity_graph_dSNR0.0_N{N}.pgf', bbox_inches='tight', pad_inches=0)
    plt.show()


def main():
    n = 6
    N = int(2 ** n)
    snr = 1.
    cc = ChannelConstructorBetaExpansion(N, 0.0)
    print(cc.beta_value)
    print(cc.getSortedChannels())
    plot_capacity_approx(N, snr)
    # plot_dai_functions(N, snr)


if __name__ == '__main__':
    main()
