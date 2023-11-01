#!/bin/python

import matplotlib.pyplot as plt
import matplotlib.ticker as tck
import numpy as np
import argparse
import time
import datetime
import signal_generator
import signal_generator_agilent_33250a
import signal_generator_srs_ds360
from moller import ctrl as moller_ctrl
from scipy import signal
from scipy.fft import rfft, rfftfreq

version = "1.0"

DEFAULT_ADC_DIVISOR = 2
ADC_SAMPLE_RATE = ((1.0/0.000000068) / DEFAULT_ADC_DIVISOR) # Dividing the sample rate by 2 to prevent errors in transmission
ADC_PACKET_SIZE = 0x4000
ADC_RESOLUTION = 4.096 / pow(2, 18)

MAX_FREQUENCY_FOR_AMPLITUDE = 15000000 # No point beyond maximum conversion rate

VISA_DEVICE_DELAY = 1.0

SIGNAL_GENERATOR_VOLTAGE = 2.048
SIGNAL_GENERATOR_OFFSET = 0

TRIUMF_AGILENT_33250A = "GPIB::10::INSTR"
TRIUMF_SRS_DS360 = "GPIB::8::INSTR"

def calculate_samples_for_freq(frequency):
    samples_needed = ((ADC_SAMPLE_RATE * 2) / frequency)
    if samples_needed < ADC_PACKET_SIZE:
        samples_needed = ADC_PACKET_SIZE

    return samples_needed

def get_amplitude_from_samples(samples):
    min_val = None
    max_val = None

    for sample in samples:
        if(min_val == None): min_val = sample[2]
        if(max_val == None): max_val = sample[2]
        if(sample[2] > max_val): max_val = sample[2]
        if(sample[2] < min_val): min_val = sample[2]

    return max_val - min_val

def get_zero_crossings(data):

    n_prev = data[0]
    for n in range(len(data)):
        if(data[n]>0 and n_prev<0):
            first = n
            break
        n_prev = data[n]


    for n in range(len(data)):
        if(data[n]>0 and n_prev<0):
            last = n

        n_prev = data[n]

    return (first, last)

def amplitude_test(res, args, max_freq):
    amplitudes = []
    amplitudes_in_db = []
    frequencies = []
    dbFS = pow(2,18) # Max amplitude is 2x max value 2^17

    amp1 = 0
    amp2 = 0
    slope = 0
    db3 = 0

    # Start at 10 Hz and go up from there
    for freq in [
        # 1, 10, 20, 30, 40, 50, 60, 70, 80, 90,
        100, 200, 300, 400, 500, 600, 700, 800, 900,
        1000, 2000, 3000, 4000, 5000, 6000, 7000, 8000, 9000,
        10000,   20000,   30000, 40000, 50000, 60000, 70000, 80000, 90000,
        100000,  200000,  300000,  400000,  500000,  600000,  700000,  800000,  900000,
        1000000, 1100000, 1200000, 1300000, 1400000, 1450000, 1500000,
        1600000, 1700000, 1800000, 1900000, 2000000, 2250000, 2500000, 2750000, 3000000, 3500000, 4000000, 4500000, 5000000, 6000000, 7000000
        #, 8000000, 9000000, 10000000, 11000000, 12000000, 13000000, 14000000, 15000000
    ]:

        # This test is helpful if the signal generator cannot reach desired frequency
        if(freq > max_freq) or (freq > res.get_max_frequency()):
            break

        samples_needed = calculate_samples_for_freq(freq)
        if samples_needed <= ADC_PACKET_SIZE:
            samples_needed = ADC_PACKET_SIZE
            configure_moller_board(args.ip, args.channel, 1)
        else:
            if(samples_needed > ADC_SAMPLE_RATE):
                samples_needed = ADC_SAMPLE_RATE
            configure_moller_board(args.ip, args.channel, DEFAULT_ADC_DIVISOR)

        print("Frequency: " + str(freq) + " [" + str(samples_needed) + "]")

        # Set frequency
        res.set_frequency(freq)
        time.sleep(VISA_DEVICE_DELAY)
        socket = moller_ctrl.data_init(args.ip)
        samples = moller_ctrl.read_samples(socket, samples_needed)
        socket.close()
        amplitude = get_amplitude_from_samples(samples)          

        if(amplitude == 0):
            print("Invalid Amplitude returned")
            return -1

        amplitudes.append(amplitude)
        amplitudes_in_db.append(20*np.log10(amplitude/dbFS))
        frequencies.append(freq)

        if(freq == 1450000):
            db3 = 20*np.log10(amplitude/dbFS)
            print(db3)

        if(freq == 3000000):
            amp1 = 20*np.log10(amplitude/dbFS)

        if(freq == 5000000):
            amp2 = 20*np.log10(amplitude/dbFS)
            slope = (np.abs(amp2 - amp1) / np.log10(5000000/3000000))

    d = datetime.datetime.now()
    with open(d.strftime('%Y%m%d') + "gain_vs_amplitude_ch" + str(args.channel) + "_" + res.get_description() + ".txt", "w+") as f:
        f.write("frequency,amplitude,amplitude_db\n")
        for n in range(len(amplitudes)):
            f.write(str(frequencies[n]) + "," + str(amplitudes[n]) + "," + str(20*np.log10(amplitudes[n]/dbFS)) + "\n")

    plt.semilogx(frequencies, amplitudes_in_db)
    plt.title("Amplitude over Frequency range with " + str(SIGNAL_GENERATOR_VOLTAGE) + " Vpp signal")
    plt.suptitle(res.get_description() + "\nChannel " + str(args.channel) + "\nRolloff: " + str("{:.2f}".format(slope)) + " dB/decade  dB at expected -3dB: " + "{:.2f}".format(db3) + " dB")
    plt.xlabel("Frequency")
    plt.ylabel("db")
    plt.ylim([-110,10])
    plt.xlim([1, ADC_SAMPLE_RATE / 2])
    plt.grid(True)
    plt.show()

def fft_test(res, args, frequency):
    if(frequency != 0):
        res.set_frequency(frequency)
        time.sleep(VISA_DEVICE_DELAY)

    segment_len = ADC_PACKET_SIZE*64
    samples = []
    configure_moller_board(args.ip, args.channel, DEFAULT_ADC_DIVISOR)

    socket = moller_ctrl.data_init(args.ip)
    samples = moller_ctrl.read_samples(socket, segment_len)
    socket.close()

    times = []
    datas = []
    for sample in samples:
        times.append(sample[0])
        datas.append(sample[2])

    times = np.array(times)
    datas = np.array(datas)

    # f,l = get_zero_crossings(datas)
    # times = times[f:l]
    # datas = datas[f:l]

    # window_weight = signal.windows.kaiser(len(datas), 1)
    # window_weight = signal.windows.blackmanharris(len(datas))
    # data_weighted = datas # * window_weight
    # fft_bins = rfftfreq(len(data_weighted), 1.0 / ADC_SAMPLE_RATE)
    # fft_data = rfft(data_weighted)
    # fft = np.abs(fft_data) / len(fft_data)
    # p = 20*np.log10(fft/(pow(2,17)))

    plt.plot(times, datas)

    # datas = datas / pow(2,17)
    # fft_bins, Pxx = signal.welch(datas, ADC_SAMPLE_RATE, scaling='spectrum', window=signal.windows.blackmanharris(len(datas)))
    # p = 10 * np.log10(Pxx)
    # plt.ylim([-160,5])
    # plt.xlim([1,ADC_SAMPLE_RATE / 2])
    # plt.plot(fft_bins, p)
    
    plt.grid(True)
    # plt.xlabel("Frequency in Hz")
    # plt.ylabel("Power in dB")

    plt.title("FFT @ " + str(frequency) + " with " + str(SIGNAL_GENERATOR_VOLTAGE) + "Vpp signal")
    if(frequency != 0):
        plt.suptitle(res.get_description())
    else:
        plt.suptitle("Manually Set Frequency / (Unknown Source)")
    plt.show()

def fft_test_plot_all(res, args):

    frequencies_to_test = [
        100, 1000, 10000, 100000, 1000000, 2000000
    ]

    fig, axs = plt.subplots(len(frequencies_to_test), sharey=True, sharex=True)

    hann_weight = []
    fft_data = []

    samples_needed = ADC_SAMPLE_RATE # calculate_samples_for_freq(freq)
    configure_moller_board(args.ip, args.channel, DEFAULT_ADC_DIVISOR)

    for n, freq in enumerate(frequencies_to_test):

        # This test is helpful if the signal generator cannot reach desired frequency
        if(freq > res.get_max_frequency()):
            break

        print("Current frequency: " + str(freq))
        res.set_frequency(freq)
        time.sleep(VISA_DEVICE_DELAY)

        socket = moller_ctrl.data_init(args.ip)
        samples = moller_ctrl.read_samples(socket, samples_needed)
        socket.close()

        sample_data = []
        for sample in samples:
            sample_data.append(sample[2])

        hann_weight = np.hanning(len(sample_data))

        data_weighted = sample_data * hann_weight

        bins = np.fft.rfftfreq(len(data_weighted), 1.0 / ADC_SAMPLE_RATE)
        data = np.fft.rfft(data_weighted)

        fft_data.append(data)

        axs[n].semilogx(bins, np.abs(fft_data[n]))
        axs[n].set_title("FFT @ " + str(freq) + " Hz")
        axs[n].grid(True)

    # plt.xlabel("Frequency")
    # plt.ylabel("Amplitude")
    # plt.suptitle(res.get_description())
    plt.tight_layout()
    plt.show()

def main():
    prog='filter_test'
    parser = argparse.ArgumentParser(prog=prog)
    parser.add_argument('--version', action='version', version='%(prog)s ' + version)
    parser.add_argument("ip", help="")
    parser.add_argument("device", choices=["agilent_33250a", "srs_ds360"],help="")

    cmd_parser = parser.add_subparsers(dest="command")

    amplitude_parser = cmd_parser.add_parser("amplitude")
    amplitude_parser.add_argument("channel")
    amplitude_parser.set_defaults(func=arg_amplitude)

    fft_parser = cmd_parser.add_parser("fft")
    fft_parser.set_defaults(func=arg_fft)
    fft_parser.add_argument("channel")
    fft_parser.add_argument("frequency")

    args = parser.parse_args()

    if(args.device == "agilent_33250a"):
        args.device_addr = TRIUMF_AGILENT_33250A
    elif(args.device == "srs_ds360"):
        args.device_addr = TRIUMF_SRS_DS360
    else:
        args.device_addr = "GPIB::0::INSTR"

    if(hasattr(args,'func')):
        args.func(args)
    else:
        parser.print_help()


def arg_amplitude(args):

    res = signal_generator.SignalGenerator(args.device)
    res.setup(args.device_addr, SIGNAL_GENERATOR_VOLTAGE, SIGNAL_GENERATOR_OFFSET)

    amplitude_test(res, args, MAX_FREQUENCY_FOR_AMPLITUDE)


def arg_fft(args):
    if(args.frequency == "0"):
        fft_test(None, args, 0)
    else:
        res = signal_generator.SignalGenerator(args.device)
        res.setup(args.device_addr, SIGNAL_GENERATOR_VOLTAGE, SIGNAL_GENERATOR_OFFSET)
        if(args.frequency == "sweep"):
            fft_test_plot_all(res, args)
        else:
            fft_test(res, args, float(args.frequency))
    pass

def configure_moller_board(ip, channel, divisor):
    global ADC_SAMPLE_RATE

    # Open up sockets to moller board
    ctrl_socket = moller_ctrl.ctrl_init(ip)

    ch = (int(channel) & 0xF)
    ch = (ch + (ch << 4)) << 16
    moller_ctrl.write_msg(ctrl_socket, 0x44, 0x80000000 | ((divisor-1) << 24) | ch | ADC_PACKET_SIZE)

    ADC_SAMPLE_RATE = (14705882 / divisor)

if __name__ == "__main__":
    main()