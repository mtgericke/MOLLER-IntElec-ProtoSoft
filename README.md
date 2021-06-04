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

## Running

Either
* './CMMonitor' (without commandline arguments - connection can be set via menu)
Or
* 'CMMonitor <ipaddress:port> <ROOT output filename>'


## Notes

### Hardware

This program currently interfaces to the Enclustra PE1+ FPGA evluation board via standard network connection.
The FPGA board connects to the current two channel ADC prototype board. The needed settings for the FPGA board
are provided below.

### Data Rate

The data rate from the FPGA board is very high, currently only being read out in streaming mode.
So the graphs in the program reset afer a while, but the data is written to a ROOT tree. Nonetheless,
the root files become very large (or there are many of them).

### PE1

The I/O voltage jumpers on the Mercurey PE1+ baseboard need to be
configured as follows
- VSEL A = 1.8 (position B)
- VSEL B = 1.8 (position B)

#### DIP Switches

1. CFG A

   | DIP Switch | Position
   |:-|:-|
   |CFG A 1|OFF (ON for JTAG boot)
   |CFG A 2|OFF
   |CFG A 3|OFF
   |CFG A 4|ON

1. CFG B

   | DIP Switch | Position
   |:-|:-|
   |CFG B 1|OFF
   |CFG B 2|OFF
   |CFG B 3|ON
   |CFG B 4|OFF

1. USER

   These are all don't cares at the moment, but may be used in the future

   | DIP Switch | Position
   |:-|:-|
   |CFG U 1|OFF
   |CFG U 2|OFF
   |CFG U 3|OFF
   |CFG U 4|OFF

#### JTAG Boot

To boot from JTAG dip switch `CFG A 1` needs to be in the `ON` position. Additionally, while the PE1 is powering up, resistor `R252` needs to be short-circuited on the Enclustra XU-8 module. See page 48 of the Mercury XU8 User Manual V06 for details.

![alt text](docs/pe1_xu8_jtag_boot_resistor.png "Logo Title Text 1")

### Common Problems

1. USB / JTAG doesn't work - Even though the docker can perform the JTAG operation, the Xilinx cable drivers need to be installed on the HOST in order to function

   1. Download `Xilinx_Unified_2020.1_0602_1208.tar.gz` and place it in the `docker` directory.
   1. Run `tar zxvf Xilinx_Unified_2020.1_0602_1208.tar.gz`
   1. Run `cd Xilinx_Unified_2020.1_0602_1208`
   1. Run ``

1. Making an SDcard image (Warning: Assumes /dev/sdb is an SDcard!)

   1. `sudo parted /dev/sdb --script -- mklabel msdos`
   1. `sudo parted /dev/sdb --script -- mkpart primary fat32 1MiB 100%`
   1. `sudo mkfs.vfat -F32 /dev/sdb1`
   1. `sudo fatlabel /dev/sdb1 PETALINUX`

1. Updating SDcard image
   1. `cp sw/linux/images/linux/BOOT.BIN sw/linux/images/linux/boot.scr sw/linux/images/linux/image.ub <SDcard_root_directory>`

1. Testing GPIO first pin
   1. `echo 504 > /sys/class/gpio/export` (Note: Name is name of amba_pl GPIO in /sys/class/gpio/...)
   1. `echo out > /sys/class/gpio/gpio504/direction`
   1. `echo 1 > /sys/class/gpio/gpio504/value`
