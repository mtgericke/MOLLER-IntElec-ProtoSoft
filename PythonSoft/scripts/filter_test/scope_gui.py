#!/bin/python

import matplotlib.pyplot as plt
import matplotlib.animation as animation
from matplotlib.collections import LineCollection
from matplotlib.widgets import TextBox
from matplotlib.offsetbox import AnchoredText
from matplotlib.widgets import SpanSelector
from matplotlib.ticker import EngFormatter

from collections import deque

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

import wx
from matplotlib.backends.backend_wxagg import FigureCanvasWxAgg as FigureCanvas
from matplotlib.figure import Figure


version = "1.0"

NUM_ADC_BITS = 18

CLOCKS_TO_NANOSECONDS = 4  # There are 4ns per clock

MIN_CONVERT_CLOCKS = 17
MAX_CONVERT_CLOCKS = 255

ADC_DIVISOR = 1
ADC_CONVERT_CLOCKS = int(MIN_CONVERT_CLOCKS*2)
ADC_CONVERT_TIME = (ADC_CONVERT_CLOCKS * CLOCKS_TO_NANOSECONDS)
ADC_SAMPLE_RATE = ((1.0/(ADC_CONVERT_TIME / 1000000000)) / ADC_DIVISOR) # Dividing the sample rate by 2 to prevent errors in transmission
ADC_PACKET_SIZE = (0x2004)
NUM_SAMPLES_PER_PACKET = ADC_PACKET_SIZE - 4
ADC_MAX_VOLTAGEpp = 4.096
ADC_RESOLUTION = ADC_MAX_VOLTAGEpp / pow(2, 18)

VISA_DEVICE_DELAY = 1.0

SIGNAL_GENERATOR_VOLTAGE = 2.048
SIGNAL_GENERATOR_OFFSET = 0

TRIUMF_AGILENT_33250A = "GPIB::10::INSTR"
TRIUMF_SRS_DS360 = "GPIB::8::INSTR"

# TODO: move these to args
NUM_SAMPLES = ADC_SAMPLE_RATE / 32
AVERAGING = 1
WINDOW_CORR_FACTOR = 2 #  https://community.sw.siemens.com/s/article/window-correction-factors

HISTOGRAM_AVG = 32

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

class Knob:
    """
    Knob - simple class with a "setKnob" method.
    A Knob instance is attached to a Param instance, e.g., param.attach(knob)
    Base class is for documentation purposes.
    """

    def setKnob(self, value):
        pass

class App(wx.App):
    def __init__(self, res, args):
        self.gpib_res = res
        self.cmd_args = args
        wx.App.__init__(self)

    def OnInit(self):
        self.frame1 = FourierDemoFrame(parent=None, title="Fourier Demo",
                                       size=(800, 600), gpib_res=self.gpib_res, cmd_args=self.cmd_args)
        self.frame1.Show()
        return True

class Param:
    """
    The idea of the "Param" class is that some parameter in the GUI may have
    several knobs that both control it and reflect the parameter's state, e.g.
    a slider, text, and dragging can all change the value of the frequency in
    the waveform of this example.
    The class allows a cleaner way to update/"feedback" to the other knobs when
    one is being changed.  Also, this class handles min/max constraints for all
    the knobs.
    Idea - knob list - in "set" method, knob object is passed as well
      - the other knobs in the knob list have a "set" method which gets
        called for the others.
    """

    def __init__(self, initialValue=None, minimum=0., maximum=1.):
        self.minimum = minimum
        self.maximum = maximum
        if initialValue != self.constrain(initialValue):
            raise ValueError('illegal initial value')
        self.value = initialValue
        self.knobs = []

    def attach(self, knob):
        self.knobs += [knob]

    def set(self, value, knob=None):
        self.value = value
        self.value = self.constrain(value)
        for feedbackKnob in self.knobs:
            if feedbackKnob != knob:
                feedbackKnob.setKnob(self.value)
        return self.value

    def constrain(self, value):
        if value <= self.minimum:
            value = self.minimum
        if value >= self.maximum:
            value = self.maximum
        return value

class SliderGroup(Knob):
    def __init__(self, parent, label, param):
        self.sliderLabel = wx.StaticText(parent, label=label)
        self.sliderText = wx.TextCtrl(parent, -1, style=wx.TE_PROCESS_ENTER)
        self.setKnob(param.value)

        sizer = wx.BoxSizer(wx.HORIZONTAL)
        sizer.Add(self.sliderLabel, 0,
                  wx.EXPAND | wx.ALL,
                  border=2)
        sizer.Add(self.sliderText, 0,
                  wx.EXPAND | wx.ALL,
                  border=2)
        self.sizer = sizer

        self.sliderText.Bind(wx.EVT_TEXT_ENTER, self.sliderTextHandler)

        self.param = param
        self.param.attach(self)

    def sliderHandler(self, event):
        value = event.GetInt() / 1000.
        self.param.set(value)

    def sliderTextHandler(self, event):
        value = float(self.sliderText.GetValue())
        self.param.set(value)

    def setKnob(self, value):
        self.sliderText.SetValue(f'{value:g}')
        # self.slider.SetValue(int(value * 1000))

class FourierDemoFrame(wx.Frame):
    def __init__(self, gpib_res, cmd_args, *args, **kwargs):
        super().__init__(*args, **kwargs)


        self.gpib_res = gpib_res
        self.cmd_args = cmd_args

        if(self.gpib_res):
            self.use_gpib = True
        else:
            self.use_gpib = False

        self.frequency = float(cmd_args.frequency)
        configure_moller_board(self.cmd_args.sim, self.cmd_args.ip, self.cmd_args.channel, ADC_DIVISOR, ADC_CONVERT_CLOCKS)

        if(self.gpib_res != None) and (self.use_gpib):
            self.gpib_res.set_frequency(self.frequency)

        self.graph_data = [[], []]

        self.top_panel = wx.Panel(self)
        self.nb = wx.Notebook(self.top_panel)
        fft_panel = wx.Panel(self.nb)
        histogram_panel = wx.Panel(self.nb)

        # # create the GUI elements
        self.createCanvas(fft_panel)
        control_sizer = self.createControls(fft_panel)
        fft_sizer = wx.BoxSizer(wx.HORIZONTAL)
        fft_sizer.Add(self.canvas, 1, wx.LEFT | wx.EXPAND)
        fft_sizer.Add(control_sizer, 0, wx.FIXED_MINSIZE, border=5)
        fft_panel.SetSizer(fft_sizer)

        # Create histogram
        self.createHistogram(histogram_panel)

        self.nb.AddPage(fft_panel, "FFT")
        self.nb.AddPage(histogram_panel, "Histogram")

        # Sizer for top panel
        top_sizer = wx.BoxSizer()
        top_sizer.Add(self.nb, 1, wx.EXPAND)
        self.top_panel.SetSizer(top_sizer)

        self.Bind(wx.EVT_NOTEBOOK_PAGE_CHANGING, self.onPageChange)
        self.Bind(wx.EVT_CLOSE, self.onCloseFrame)
        top_sizer.SetSizeHints(self)

    def onPageChange(self, event):
        # Tab 0 is FFT
        # Tab 1 is Histogram

        if(event.GetOldSelection() == 0):
            self.fft_anim.pause()

        if(event.GetOldSelection() == 1):
            self.histogram_anim.pause()

        if(event.GetSelection() == 0):
            self.fft_anim.resume()

        if(event.GetSelection() == 1):
            self.histogram_anim.resume()

    def onCloseFrame(self, event):
        self.fft_anim.pause() # Without this, it takes two clicks to close
        self.histogram_anim.pause()
        plt.close('all') # Force closure of all plots, otherwise wxWidgets closes but matplotlib does not!
        self.Destroy()

    def createHistogram(self, parent):

        self.graph_data = [[], []]


        #P = []
        #for n in range(-pow(2,NUM_ADC_BITS-1), pow(2,NUM_ADC_BITS-1), 1):
        #    P.append( 1 / np.pi * (np.arcsin((ADC_MAX_VOLTAGEpp * (n - pow(2, NUM_ADC_BITS-1))) / (SIGNAL_GENERATOR_VOLTAGE * pow(2, NUM_ADC_BITS))) -
        #                           np.arcsin((ADC_MAX_VOLTAGEpp * (n - 1 - pow(2, NUM_ADC_BITS-1))) / (SIGNAL_GENERATOR_VOLTAGE * pow(2, NUM_ADC_BITS))) ))
#

        self.hist_figure = Figure(layout='constrained')
        self.hist_canvas = FigureCanvas(parent, -1, self.hist_figure)

        hist_sizer = wx.BoxSizer(wx.HORIZONTAL)
        hist_sizer.Add(self.hist_canvas, 1, wx.EXPAND)
        parent.SetSizer(hist_sizer)

        self.hist_chart_zoom, self.hist_chart = self.hist_figure.subplots(2, 1, height_ratios=[1, 5])

        self.hist_queue = deque()
        self.hist_sum = np.zeros(pow(2,NUM_ADC_BITS))
        self.hist_bins = np.arange(-pow(2,NUM_ADC_BITS-1), pow(2,NUM_ADC_BITS-1), 1)
        self.hist_data = np.zeros(pow(2,NUM_ADC_BITS))

        self.hist_zoom_ln, = self.hist_chart_zoom.step(self.hist_bins, self.hist_data, where='mid')
        self.hist_bins_ln, = self.hist_chart.step(self.hist_bins, self.hist_data, where='mid')
        # self.hist_expected_ln, = self.hist_chart.plot(self.hist_bins, P)

        self.hist_chart_zoom_xmin = -pow(2,NUM_ADC_BITS-1)
        self.hist_chart_zoom_xmax = pow(2,NUM_ADC_BITS-1)

        def onselect(xmin, xmax):
            self.hist_chart_zoom_xmin = xmin
            self.hist_chart_zoom_xmax = xmax
            if(xmin == xmax):
                self.hist_chart_zoom_xmin = -pow(2,NUM_ADC_BITS-1)
                self.hist_chart_zoom_xmax = pow(2,NUM_ADC_BITS-1)

        self.hist_span = SpanSelector(self.hist_chart_zoom, onselect, 'horizontal', useblit=False, props=dict(alpha=0.5, facecolor="tab:blue"), interactive=True, drag_from_anywhere=True, snap_values=np.arange(-pow(2,NUM_ADC_BITS-1), pow(2,NUM_ADC_BITS-1), pow(2,8)))

        def get_data(frequency):

            data = {'time': [], 'samples': []}
            samples_to_take = ADC_SAMPLE_RATE / HISTOGRAM_AVG

            if(self.cmd_args.sim):
                Fs = ADC_SAMPLE_RATE
                sample_time = np.arange(0, samples_to_take * ADC_CONVERT_TIME * ADC_DIVISOR, ADC_CONVERT_TIME * ADC_DIVISOR)

                sample_time = sample_time.astype(np.int32)
                sample_data = np.sin(2 * np.pi * frequency / (ADC_CONVERT_TIME * ADC_DIVISOR) * sample_time / Fs) * (pow(2,NUM_ADC_BITS-1) - 2)

                # Generate harmonics
                # for n in range(20):
                #     if(n % 2 == 0):
                #         sample_data = sample_data + np.sin(2 * np.pi * (frequency * (n * 0.5)) / (68 * ADC_DIVISOR) * sample_time / Fs) * pow(2,12)
                #     sample_data = sample_data + np.sin(2 * np.pi * (frequency * n) / (68 * ADC_DIVISOR) * sample_time / Fs) * pow(2,7-n)

                # Add some noise
                sample_data = sample_data + np.random.normal(0, np.sqrt(0.1), len(sample_data))

                # Convert to integer values to match what the ADC produces
                sample_data = sample_data.astype(np.int32)

                data['time'] = sample_time
                data['samples'] = sample_data
            else:
                socket = moller_ctrl.data_init(self.cmd_args.ip)
                samples = moller_ctrl.read_samples(socket, samples_to_take, ADC_CONVERT_TIME, ADC_PACKET_SIZE, True)
                time_data = []
                sample_data = []
                for sample in samples:
                    time_data.append(sample[0])
                    sample_data.append(sample[2])

                data['time'] = np.array(time_data)
                data['samples'] = np.array(sample_data)

                socket.close()

            return data

        def update(i):
            data = get_data(self.frequency)

            hist, _ = np.histogram(data['samples'], pow(2,18), range=(-pow(2,17), pow(2,17)), density=False)

            self.hist_sum = self.hist_sum + hist
            self.hist_queue.append(hist)
            if(len(self.hist_queue) > (HISTOGRAM_AVG*4)):
                last = self.hist_queue.popleft()
                self.hist_sum = self.hist_sum - last

            # print(len(self.hist_queue))

            self.hist_bins_ln.set_ydata(self.hist_sum)
            self.hist_zoom_ln.set_ydata(self.hist_sum)

            self.hist_chart.set_ylim([-1, np.max(self.hist_sum)])
            self.hist_chart.set_xlim([self.hist_chart_zoom_xmin, self.hist_chart_zoom_xmax])

            self.hist_chart_zoom.set_ylim([-0.2, 20])
            self.hist_chart_zoom.set_xlim([-pow(2,NUM_ADC_BITS-1), pow(2,NUM_ADC_BITS-1)])

            # x = data['time']
            # y = data['samples']
#
            # regr = linear_model.LinearRegression()
            # regr.fit(x[:,np.newaxis],y)
            # y_fit = regr.predict(x[:,np.newaxis])
            #
            # bits=NUM_ADC_BITS
            # onebit = 1.0/(2**bits)
            # stepsz = (np.max(y)-np.min(y))/len(y)
            # nsteps_per_lsb = onebit/stepsz
            # total_lsb_steps = int(len(y)/nsteps_per_lsb)
            # steps_either_side = int(nsteps_per_lsb/2)
            # centers = [int(round(nsteps_per_lsb*_)) for _ in range(total_lsb_steps)]
            # dnl = [np.average(y[centers[hi]-steps_either_side:centers[hi]+steps_either_side])-np.average(y[centers[low]-steps_either_side:centers[low]+steps_either_side]) for low, hi in zip(range(total_lsb_steps-1),range(1,total_lsb_steps))]
            # dnl = np.array(dnl)
            # dnl = (dnl-onebit)/onebit
            # lower = -int(len(dnl)/2)
            # upper = lower+len(dnl)
            # dnl_x = np.array(range(lower,upper))
            # print("x:",len(x))
            # print("dnl:",len(dnl))
            # #lower = -int(len(dnl)/2)
            # #upper = lower+len(dnl)
            # inl = (y-y_fit)/onebit
            # print(inl)
            # #print("Coefficients:\n",regr.coef_)
            # #print("intercept:\n",regr.intercept_)

            return self.hist_bins_ln, self.hist_zoom_ln

        interval = 20

        self.histogram_anim = animation.FuncAnimation(
            self.hist_figure, update, interval=interval, blit=False, cache_frame_data=False,
        )

    def createCanvas(self, parent):
        self.lines = []
        self.figure = Figure(layout='constrained')
        self.canvas = FigureCanvas(parent, -1, self.figure)
        self.state = ''
        self.f0 = Param(self.frequency, minimum=1., maximum=8000000.)
        self.A = Param(1., minimum=0.01, maximum=2.)
        self.createPlots()

        # Not sure I like having two params attached to the same Knob,
        # but that is what we have here... it works but feels kludgy -
        # although maybe it's not too bad since the knob changes both params
        # at the same time (both f0 and A are affected during a drag)
        self.f0.attach(self)
        # self.A.attach(self)

    def createControls(self, panel):

        sizer = wx.BoxSizer(wx.VERTICAL)

        self.frequencySliderGroup = SliderGroup(
            panel,
            label='Frequency:',
            param=self.f0)

        self.cb_use_gpib = wx.CheckBox(panel, label = 'Use GPIB')
        self.cb_use_gpib.SetValue( self.use_gpib )
        self.Bind(wx.EVT_CHECKBOX, self.OnUseGPIB, self.cb_use_gpib)

        sizer.Add(self.frequencySliderGroup.sizer, 0,
                  wx.EXPAND | wx.ALL, border=5)

        sizer.Add(self.cb_use_gpib, 0,
                  wx.EXPAND | wx.ALL, border=5)

        return sizer

    def OnUseGPIB(self, evt):

        self.use_gpib = self.cb_use_gpib.IsChecked()
        if self.gpib_res != None:
            self.gpib_res.set_remote(self.use_gpib)

    def setKnob(self, value):
        pass

    def createPlots(self):
        self.subfigs = self.figure.subfigures(2, 1, height_ratios=[1, 5])
        self.dat_chart, self.coh_chart = self.subfigs[0].subplots(1, 2, sharey=True)

        self.fft_chart_zoom, self.fft_chart = self.subfigs[1].subplots(2, 1, sharex=False, height_ratios=[1, 5])

        self.coh_chart_lc = LineCollection([], linewidth=0.1, colors='blue')
        self.coh_chart_stepped_lc = LineCollection([], linewidth=0.1, colors='red')
        self.coh_chart.add_collection(self.coh_chart_lc)
        self.coh_chart.add_collection(self.coh_chart_stepped_lc )

        self.fft_zoom_ln, = self.fft_chart_zoom.plot([], [], color='blue', linewidth=0.2)

        self.fft_chart_lc = LineCollection([])
        self.fft_chart.add_collection( self.fft_chart_lc )

        self.dat_ln, = self.dat_chart.plot([], [])
        self.fft_ln, = self.fft_chart.plot([], [], color='blue', linewidth=0.4)
        self.fft_harmonic_annotation = []
        for n in range(0, HARMONICS_TO_ANNOTATE):
            self.fft_harmonic_annotation.append( self.fft_chart.annotate('',
                xy=(0, 0), xycoords='data',
                xytext=(1.5, 1.5), textcoords='offset points'))


        self.txt_info = AnchoredText("", loc='upper right')
        self.fft_chart.add_artist(self.txt_info)

        self.fft_chart_zoom_xmin = 0
        self.fft_chart_zoom_xmax = ADC_SAMPLE_RATE/2

        def onselect(xmin, xmax):
            self.fft_chart_zoom_xmin = xmin
            self.fft_chart_zoom_xmax = xmax
            if(xmin == xmax):
                self.fft_chart_zoom_xmin = 0
                self.fft_chart_zoom_xmax = ADC_SAMPLE_RATE/2

        step = 5000
        num = ADC_SAMPLE_RATE
        start = 0
        self.span = SpanSelector(self.fft_chart_zoom, onselect, 'horizontal', useblit=False, props=dict(alpha=0.5, facecolor="tab:blue"), interactive=True, drag_from_anywhere=True, snap_values=np.arange(0,num)*step+start)
        # self.span.extents = (self.fft_chart_zoom_xmin, self.fft_chart_zoom_xmax)

        def get_data(frequency):

            data = {'time': [], 'samples': []}

            if(self.cmd_args.sim):
                Fs = ADC_SAMPLE_RATE
                sample_time = np.arange(0, NUM_SAMPLES * ADC_CONVERT_TIME * ADC_DIVISOR, ADC_CONVERT_TIME * ADC_DIVISOR)

                sample_time = sample_time.astype(np.int32)
                sample_data = np.sin(2 * np.pi * frequency / (ADC_CONVERT_TIME * ADC_DIVISOR) * sample_time / Fs) * pow(2,17)

                # Generate harmonics
                # for n in range(20):
                #     if(n % 2 == 0):
                #         sample_data = sample_data + np.sin(2 * np.pi * (frequency * (n * 0.5)) / (68 * ADC_DIVISOR) * sample_time / Fs) * pow(2,12)
                #     sample_data = sample_data + np.sin(2 * np.pi * (frequency * n) / (68 * ADC_DIVISOR) * sample_time / Fs) * pow(2,7-n)

                # Add some noise
                sample_data = sample_data + np.random.normal(0, np.sqrt(0.1), len(sample_data))

                # Convert to integer values to match what the ADC produces
                sample_data = sample_data.astype(np.int32)

                data['time'] = sample_time
                data['samples'] = sample_data
            else:
                socket = moller_ctrl.data_init(self.cmd_args.ip)
                samples = moller_ctrl.read_samples(socket, NUM_SAMPLES, ADC_CONVERT_TIME, ADC_PACKET_SIZE, True)
                time_data = []
                sample_data = []
                for sample in samples:
                    time_data.append(sample[0])
                    sample_data.append(sample[2])

                data['time'] = np.array(time_data)
                data['samples'] = np.array(sample_data)

                socket.close()

            return data

        def init():
            hertz_formatter = EngFormatter(unit='Hz')
            volt_formatter = EngFormatter(unit='V')
            time_formatter = EngFormatter(unit='s')

            self.dat_chart.set_xlim(0,(NUM_SAMPLES*ADC_CONVERT_TIME*ADC_DIVISOR))
            self.dat_chart.set_ylim(-(ADC_MAX_VOLTAGEpp/2), ADC_MAX_VOLTAGEpp/2)
            self.dat_chart.yaxis.set_major_locator(plt.MultipleLocator(ADC_MAX_VOLTAGEpp/4))
            self.dat_chart.yaxis.set_major_formatter(volt_formatter)
            self.dat_chart.xaxis.set_major_formatter(time_formatter)
            self.dat_chart.grid(True)

            self.coh_chart.set_xlim(0,(NUM_SAMPLES*ADC_CONVERT_TIME*ADC_DIVISOR))
            self.coh_chart.set_ylim(-(ADC_MAX_VOLTAGEpp/2), ADC_MAX_VOLTAGEpp/2)
            self.coh_chart.yaxis.set_major_locator(plt.MultipleLocator(ADC_MAX_VOLTAGEpp/4))
            self.coh_chart.yaxis.set_major_formatter(volt_formatter)
            self.coh_chart.xaxis.set_major_formatter(time_formatter)
            self.coh_chart.grid(True)

            self.fft_chart.grid(True)
            self.fft_chart.set_xlabel("Frequency (Hz)")
            self.fft_chart.set_ylabel("Power (dBFS)")
            self.fft_chart.xaxis.set_major_formatter(hertz_formatter)

            self.fft_chart_zoom.xaxis.set_major_formatter(hertz_formatter)
            self.fft_chart_zoom.grid(True)

            return self.dat_ln, self.fft_ln, self.fft_zoom_ln, self.coh_chart_lc, self.coh_chart_stepped_lc

        def update(i):


            if(self.frequency != self.f0.value):
                self.frequency = self.f0.value
                if(self.gpib_res != None) and self.use_gpib:
                    self.gpib_res.set_frequency(self.frequency)

            if(self.span.extents[0] == self.span.extents[1]):
                self.fft_chart_zoom_xmin = 0
                self.fft_chart_zoom_xmax = ADC_SAMPLE_RATE/2

            plot_data = get_data(self.frequency)

            time_with_zc = plot_data['time'] / 1000000000  # Put into seconds
            data_with_zc = plot_data['samples']

            samples_per_period = int(ADC_SAMPLE_RATE / self.frequency)
            num_periods = math.floor(len(data_with_zc) / samples_per_period)
            if num_periods == 0:
                num_periods = 1

            total_segments_len = num_periods * samples_per_period
            if total_segments_len > len(data_with_zc):
                total_segments_len = len(data_with_zc)

            # Data for 'raw' data plot
            # Limit display to six periods
            if(num_periods > 5):
                num_periods_to_display = 6
            else:
                num_periods_to_display = num_periods

            dat_plot_sample_count = int(ADC_SAMPLE_RATE / self.frequency * num_periods_to_display)
            if(dat_plot_sample_count == 0):
                dat_plot_sample_count = len(data_with_zc)
            elif dat_plot_sample_count > len(data_with_zc):
                dat_plot_sample_count = len(data_with_zc)

            self.dat_chart.set_xlim([0, time_with_zc[dat_plot_sample_count-1]])
            self.dat_ln.set_data(time_with_zc[0:dat_plot_sample_count], data_with_zc[0:dat_plot_sample_count] * ADC_RESOLUTION)

            # Prep for coherent plot, break data into segments
            self.coh_chart.set_xlim(0,(samples_per_period*ADC_CONVERT_TIME*ADC_DIVISOR))

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

            if(samples_per_period > len(time_segment)):
                samples_per_period = len(time_segment)

            self.coh_chart.set_xlim([0, time_segment[samples_per_period-1]])
            self.coh_chart_lc.set_segments( coherent_segments )
            self.coh_chart_stepped_lc.set_segments( coherent_segments_stepped )

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

            self.fft_chart.set_xlim([self.fft_chart_zoom_xmin, self.fft_chart_zoom_xmax])
            self.fft_chart.set_ylim([-180,5])
            self.fft_ln.set_data( fft_bins, p_dB)

            self.fft_chart_zoom.set_xlim([0, ADC_SAMPLE_RATE/2])
            self.fft_chart_zoom.set_ylim([-180,5])
            self.fft_zoom_ln.set_data( fft_bins, p_dB)

            bin_size = fft_bins[1] # in Hz
            SNR_ideal = -(6.02 * (NUM_ADC_BITS-1) + 1.76)
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

            distance = (self.frequency/bin_size) * 0.98
            if distance < 1.0:
                distance = 1.0

            # Get peaks
            peaks, peak_properties = sp.signal.find_peaks(p_dB, height=FFT_floor_ideal+25, prominence=5, width=1, distance=distance)

            # Removes peaks lower in frequency than EXPECTED fundamental frequency
            peaks = (peaks[peaks > ((self.frequency / bin_size) - bin_size)])

             # Attempt to fit found peaks to harmonics
            harmonics = []
            for n in range(1, HARMONICS_TO_CAPTURE):
                found = False
                db = 0
                freq = 0
                fft_i = 0
                peak_i = 0
                for peak_index, peak in enumerate(peaks):
                    if(math.isclose(fft_bins[peak], (self.frequency * n), rel_tol=0.01)):
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

                Pn = 0 # Avoid divide by zero
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
                if n < len(self.fft_harmonic_annotation):
                    if(harmonic[1] != 0):
                        self.fft_harmonic_annotation[n].set_text( "F" + str(harmonics[n][0]) )
                        harmonic_name.append("F" + str(n+1))
                        harmonic_freq.append(harmonic[1])
                        harmonic_db.append(harmonic[2])
                        strHarmonic = strHarmonic + "F" + str(n+1) + " {:0.2f}".format(harmonic[2]) + " dB\n"
                    else:
                        self.fft_harmonic_annotation[n].set_text( "" )
                        strHarmonic = strHarmonic + "F" + str(n+1) + " Missing\n"
                    self.fft_harmonic_annotation[n].xy = harmonics[n][1], harmonics[n][2]

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

            self.fft_chart_lc.set_linestyle( 'dashed' )
            self.fft_chart_lc.set_segments( fft_annotations )
            self.fft_chart_lc.set_colors( fft_annotations_colors )

            if(not self.cmd_args.sim):
                strChannel = "Channel " + str(self.cmd_args.channel) + "\n"
            else:
                strChannel = "Simulation\n"

            self.txt_info.txt.set_text(
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

            return self.dat_ln, self.fft_ln, self.fft_zoom_ln, self.coh_chart_lc, self.coh_chart_stepped_lc


        plt.grid(True)
        plt.ioff()

        interval = 250

        self.fft_anim = animation.FuncAnimation(
            self.figure, update, init_func=init, interval=interval, blit=False, cache_frame_data=False,
        )

        # # Blocks until closed
        # plt.show()
    #
        # # Tell thread to end
        # flag.change_state(0, 0)


def main():
    prog='scope_gui'
    parser = argparse.ArgumentParser(prog=prog)
    parser.add_argument('--version', action='version', version='%(prog)s ' + version)
    parser.add_argument("-d", "--device", default=None, choices=["agilent_33250a", "srs_ds360"], help="")
    parser.add_argument("-s", "--sim", action='store_true')
    parser.add_argument("ip", help="")
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

    app = App(res, args)
    app.MainLoop()

def configure_moller_board(sim, ip, channel, divisor, sample_clocks):
    global ADC_SAMPLE_RATE

    if(sample_clocks < MIN_CONVERT_CLOCKS):
        sample_clocks = MIN_CONVERT_CLOCKS

    if(sample_clocks > MAX_CONVERT_CLOCKS):
        sample_clocks = MAX_CONVERT_CLOCKS

    if(not sim):
        # Open up sockets to moller board
        ctrl_socket = moller_ctrl.ctrl_init(ip)

        channel = channel - 1
        ch = (int(channel) & 0xF)
        ch = (ch + (ch << 4)) << 16
        moller_ctrl.write_msg(ctrl_socket, 0x44, 0x80000000 | ((divisor-1) << 24) | ch | ADC_PACKET_SIZE)
        moller_ctrl.write_msg(ctrl_socket, 0x48, 0x80000000 | ((sample_clocks & 0xFF) << 16))

    ADC_SAMPLE_RATE = ((1.0/((sample_clocks * CLOCKS_TO_NANOSECONDS) / 1000000000)) / divisor)


if __name__ == "__main__":
    main()
