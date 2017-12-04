#!/usr/bin/env python
from __future__ import print_function, division
import numpy as np
import scipy.signal as scisig


def ebn0_to_sigma(ebn0, coderate, constellation_order=1.):
    # Assume E_b = 1.0! Assume complex Gaussian noise N0 = xxx!
    rate = constellation_order * coderate
    # Assume E_b = 1.0
    ebn0_lin = 10. ** (ebn0 / 10.)
    snr_lin = ebn0_lin * rate
    return np.sqrt(1. / snr_lin)


def get_complex_noise_vector(vec_len, sigma):
    '''
    Generate a random vector with specified size and noise power. Noise power is given as amplitude.
    :param vec_len: number of complex noise samples
    :param sigma: standard deviation of noise. 
    :return: complex random vector
    '''
    dev = np.sqrt(.5) * sigma  # we expect a complex value, needs to be split for I and Q
    noise = np.random.normal(0.0, dev, vec_len) + 1j * np.random.normal(0.0, dev, vec_len)
    return noise


def evaluate_noise_level():
    for constellation_order in np.arange(1., 3.):
        for inv_coderate in np.arange(2., 4,):
            print('constellation order {} invers coderate {}'.format(constellation_order, inv_coderate))
            for ebn0 in np.arange(0., 13.):
                sigma = 10. ** (-ebn0 / 20.)
                f_sigma = ebn0_to_sigma(ebn0, 1. / inv_coderate, constellation_order)
                rate = 1. * constellation_order / inv_coderate

                fsq = f_sigma ** 2
                osq = sigma ** 2
                s_ratio = fsq / osq
                ebn0_db = 10. * np.log10(1. / osq)
                snr_db = 10. * np.log10(1. / fsq)

                print('{:.4f}, {:.4f} ratio {:.4f}({:.4f})\t\tEbN0 {:.2f}\tvs\tSNR {:.2f}'.format(osq, fsq, s_ratio, 1. / rate, ebn0, snr_db))
                assert np.abs(rate - 1. / s_ratio) < 1e-6

                noise = get_complex_noise_vector(200000, sigma)

                est_sigma = np.sqrt(np.sum(noise.real ** 2 + noise.imag ** 2) / (len(noise)-1))
                # print('compare set {:.4f} and est {:.4f}'.format(sigma, est_sigma))
                assert np.abs(sigma - est_sigma) < 1e-2  # make sure estimated and set sigma are approximately the same!
                assert np.abs(ebn0 - ebn0_db) < 1e-6


def calculate_signal_energy(s):
    return np.sum(s.real ** 2 + s.imag ** 2)


def normalize_power_delay_profile(pdp):
    e = calculate_signal_energy(pdp)
    return pdp / np.sqrt(e)


def generate_2tap_pdp(channel_delay, samp_rate, oversampling_factor, attenuation_db):
    sample_dur = 1. / samp_rate
    samp_channel_delay = int(np.ceil(channel_delay / sample_dur))

    if attenuation_db > 0.0:
        attenuation_db *= -1.
    attenuation_lin = 10 ** (attenuation_db / 20.)

    pdp = np.zeros(int(2 * samp_channel_delay * oversampling_factor))
    pdp[0] = 1.
    pdp[samp_channel_delay * oversampling_factor + np.random.randint(-oversampling_factor//2, oversampling_factor//2)] = 1. * attenuation_lin
    pdp = scisig.resample(pdp, 2 * samp_channel_delay)
    pdp = np.abs(pdp)
    return normalize_power_delay_profile(pdp)


def evaluate_rayleigh_channel():
    import matplotlib.pyplot as plt
    channel_delay = 100.e-9
    samp_rate = 25.6e6
    oversampling_factor = 9
    attenuation_db = 10.

    for i in range(10):
        pdp = generate_2tap_pdp(channel_delay, samp_rate, oversampling_factor, attenuation_db)
        pdp *= np.random.randn(len(pdp))
        plt.plot(np.abs(np.fft.fft(pdp, 128)))
    plt.show()

    print(pdp)


def verify_tap_values():
    epsilon = 5e-3
    target = 1.0
    success = False
    avg_e = 0.0
    for i in range(10):
        taps = random_complex_symbols(100000)
        et = calculate_signal_energy(taps)
        avg_e = np.sum(et) / len(taps)
        print(avg_e)
        if np.abs(avg_e - target) < epsilon:
            success = True
            break

    if success:
        print('Taps have expected average energy ({})'.format(target))
    else:
        raise ValueError('Average tap energy ({}) not within bound ({}) of target ({})!'.format(avg_e, epsilon, target))





def get_2tap_pdp(channel_delay, attenuation_db, samp_rate):
    if attenuation_db > 0.0:
        attenuation_db *= -1.
    T_samp = 1. / samp_rate
    ir_len = int(np.ceil(channel_delay / T_samp))
    ir_len = max(1, ir_len)
    tap2_sigma = 10 ** (attenuation_db / 20.)
    pdp = np.zeros(ir_len)
    pdp[0] = 1.0
    pdp[-1] = tap2_sigma
    return normalize_power_delay_profile(pdp)


def validate_2tap_pdp():
    channel_delay = 100.e-9
    samp_rate = 25.6e6
    for i in range(11):
        pdp = get_2tap_pdp(channel_delay, i, samp_rate)
        e = calculate_signal_energy(pdp)
        print(e, pdp)


def hiflecs_channel_taps(channel_delay=100.e-9, attenuation_db=-10., samp_rate=25.6e6, print_channel_summary=True):
    '''
    the default params were discussed in a Telco on 10.1.2017

    This function returns the channel taps for a 2-tap Rayleigh channel.
    The second path is attenuated by "attenuation_db" in relation to the first path
    The parameters channel_delay and samp_rate are used to calculate the sample distance between the first and second path.
    '''
    # First, get a power delay profile.
    pdp = get_2tap_pdp(channel_delay, attenuation_db, samp_rate)

    # get random complex symbols with E(|h|^2) == 1.
    h = random_complex_symbols(len(pdp))
    h *= pdp

    if print_channel_summary:
        print('HiFlecs channel PARAMS: tau={}s, Att={}dB, sample rate {}sps'.format(channel_delay, attenuation_db, samp_rate))
        print('sample duration {}s, 2nd path sigma={}, #taps={}'.format(1. / samp_rate, pdp[-1], len(pdp)))
        print('channel energy: ', calculate_signal_energy(h))
        print('channel taps:   ', h)
        print('tap amplitudes: ', np.abs(h))
        print('2nd path atten: ', 20 * np.log10(np.abs(h[-1]) / np.abs(h[0])))
    return h


def main():
    np.set_printoptions(precision=2, linewidth=150)
    evaluate_noise_level()
    evaluate_rayleigh_channel()


if __name__ == '__main__':
    main()
