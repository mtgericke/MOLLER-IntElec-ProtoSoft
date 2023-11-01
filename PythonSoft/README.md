# MOLLER Digitizer

## Installation

To install/upgrade:
`pip install -U moller`

## Command Line Tool

The installation includes a command line tool `moller-ctl` that can be used to perform simple readback and control of the device for diagnostic purposes.

To get additional help with the tool you can use `moller-ctl -h` or `moller-ctl <command> -h`

### Available commands

- `moller-ctl discover` - Discover local subnet digitizers
- `moller-ctl <ip> status` - Detailed status of digitizer
- `moller-ctl <ip> align` - Redoes ADC alignment and displays plot
- `moller-ctl <ip> plot [all]` or `moller-ctl [ip] plot [1-16]` or `moller-ctl [ip] plot [1-16] [1-16]` - Plots channel data in real-time
- `moller-ctl <ip> data <ch1> [ch2]` - Gets channel data
- `moller-ctl <ip> read <address>` - Read from register at address
- `moller-ctl <ip> write <address> <data>` - Write to register at address

