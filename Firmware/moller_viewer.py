#!/bin/python

import matplotlib.pyplot as plt
import matplotlib.ticker as tck
import numpy as np
import argparse
import datetime
import os
import struct

version = "1.0"

ADC_CONVERSION = (0.00003125)
TS_TO_NS = 4  # nanoseconds per timestamp tick
TS_CONVERSION = 17 # Number of cycles to convert (68ns  / 4ns) at 250Mhz

mult = 0.000000068

def snap_to_multiple(n: float, multiple: float):
    n = int(n / multiple) * multiple
    return n

def ch_to_str(ch, data):
    txt = ""
    for n in range(16):
        if(ch == n):
            txt = txt + str(data)
        else:
            txt = txt + "0"

        if(n < 15):
            txt = txt + ","
    return txt

def chs_to_str(ch0, data0, ch1, data1):
    txt = ""
    for n in range(16):
        if(ch0 == n):
            txt = txt + str(data0)
        elif(ch1 == n):
            txt = txt + str(data1)
        else:
            txt = txt + "0"

        if(n < 15):
            txt = txt + ","
    return txt


# Custom formatter class
class CustomFormatter(tck.Formatter):
    def __init__(self, ax):
        super().__init__()
        self.set_axis(ax)


    def __call__(self, x, pos=None):
        # Find the axis range
        vmin, vmax = self.axis.get_view_interval()
        range = (vmax - vmin) / mult
        if(range < 1000):
            scale_factor = 1000000000
            prefix = "ns"
        elif(range < 100000):
            scale_factor = 1000000
            prefix = "us"
        elif(range < 100000000):
            scale_factor = 1000
            prefix = "ms"
        else:
            scale_factor = 1
            prefix = "s"

        # Use the range to define the custom string
        if(x-vmin) > mult:
            if(prefix == "ns"):
                return f"{int(x*scale_factor):d}" + prefix + " [" + f"{int((x-vmin)*scale_factor):d}" + prefix + "]"
            else:
                return f"{(x*scale_factor):.1f}" + prefix + " [" + f"{((x-vmin)*scale_factor):.1f}" + prefix + "]"
        else:
            if(prefix == "ns"):
                return f"{int(x*scale_factor):d}" + prefix
            else:
                return f"{(x*scale_factor):.1f}" + prefix


def convert_stream_segment(infile, outfile):
    stream_file = open(infile, "rb")
    sample_data = bytearray(stream_file.read())
    stream_file.close()

    text_file = open(outfile, "wt")

    read_bytes = 0
    while(read_bytes < len(sample_data)):
        num_words, num_pkt, id, ts = struct.unpack_from("<HIxBQ", sample_data, read_bytes)
        num_samples = num_words - 1
        for n in range(num_samples):
            ch0, ch1 = struct.unpack_from("<ii", sample_data, read_bytes + 16 + (n * 8))
            ch1_data = ch1 >> 14
            ch0_data = ch0 >> 14

            ch1_sel = ch1 & 0xF
            ch0_sel = ch0 & 0xF

            stream_div = ((ch0 >> 4) & 0x7F) + 1

            if(ch0_sel == ch1_sel):
                text_file.write(str((ts + ((n*2) * TS_CONVERSION * stream_div)) *  TS_TO_NS) + "," + str(stream_div) + "," + ch_to_str(ch1_sel, ch1_data) + "\n")
                text_file.write(str((ts + (((n*2)+1) * TS_CONVERSION * stream_div)) * TS_TO_NS)+ "," + str(stream_div) + "," + ch_to_str(ch0_sel, ch0_data) + "\n")
            else:
                text_file.write(str((ts + (n * TS_CONVERSION * stream_div)) * TS_TO_NS) + "," + str(stream_div) + "," + chs_to_str(ch0_sel, ch0_data, ch1_sel, ch1_data) + "\n")

        read_bytes = read_bytes + 16 + (num_samples * 8)

    text_file.close()

def main():
    prog='moller_viewer'
    parser = argparse.ArgumentParser(prog=prog)
    parser.add_argument('--version', action='version', version='%(prog)s ' + version)
    parser.add_argument("file", nargs="?", default="sample_data.raw", help="file to plot")
    args = parser.parse_args()

    print("Converting raw to text")
    convert_stream_segment("sample_data.raw", "sample_data.txt")

    print("Loading text")
    ts, divider, ch1, ch2, ch3, ch4, ch5, ch6, ch7, ch8, ch9, ch10, ch11, ch12, ch13, ch14, ch15, ch16 = np.loadtxt("sample_data.txt", dtype=int, delimiter=',', unpack=True, skiprows=1)

    # Change to elapsed time from start of capture
    print("Removing ts offset")
    initial_ts = ts[0]
    np.place(ts, ts > 0, (ts - initial_ts))

    print("Converting ts to float")
    ts = ts.astype(np.float64)
    np.place(ts, ts > 0, ts / 1000000000.0)

    print("Making plot")
    ch = [ch1, ch2, ch3, ch4, ch5, ch6, ch7, ch8, ch9, ch10, ch11, ch12, ch13, ch14, ch15, ch16]

    fig, ax = plt.subplots()
    ax.set_title('Click on legend line to toggle line on/off')
    ax.use_sticky_edges = True

    formatter = CustomFormatter(ax)
    ax.xaxis.set_major_formatter(formatter)

    lines = []
    for n in range(16):
        lines.append(ax.plot(ts, ch[n], lw=2, label="CH" + str(n+1)))
        leg = ax.legend(fancybox=True, shadow=False, loc="center right")

    lined = {}  # Will map legend lines to original lines.
    for legline, origline in zip(leg.get_lines(), lines):
        legline.set_picker(True)  # Enable picking on the legend line.
        lined[legline] = origline[0]

        visible = np.any(lined[legline].get_ydata(orig=True))
        lined[legline].set_visible(visible)
        legline.set_alpha(1.0 if visible else 0.2)


    def on_pick(event):
        # On the pick event, find the original line corresponding to the legend
        # proxy line, and toggle its visibility.
        legline = event.artist
        origline = lined[legline]
        visible = not origline.get_visible()
        origline.set_visible(visible)
        # Change the alpha on the line in the legend so we can see what lines
        # have been toggled.
        legline.set_alpha(1.0 if visible else 0.2)
        fig.canvas.draw()

    def on_zoom(event):
        x_min, x_max = ax.get_xlim()
        x_min = snap_to_multiple(x_min, mult)
        x_max = snap_to_multiple(x_max, mult)

        num_ticks = int((x_max-x_min) / mult)

        tick_locs = np.arange(x_min, x_max, (int(num_ticks/10)*mult)+mult)
        ax.set_xticks(tick_locs)

        # had to add flush_events to get the ticks to redraw on the last update.
        fig.canvas.flush_events()

    fig.canvas.mpl_connect('button_release_event', on_zoom)
    fig.canvas.mpl_connect('draw_event', on_zoom)
    fig.canvas.mpl_connect('resize_event', on_zoom)
    fig.canvas.mpl_connect('pick_event', on_pick)

    # plt.ylim(-131072, 131072)
    plt.margins(0, 0)
    plt.grid(which='major', linestyle='-', linewidth=1.5)
    plt.show()

if __name__ == "__main__":
    main()