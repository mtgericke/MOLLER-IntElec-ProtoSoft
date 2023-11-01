#!/bin/python

import matplotlib.pyplot as plt
import matplotlib.animation as animation
from matplotlib.collections import LineCollection
from matplotlib.widgets import TextBox
from matplotlib.offsetbox import AnchoredText
from matplotlib.widgets import SpanSelector
from matplotlib.ticker import EngFormatter

import numpy as np
import argparse
import math
import signal
import time
import signal_generator
import signal_generator_agilent_33250a
import signal_generator_srs_ds360

from moller import ctrl as moller_ctrl
import scipy as sp

version = "1.0"

NUM_ADC_BITS = 18

ADC_DIVISOR = 2
ADC_SAMPLE_RATE = ((1.0/0.000000068) / ADC_DIVISOR) # Dividing the sample rate by 2 to prevent errors in transmission
ADC_PACKET_SIZE = (0x2004)
NUM_SAMPLES_PER_PACKET = ADC_PACKET_SIZE - 4
ADC_MAX_VOLTAGEpp = 4.096
ADC_RESOLUTION = ADC_MAX_VOLTAGEpp / pow(2, NUM_ADC_BITS)

VISA_DEVICE_DELAY = 1.0

SIGNAL_GENERATOR_VOLTAGE = 2.048
SIGNAL_GENERATOR_OFFSET = 0

TRIUMF_AGILENT_33250A = "GPIB::10::INSTR"
TRIUMF_SRS_DS360 = "GPIB::8::INSTR"

# TODO: move these to args
NUM_SAMPLES = NUM_SAMPLES_PER_PACKET * 32
AVERAGING = 1
WINDOW_CORR_FACTOR = 2 #  https://community.sw.siemens.com/s/article/window-correction-factors

# Hann 2.0
# Blackman 2.80
# Blackman-harris 2.2
# Flattop 4.18

HARMONICS_TO_CAPTURE = 25
HARMONICS_TO_ANNOTATE = 7

"""
CTRL+C Interrupt Handler
"""
class GracefulExiter:
    def __init__(self):
        self.state = False
        signal.signal(signal.SIGINT, self.change_state)

    def change_state(self, signum, frame):
        signal.signal(signal.SIGINT, signal.SIG_DFL)
        self.state = True

    def exit(self):
        return self.state


flag = GracefulExiter()

def fft_test(res, args, frequency):

    # def submit_freq(text):
    #     nonlocal frequency
    #     nonlocal res
#
    #     frequency = int(text)
    if(res != None):
        res.set_frequency(frequency)

    graph_data = [[], []]

    configure_moller_board(args.sim, args.ip, args.channel, ADC_DIVISOR)

    fig = plt.figure(figsize=(160, 90))
    gs = fig.add_gridspec(6, 2)
    dat_chart = fig.add_subplot(gs[0, 0])

    coh_chart = fig.add_subplot(gs[0, 1], sharey=dat_chart)
    coh_chart_lc = LineCollection([], linewidth=0.1, colors='blue')
    coh_chart_stepped_lc = LineCollection([], linewidth=0.1, colors='red')
    coh_chart.add_collection(coh_chart_lc)
    coh_chart.add_collection( coh_chart_stepped_lc )

    fft_chart_zoom = fig.add_subplot(gs[1, :])
    fft_zoom_ln, = fft_chart_zoom.plot([], [], color='blue', linewidth=0.2)

    fft_chart = fig.add_subplot(gs[2:, :])
    fft_chart_lc = LineCollection([])
    fft_chart.add_collection( fft_chart_lc )

    dat_ln, = dat_chart.plot([], [])
    fft_ln, = fft_chart.plot([], [], color='blue', linewidth=0.4)
    fft_peaks_ln, = fft_chart.plot([], [], color='orange', linewidth=0.4, marker='x')
    fft_harmonic_annotation = []
    for n in range(0, HARMONICS_TO_ANNOTATE):
        fft_harmonic_annotation.append( fft_chart.annotate('',
            xy=(0, 0), xycoords='data',
            xytext=(1.5, 1.5), textcoords='offset points'))


    txt_info = AnchoredText("", loc='upper right')
    fft_chart.add_artist(txt_info)

    # txtbox_freq = TextBox(dat_chart, 'Expected Frequency', initial=str(frequency))
    # txtbox_freq.on_submit(submit_freq)

    fft_chart_zoom_xmin = 0
    fft_chart_zoom_xmax = 1000000

    def onselect(xmin, xmax):
        nonlocal fft_chart_zoom_xmin
        nonlocal fft_chart_zoom_xmax
        fft_chart_zoom_xmin = xmin
        fft_chart_zoom_xmax = xmax
        if(xmin == xmax):
            fft_chart_zoom_xmin = 0
            fft_chart_zoom_xmax = ADC_SAMPLE_RATE/2

    step = 5000
    num = ADC_SAMPLE_RATE
    start = 0
    span = SpanSelector(fft_chart_zoom, onselect, 'horizontal', useblit=True, props=dict(alpha=0.5, facecolor="tab:blue"), interactive=True, drag_from_anywhere=True, snap_values=np.arange(0,num)*step+start)
    span.extents = (fft_chart_zoom_xmin, fft_chart_zoom_xmax)

    def get_data(args, graph_data):
        nonlocal frequency

        # while not flag.exit():

        if(args.sim):
            Fs = ADC_SAMPLE_RATE
            sample_time = np.arange(0, NUM_SAMPLES * 68 * ADC_DIVISOR, 68 * ADC_DIVISOR)

            sample_time = sample_time.astype(np.int32)
            sample_data = np.sin(2 * np.pi * frequency / (68 * ADC_DIVISOR) * sample_time / Fs) * pow(2,17)

            # Generate harmonics
            # for n in range(20):
            #     if(n % 2 == 0):
            #         sample_data = sample_data + np.sin(2 * np.pi * (frequency * (n * 0.5)) / (68 * ADC_DIVISOR) * sample_time / Fs) * pow(2,12)
            #     sample_data = sample_data + np.sin(2 * np.pi * (frequency * n) / (68 * ADC_DIVISOR) * sample_time / Fs) * pow(2,7-n)

            # Add some noise
            sample_data = sample_data + np.random.normal(0, np.sqrt(0.1), len(sample_data))

            # Convert to integer values to match what the ADC produces
            sample_data = sample_data.astype(np.int32)

            graph_data[0] = sample_time
            graph_data[1] = sample_data
        else:
            socket = moller_ctrl.data_init(args.ip)
            samples = moller_ctrl.read_samples(socket, NUM_SAMPLES, ADC_PACKET_SIZE, True)
            time_data = []
            sample_data = []
            for sample in samples:
                time_data.append(sample[0])
                sample_data.append(sample[2])

            graph_data[0] = np.array(time_data)
            graph_data[1] = np.array(sample_data)

            socket.close()

            # time.sleep(0.0001)

    def init():
        hertz_formatter = EngFormatter(unit='Hz')
        volt_formatter = EngFormatter(unit='V')
        time_formatter = EngFormatter(unit='s')

        dat_chart.set_title("Subset of data samples in voltage domain")
        dat_chart.set_xlim(0,(NUM_SAMPLES*68*ADC_DIVISOR))
        dat_chart.set_ylim(-(ADC_MAX_VOLTAGEpp/2), ADC_MAX_VOLTAGEpp/2)
        dat_chart.yaxis.set_major_locator(plt.MultipleLocator(ADC_MAX_VOLTAGEpp/4))
        dat_chart.yaxis.set_major_formatter(volt_formatter)
        dat_chart.xaxis.set_major_formatter(time_formatter)
        dat_chart.grid(True)

        coh_chart.set_title("Overlayed segments of data samples of expected periodic length")
        coh_chart.set_xlim(0,(NUM_SAMPLES*68*ADC_DIVISOR))
        coh_chart.set_ylim(-(ADC_MAX_VOLTAGEpp/2), ADC_MAX_VOLTAGEpp/2)
        coh_chart.yaxis.set_major_locator(plt.MultipleLocator(ADC_MAX_VOLTAGEpp/4))
        coh_chart.yaxis.set_major_formatter(volt_formatter)
        coh_chart.xaxis.set_major_formatter(time_formatter)
        coh_chart.grid(True)


        fft_chart.set_title("FFT")
        fft_chart.grid(True)
        fft_chart.set_xlabel("Frequency (Hz)")
        fft_chart.set_ylabel("Power (dBFS)")
        fft_chart.xaxis.set_major_formatter(hertz_formatter)

        fft_chart_zoom.set_title("Zoom Window")
        fft_chart_zoom.xaxis.set_major_formatter(hertz_formatter)
        fft_chart_zoom.grid(True)

        gs.tight_layout(fig, pad=30)

        return dat_ln,

    def update(i, graph_data):
        nonlocal frequency
        nonlocal fft_chart_zoom_xmin
        nonlocal fft_chart_zoom_xmax

        if(span.extents[0] == span.extents[1]):
            fft_chart_zoom_xmin = 0
            fft_chart_zoom_xmax = ADC_SAMPLE_RATE/2

        get_data(args, graph_data)

        time_with_zc = graph_data[0] / 1000000000  # Put into seconds
        data_with_zc = graph_data[1]

        samples_per_period = int(ADC_SAMPLE_RATE / frequency)
        num_periods = math.floor(len(data_with_zc) / samples_per_period)
        total_segments_len = num_periods * samples_per_period

        # Data for 'raw' data plot
        # Limit display to six periods
        if(num_periods > 5):
            num_periods_to_display = 6
        else:
            num_periods_to_display = num_periods

        dat_plot_sample_count = int(ADC_SAMPLE_RATE / frequency * num_periods_to_display)

        dat_chart.set_xlim([0, time_with_zc[dat_plot_sample_count-1]])
        dat_ln.set_data(time_with_zc[0:dat_plot_sample_count], data_with_zc[0:dat_plot_sample_count] * ADC_RESOLUTION)

        # Prep for coherent plot, break data into segments
        coh_chart.set_xlim(0,(samples_per_period*68*ADC_DIVISOR))

        time_segment = time_with_zc[0:samples_per_period]
        data_full = data_with_zc[0:total_segments_len]

        data_segments = np.split(data_with_zc[0:total_segments_len], num_periods)

        step_of_segments = math.floor(len(data_segments) / 10)
        if(step_of_segments == 0):
            step_of_segments = 1

        range_of_segments = len(data_segments)
        if(range_of_segments > 10):
            range_of_segments = 10

        coherent_segments = []
        for n in range(0, range_of_segments):
            coherent_segments.append( np.stack((time_segment , data_segments[n] * ADC_RESOLUTION), axis=1) )


        coherent_segments_stepped = []
        for n in range(0, len(data_segments), step_of_segments):
            coherent_segments_stepped.append( np.stack((time_segment, data_segments[n] * ADC_RESOLUTION), axis=1) )

        coh_chart.set_xlim([0, time_segment[samples_per_period-1]])
        coh_chart_lc.set_segments( coherent_segments )
        coh_chart_stepped_lc.set_segments( coherent_segments_stepped )

        # NOTE: Careful! Applying a windowing function causes issues calculating the power (dB) of the signal
        # data_weighted = data_full # * sp.signal.windows.blackmanharris(len(data_full))
        # fft_len = len(data_weighted) # sp.fft.next_fast_len(len(data_weighted))
        # fft_bins = sp.fft.rfftfreq(fft_len, 1.0 / ADC_SAMPLE_RATE)
        # fft_data = np.abs( sp.fft.rfft(data_weighted, fft_len, overwrite_x=True) )
        # fft_data = fft_data / len(fft_data)
        # p_dB = 20*np.log10(fft_data / pow(2,17)) # Converting to dB from voltage

        data_full = data_full / pow(2,17)
        fft_bins, Pxx = sp.signal.welch(data_full, ADC_SAMPLE_RATE, scaling='spectrum', nperseg=math.ceil(len(data_full)/AVERAGING), window=sp.signal.windows.get_window('hann', math.ceil(len(data_full)/AVERAGING), True))
        p_dB = 10 * np.log10(Pxx * WINDOW_CORR_FACTOR)

        fft_chart.set_xlim([fft_chart_zoom_xmin, fft_chart_zoom_xmax])
        fft_chart.set_ylim([-180,5])
        fft_ln.set_data( fft_bins, p_dB)

        fft_chart_zoom.set_xlim([0, ADC_SAMPLE_RATE/2])
        fft_chart_zoom.set_ylim([-180,5])
        fft_zoom_ln.set_data( fft_bins, p_dB)

        bin_size = fft_bins[1] # in Hz
        SNR_ideal = -(6.02 * (NUM_ADC_BITS) + 1.76)
        FFT_floor_ideal = SNR_ideal + -(10*np.log10(len(fft_bins)))

        # Get all peaks
        all_peaks, _ = sp.signal.find_peaks(p_dB, height=FFT_floor_ideal, prominence=5)

        if(len(all_peaks) > 1):
            # Sort them
            ind = np.argsort(p_dB[all_peaks])

            p_db_ordered = (p_dB[all_peaks])[ind]
            f_ordered = (fft_bins[all_peaks])[ind]

            # Take the two highest, that is the SFDR
            if p_db_ordered[-1] > SNR_ideal:
                SFDR = p_db_ordered[-1] - p_db_ordered[-2]
            else:
                SFDR = 0

            all_peaks[np.argmax(p_dB[all_peaks])]
        else:
            SFDR = 0

        # Get peaks
        peaks, peak_properties = sp.signal.find_peaks(p_dB, height=FFT_floor_ideal+25, prominence=5, width=1, distance=(frequency/bin_size) * 0.98)

        # Removes peaks lower in frequency than EXPECTED fundamental frequency
        peaks = (peaks[peaks > ((frequency / bin_size) - bin_size)])

        # fft_peaks_ln.set_data( fft_bins[peaks], p_dB[peaks])

        # Attempt to fit found peaks to harmonics
        harmonics = []
        for n in range(1, HARMONICS_TO_CAPTURE):
            found = False
            db = 0
            freq = 0
            fft_i = 0
            peak_i = 0
            for peak_index, peak in enumerate(peaks):
                if(math.isclose(fft_bins[peak], (frequency * n), rel_tol=0.01)):
                    freq = fft_bins[peak]
                    db = p_dB[peak]
                    p = Pxx[peak]
                    fft_i = peak
                    peak_i = peak_index
                    found = True

                if(found == True):
                    break

            if(found):
                harmonics.append((n, freq, db, p, fft_i, peak_i))
            else:
                harmonics.append((n, 0, 0, 0.000000000000000001, None, None))


        THD = 0
        if len(harmonics) > 1 and harmonics[0][4] != None:
            Pxx_copy = np.copy(Pxx)

            fft_idx = harmonics[0][4]
            peak_idx = harmonics[0][5]
            samples_around_peak = math.ceil(peak_properties['widths'][peak_idx] / 2) + 2
            min_range = fft_idx - samples_around_peak
            if(min_range < 0):
                min_range = 0

            max_range = fft_idx + samples_around_peak
            if max_range > len(Pxx_copy) - 1:
                max_range = Pxx_copy - 1

            Ps = np.sum(Pxx_copy[min_range:max_range])

            Pn = 0
            for n in range(1, len(harmonics)-1):
                if(harmonics[n][4] != None):
                    fft_idx = harmonics[n][4]
                    peak_idx = harmonics[n][5]
                    samples_around_peak = math.ceil(peak_properties['widths'][peak_idx] / 2) + 2
                    min_range = fft_idx - samples_around_peak
                    if(min_range < 0):
                        min_range = 0

                    max_range = fft_idx + samples_around_peak
                    if max_range > len(Pxx_copy) - 1:
                        max_range = Pxx_copy - 1

                    Pn = Pn + np.sum(Pxx_copy[min_range:max_range])

            if Pn == 0:
                THD = 0
            else:
                THD = 10 * np.log10(Ps/Pn)

        SINAD = 0
        if len(harmonics) > 0:
            if(harmonics[0][4] != None):

                Pxx_copy = np.copy(Pxx)
                fft_idx = harmonics[0][4]
                peak_idx = harmonics[0][5]

                samples_around_peak = math.ceil(peak_properties['widths'][peak_idx] / 2) + 2 # Fudge number to get spectral leakage
                min_range = fft_idx - samples_around_peak
                if(min_range < 0):
                    min_range = 0

                max_range = fft_idx + samples_around_peak
                if max_range > len(Pxx_copy) - 1:
                    max_range = Pxx_copy - 1

                Ps = np.sum(Pxx_copy[min_range:max_range])
                Pxx_copy[min_range:max_range] = 0

                Pn_and_Pd = np.sum(Pxx_copy)

                SINAD = 10 * np.log10( (Ps + Pn_and_Pd) / Pn_and_Pd )

        ENOB = (SINAD - 1.76) / 6.02

        harmonic_name = []
        harmonic_freq = []
        harmonic_db = []
        strHarmonic = ""
        for n, harmonic in enumerate(harmonics):
            if n < len(fft_harmonic_annotation):
                if(harmonic[1] != 0):
                    fft_harmonic_annotation[n].set_text( "F" + str(harmonics[n][0]) )
                    harmonic_name.append("F" + str(n+1))
                    harmonic_freq.append(harmonic[1])
                    harmonic_db.append(harmonic[2])
                    strHarmonic = strHarmonic + "F" + str(n+1) + " {:0.2f}".format(harmonic[2]) + " dB\n"
                else:
                    fft_harmonic_annotation[n].set_text( "" )
                    strHarmonic = strHarmonic + "F" + str(n+1) + " Missing\n"
                fft_harmonic_annotation[n].xy = harmonics[n][1], harmonics[n][2]

        fft_annotations = [
            [(1, SNR_ideal), (ADC_SAMPLE_RATE/2, SNR_ideal)],
            [(1, FFT_floor_ideal), (ADC_SAMPLE_RATE/2, FFT_floor_ideal)],
        ]

        fft_annotations_colors = [
            (1, 0, 0, 1), # SNR_ideal
            (0, 1, 0, 1), # FFT_floor_ideal
        ]

        if(SFDR != 0):
            fft_annotations.append([(f_ordered[-1], p_db_ordered[-1]), (f_ordered[-2], p_db_ordered[-2]) ])
            fft_annotations_colors.append((1, 0, 0, 0.5))

        fft_chart_lc.set_linestyle( 'dashed' )
        fft_chart_lc.set_segments( fft_annotations )
        fft_chart_lc.set_colors( fft_annotations_colors )

        if(not args.sim):
            strChannel = "Channel " + str(args.channel) + "\n"
        else:
            strChannel = "Simulation\n"

        txt_info.txt.set_text(
            strChannel +
            "Sample Rate: " + "{:0.2f}".format(ADC_SAMPLE_RATE / 1000000) + " Msps\n"
            "Samples: " + str(len(time_with_zc)) + " (" + str(len(data_full)) + ")\n"
            "Bins: " + str(len(fft_bins)) + "\n"
            "Bin size: " + "{:0.2f}".format(bin_size) + " Hz/bin\n"
            "SINAD: " + "{:.02f}".format(SINAD) + " dB\n"
            "SFDR: " + "{:.02f}".format(SFDR) + " dBc\n"
            "THD: " + "{:.02f}".format(THD) + " dB\n"
            "ENOB: " + "{:.01f}".format(ENOB) + " bits\n"
            "\nHarmonics \n" +
            strHarmonic[:-1] # Drops the trailing \n
        )

        return dat_ln, fft_ln, fft_zoom_ln, coh_chart_lc, coh_chart_stepped_lc


    plt.grid(True)
    plt.ioff()

    interval = 50

    ani = animation.FuncAnimation(
        fig, update, init_func=init, fargs=(graph_data,), interval=interval, blit=True, cache_frame_data=False,
    )

    # Blocks until closed
    plt.show()

    # Tell thread to end
    flag.change_state(0, 0)


def main():
    prog='filter_test'
    parser = argparse.ArgumentParser(prog=prog)
    parser.add_argument('--version', action='version', version='%(prog)s ' + version)
    parser.add_argument("ip", help="")
    parser.add_argument("-d", "--device", default=None, choices=["agilent_33250a", "srs_ds360"], help="")
    parser.add_argument("-s", "--sim", action='store_true')
    parser.add_argument("channel")
    parser.add_argument("frequency")

    args = parser.parse_args()

    if(args.device == "agilent_33250a"):
        args.device_addr = TRIUMF_AGILENT_33250A
    elif(args.device == "srs_ds360"):
        args.device_addr = TRIUMF_SRS_DS360
    else:
        args.device_addr = "GPIB::0::INSTR"

    args.channel = int(args.channel)
    if args.channel < 1:
        args.channel = 1
    elif args.channel > 16:
        args.channel = 16

    if(args.device != None):
        res = signal_generator.SignalGenerator(args.device)
        res.setup(args.device_addr, SIGNAL_GENERATOR_VOLTAGE, SIGNAL_GENERATOR_OFFSET)
    else:
        res = None

    fft_test(res, args, int(args.frequency))

def configure_moller_board(sim, ip, channel, divisor):
    global ADC_SAMPLE_RATE

    if(not sim):
        # Open up sockets to moller board
        ctrl_socket = moller_ctrl.ctrl_init(ip)

        channel = channel - 1
        ch = (int(channel) & 0xF)
        ch = (ch + (ch << 4)) << 16
        moller_ctrl.write_msg(ctrl_socket, 0x44, 0x80000000 | ((divisor-1) << 24) | ch | ADC_PACKET_SIZE)

    ADC_SAMPLE_RATE = (14705882 / divisor)

if __name__ == "__main__":
    main()