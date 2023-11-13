#!/bin/python

import zmq
import time
import struct
import signal
import argparse

version = "1.0"

"""
CTRL+C Interrupt Handler
"""
class GracefulExiter():

    def __init__(self):
        self.state = False
        signal.signal(signal.SIGINT, self.change_state)

    def change_state(self, signum, frame):
        signal.signal(signal.SIGINT, signal.SIG_DFL)
        self.state = True

    def exit(self):
        return self.state


def write_msg(socket, addr, data):
    msg = struct.pack("<III", ord('w'), int(addr / 4), data)
    socket.send(msg, 0)
    resp = socket.recv()
    msg = struct.unpack_from("<I", resp, 0)
    if(msg[0] == 114):
        msg = struct.unpack_from("<I", resp, 4)
        return msg[0]
    else:
        raise("Write Error")

def read_msg(socket, addr):
    msg = struct.pack("<III", ord('r'), int(addr / 4), 0)
    socket.send(msg, 0)
    resp = socket.recv()
    msg = struct.unpack_from("<I", resp, 0)
    if(msg[0] == 114):
        return struct.unpack_from("<I", resp, 4)[0]
    else:
        raise("Read Error")

def arg_write(args):
    try:
        context = zmq.Context()
        server = "tcp://" + args.ip + ":5555"

        #  Socket to talk to server
        print("Connecting to " + server)
        socket = context.socket(zmq.REQ)
        socket.connect(server)
        resp = write_msg(socket, int(args.addr, 0), int(args.data, 0))
        print(hex(resp))
        print(str(int(resp)))

    except zmq.ZMQError:
        # No message received, keep looping
        pass
    pass

def arg_read(args):

    try:
        context = zmq.Context()
        server = "tcp://" + args.ip + ":5555"

        #  Socket to talk to server
        print("Connecting to " + server)
        socket = context.socket(zmq.REQ)
        socket.connect(server)

        if(args.addr == "revision") or (args.addr == "rev"):
            resp = read_msg(socket, 0x40)
            print("Moller Regmap Revision: " + str(int(resp)))

        elif(args.addr == "clock"):
            for n in range(4):
                resp = read_msg(socket, 0x48 + (n*4))
                if n == 0:
                    print("TI: " + str(int(resp) / 1000000 ) + " MHz")
                if n == 1:
                    print("Osc: " + str(int(resp) / 1000000 ) + " MHz")
                if n == 2:
                    print("SOM0: " + str(int(resp) / 1000000 ) + " MHz")
                if n == 3:
                    print("SOM1: " + str(int(resp) / 1000000 ) + " MHz")

        elif(args.addr == "adc"):
            print("ADC\tData")
            for n in range(16):
                data = read_msg(socket, 0 + (n*4))
                print(str(n+1) +"\t" + format(int(data) & 0x3FFFF, '018b') + " [" + format(int(data) & 0x3FFFF, '05x') + "]")

            phase_hi = read_msg(socket, int(0x60))
            phase_lo = read_msg(socket, int(0x64))

            print("Phase: " + format(int(phase_hi), '024b') + format(int(phase_lo), '032b'))

            # Read phase registers and generate phase.txt for display
            phase = []
            phase_data_file = open('phase.txt', 'w')
            for n in range(16*56):
                resp = read_msg(socket, 0x20000 + (n*4))
                if(n % 16 == 15):
                    phase_data_file.write(str(int(resp)) + "\n")
                    # print(str(int(resp)), end='\n')
                else:
                    phase_data_file.write(str(int(resp)) + ",")
                    # print(str(int(resp)), end='\t')


            phase_data_file.close()
            print("Wrote phase results to phase.txt")

        else:
            msg = struct.pack("<III", ord('r'), int(int(args.addr, 0) / 4), 0)
            socket.send(msg, 0)
            resp = socket.recv()
            msg = struct.unpack_from("<I", resp, 0)
            if(msg[0] == 114):
                msg = struct.unpack_from("<I", resp, 4)
                print('0x{0:08X}'.format(int(msg[0])) + "[" + str(int(msg[0])) + "]")
                print()
            else:
                print("Read Error")

    except zmq.ZMQError:
        # No message received, keep looping
        pass

def main():
    prog='moller_ctrl'
    parser = argparse.ArgumentParser(prog=prog)
    parser.add_argument('--version', action='version', version='%(prog)s ' + version)
    parser.add_argument("ip", help="IP address of Moller Integrating ADC")

    cmd_parser = parser.add_subparsers(dest="command")

    read_parser = cmd_parser.add_parser("read")
    read_parser.add_argument("addr")
    read_parser.set_defaults(func=arg_read)

    write_parser = cmd_parser.add_parser("write")
    write_parser.set_defaults(func=arg_write)
    write_parser.add_argument("addr")
    write_parser.add_argument("data")

    args = parser.parse_args()

    flag = GracefulExiter()
    print(args)

    args.func(args)

if __name__ == "__main__":
    main()