# Software Front-end for testing the MOLLER/P2 Integrating Electronics Prototypes 

## This was successfully compiled with

* Ubuntu 20.04.2 LTS
* 5.4.0-70-generic
* ROOT 6.22/08 with all required and recommended support programs
* The program uses the ROOT graphical interface components (TG*)
* sudo apt install libzmq3-dev

## Project Layout

* /lib - Previously written data display utility library
* /include
* /source

## Setup

* 'mkdir <some directory>'
* 'cd <some directory>'
* clone the repository
* 'cd lib'
* 'mkdir build'
* 'cd build'
* 'cmake ..'
* 'make'
* 'cd ..'
* 'mkdir build'
* 'cd build'
* 'cmake ..'
* 'make'
* cp CMMonitor ..
* cd ..


## Running

* './CMMonitor' (without commandline arguments - connection can be set via menu)
* Note: moller_cntr.py and CMMonitorSettings.txt must be in the directory from which CMMonitor is started
* Follow these steps:
* Set the ip address from the menu (only has to be done ones when the address changes)
* The run number, run time, prescale, ch0, ch1 setting are read from the CMMonitorSettings.txt file. Adjust those as needed (see below).
* Select "Connect" (MUST BE DONE AFTER ALL OF THE ABOVE INFORMATION IS ENTERED) from the "Data" menu and then press "Start"
* The program also has the ability to take multiple runs at a time. The number of runs can be specified at right end of the toolbar.
* Data is added to all histograms cummulatively (except in multi-run mode) so plots can get very large and slow to load (in ROOT). The "Clear Plots" button resets the data in the ROOT plots. It does not clear the actuall data writtent to file, which happens very quickly after the "Start" of the run.

# Menus

* File/Open(Data File): Read in a previously taken run
* File/Save(Settings): Modifies the CMMonitorSettings.txt file with the options that are currently entered in the Toolbar. Caution: This overrides the information about the last raken run in that diectory.
* Data/Set IP: Set the ip address of the ADC board here
* Data/Connect: Connects the board to the ADC
* Data/Stop: Force stops the data taking process

## Inputs and Buttons:

* Run: Run number - incremented automatically after the run is taken and stored in CMMonitorSettings.txt
* Run Time: Number of second length of the run. Note that the runs should be very short (a few seconds at most), since a lot of data is currently written, depending on the sampling rate. Depending on the network speed (currently only copper Eth0) there could be gaps in the data if the run is too long, unless the rate is decereased with "Prescale".
*Prescale: The sampling rate of the ADC is 14705883 Smpls/sec and currently every sample is taken and transferred. The Prescale setting allows to decrease the number of buffered and transferred samples. A Prescale=2 means only every other sample is buffered and transferred. If the network connection or the computer running this software is not fast enough, not all buffered samples will be transferred in time, before the buffer is overwritten on the FPGA SoM. So gaps may occur in the colleced data. For normal computers and network connections a larger prescale factor will prevent this (of course also reducing the available bandwidth information).
*Ch0/Ch1: Set the two channels to be read out simultaneously (both can be chosen between 1-16). Currently at most two channels can be read out at the same time, again, due to speed limitations, but all channels on the 16 channel ADC board are operational.
*Start: Start a run after selecting "Connect" from the "Data" menu. The run only takes a short moment to be written to file (binary file), but filling the ROOT plots may take a little longer. The histogram shows the progrss.
* Time Graph: Generates plots of the data as a function of time (from the sample timestamp in each run)
* Histo (HR): Generates a higher resolution histogram of the data. Same as the one that is filled at first, but with a higher bin resolution
* FFT: Generates the FFT of the data
* Clear Plots: Clear the plots and resets them to zero. Can be used if things take too long to plot.


#### Hardware

This program looks for the 16 ADC board with the specified ip address. The board should be powered and booted, which
it does normally on power on. Look for the 4 vertical LEDs pattern at the front of the board, which
should be [on,on,off,on] from top to bottom. This pattern means the board booted correctly.

### Data Rate

The data rate from the FPGA board is very high, currently only being read out in streaming mode.
So the graphs in the program should be reset afer a while, but the data is written to a binary file. Nonetheless,
the files become very large if runs are too long.

### Firmware

Presently, the easiest way to load the firmware and boot the SoM, is with an SDcard that can be inserted on the side and underneath the SoM module.
The 

1. Making an SDcard image (Warning: Assumes /dev/sdb is an SDcard!)

   1. `sudo parted /dev/sdb --script -- mklabel msdos`
   2. `sudo parted /dev/sdb --script -- mkpart primary fat32 1MiB 100%`
   3. `sudo mkfs.vfat -F32 /dev/sdb1`
   4. `sudo fatlabel /dev/sdb1 PETALINUX`

2. Copy files to SDCard
   
   1. `unzip -p ${SDCARD_ZIP} BOOT.BIN >/media/${USER}/${SDCARD_BOOT_LABEL}/BOOT.BIN`
   2. `unzip -p ${SDCARD_ZIP} boot.scr >/media/${USER}/${SDCARD_BOOT_LABEL}/boot.scr`
   3. `unzip -p ${SDCARD_ZIP} Image >/media/${USER}/${SDCARD_BOOT_LABEL}/Image`
   4. `unzip -p ${SDCARD_ZIP} rootfs.tar.gz | sudo tar -C /media/${USER}/${SDCARD_ROOTFS_LABEL} -zxvf -`
   
Notes:

   * Replace ${SDCARD_ZIP} with the name of the zip file (currently moller_20220505.zip)
   * Replace ${USER} with your usename in linux
   * Replace ${SDCARD_BOOT_LABEL} with BOOT
   * Replace ${SDCARD_ROOTFS_LABEL} with PETALINUX
