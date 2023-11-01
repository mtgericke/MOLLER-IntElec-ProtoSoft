import matplotlib.pyplot as plt
import matplotlib.colors as colors
import numpy as np
import argparse

version = "1.0"

def main():
    prog='moller_viewer_phase'
    parser = argparse.ArgumentParser(prog=prog)
    parser.add_argument('--version', action='version', version='%(prog)s ' + version)
    parser.add_argument("file", nargs="?", default="phase.txt", help="file to plot")
    args = parser.parse_args()

    data = np.loadtxt(args.file, delimiter=',', unpack=True)

    colormap = plt.cm.get_cmap("OrRd").copy() #or any other colormap
    colormap.set_under(color='green')

    normalize = colors.Normalize(vmin=1,vmax=np.amax(data))

    plt.pcolormesh(data, norm=normalize, cmap=colormap, edgecolors='k', linewidth=0.5)
    plt.ylabel('ADC Channel')
    plt.xlabel('Phase Position')
    plt.xticks(np.arange(0.5, 56.5, 1))
    plt.yticks(np.arange(0.5, 16.5, 1))
    plt.gca().set_xticklabels(np.arange(1, 57, 1))
    plt.gca().set_yticklabels(np.arange(1, 17, 1))

    plt.colorbar(label="Number of errors detected", orientation="horizontal")
    plt.show()

if __name__ == "__main__":
    main()