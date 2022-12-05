#!/bin/python

from numpy import array
import zmq
import time
import struct
import signal
import argparse
import datetime

version = "1.0"

TS_CONVERSION = 17  # Number of cycles to convert (68ns  / 4ns) at 250Mhz

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

def main():
    prog='moller_dump_samples'
    parser = argparse.ArgumentParser(prog=prog)
    parser.add_argument('--version', action='version', version='%(prog)s ' + version)
    parser.add_argument("ip", help="IP address of Moller ADC")
    parser.add_argument("file_append", nargs="?", default="sample_data.raw", help="File to write to")
    args = parser.parse_args()

    flag = GracefulExiter()
    context = zmq.Context()
    server = "tcp://" + args.ip + ":5556"
    bit_count = 0
    delta = 0
    msg_count = 0
    sample_count = 0
    dropped_pkts = 0
    missed_ts = 0
    RATE = 4 # Rate in seconds

    buffer_depth = 65536 * 1024

    #  Socket to talk to server
    print("Connecting to " + server)
    socket = context.socket(zmq.SUB)
    socket.connect(server)
    socket.setsockopt_string( zmq.SUBSCRIBE, "")
    # With these set, we can capture for quite awhile even when python isn't keeping up...
    socket.set_hwm(buffer_depth)
    socket.setsockopt(zmq.RCVBUF, buffer_depth)

    start_time = time.time()

    last_ts = 0
    last_pkt = 0
    sample_file = open(args.file_append, 'wb')

    integration_file = open("int_data.txt", 'wt')
    integration_file.write("Samples,Packets,Timestamp,Squares,Sum,Count\n")

    statistic_file = open("stat_data.txt", 'wt')
    statistic_file.write("time_elapsed,num_pkts,num_bytes,num_errors,num_valid,num_timeouts,num_samples,temp1,temp2,temp3,ps_temp,remote_temp,pl_temp,vcc_pspll0,vcc_psbatt,vccint2,vccbram3,vccaux4,vcc_psddrpll,vccpsintfpddr,vccpsintlp,vccpsintfp,vccpsaux,vccpsddr,vccpsio3,vccpsio0,vccpsio1,vccpsio2,psmgtravcc,psmgtravtt,vccams17,vccint18,vccaux19,vccvrefp,vccvrefn,vccbram22,vccplintlp,vccplintfp,vccplaux,vccams26\n")
    while(1):
        try:
            msg = socket.recv_multipart(flags=zmq.NOBLOCK)

            msg_id = msg[0]
            message = msg[1]

            if(msg_id == b"AVG"):
                ch_square = []
                ch_sum = []
                ch_sample_count = []
                for n in range(16):
                    square, = struct.unpack_from("<Q", message, 32 + ((n+32)*8))
                    sum, = struct.unpack_from("<Q", message, 32 + ((n+16)*8))
                    sample_count, = struct.unpack_from("<Q", message, 32 + (n*8))

                    ch_square.append(square)
                    ch_sum.append(sum)
                    ch_sample_count.append(sample_count)

                header, ts, pkt_counter, samples_captured  = struct.unpack_from("<QQQQ", message, 0)
                integration_file.write(str(samples_captured) + "," + str(pkt_counter) + "," + str(ts) + "," + str(ch_square) + "," + str(ch_sum) + "," + str(ch_sample_count) + "\n")

                # print("Samples captured: " + str(samples_captured))
                # print("Pkts: " + str(pkt_counter))
                # print("Timestamp: " + str(ts))
                # print("Squares: " + str(ch_square))
                # print("Sum: " + str(ch_sum))
                # print("sample count: " + str(ch_sample_count))

            if(msg_id == b"STA"):
                time_elapsed, num_pkts, num_bytes, num_errors, num_valid, num_timeouts, num_samples, temp1, temp2, temp3, \
                ps_temp, remote_temp, pl_temp, vcc_pspll0, vcc_psbatt, vccint2, vccbram3, vccaux4, vcc_psddrpll, vccpsintfpddr, \
                vccpsintlp, vccpsintfp, vccpsaux, vccpsddr, vccpsio3, vccpsio0, vccpsio1, vccpsio2, psmgtravcc, psmgtravtt, \
                vccams17, vccint18, vccaux19, vccvrefp, vccvrefn, vccbram22, vccplintlp, vccplintfp, vccplaux, vccams26 \
                = struct.unpack_from("<QQQQQQQdddddd27d", message, 0)

                statistic_file.write(str(time_elapsed) + "," + str(num_pkts) + "," + str(num_bytes) + "," + str(num_errors) + "," + str(num_valid) + "," + str(num_timeouts) + "," + str(num_samples) + "," + str(temp1) + "," + str(temp2) + "," + str(temp3) +
                str(ps_temp) + "," + str(remote_temp) + "," + str(pl_temp) + "," + str(vcc_pspll0) + "," + str(vcc_psbatt) + "," + str(vccint2) + "," + str(vccbram3) + "," + str(vccaux4) + "," + str(vcc_psddrpll) + "," + str(vccpsintfpddr) +
                str(vccpsintlp) + "," + str(vccpsintfp) + "," + str(vccpsaux) + "," + str(vccpsddr) + "," + str(vccpsio3) + "," + str(vccpsio0) + "," + str(vccpsio1) + "," + str(vccpsio2) + "," + str(psmgtravcc) + "," + str(psmgtravtt) +
                str(vccams17) + "," + str(vccint18) + "," + str(vccaux19) + "," + str(vccvrefp) + "," + str(vccvrefn) + "," + str(vccbram22) + "," + str(vccplintlp) + "," + str(vccplintfp) + "," + str(vccplaux) + "," + str(vccams26) + "\n")

                # print("Packet Info")
                # print("Pkt rate: " + f"{(num_pkts / time_elapsed * 1000000):.3f}" + " pps")
                # print("Data rate: " + f"{(num_bytes / time_elapsed):.3f}" + " mbps")
                # print("\nDMA")
                # print("Num Pkts: " + str(num_pkts))
                # print("Num Bytes: " + str(num_bytes))
                # print("Num Errors: " + str(num_errors))
                # print("Num Valid:  " + str(num_valid))
                # print("Num Timeouts: " + str(num_timeouts))
                # print("\nVoltages and Temperature")
                # print("Enclustra: " + f"{temp1:.3f}" + " C")
                # print("Power Supply: " + f"{temp2:.3f}" + " C")
                # print("Board: " + f"{temp3:.3f}" + " C")
                # print("ps_temp: " + f"{ps_temp:.3f}" + " C")
                # print("pl_temp: " + f"{pl_temp:.3f}" + " C")
                # print("remote_temp: " + f"{remote_temp:.3f}" + " C")
                # print("vcc_pspll0: " + f"{vcc_pspll0:.3f}" + " V")
                # print("vcc_psbatt: " + f"{vcc_psbatt:.3f}" + " V")
                # print("vccint(2): " + f"{vccint2:.3f}" + " V")
                # print("vccbram(3): " + f"{vccbram3:.3f}" + " V")
                # print("vccaux(4): " + f"{vccaux4:.3f}" + " V")
                # print("vcc_psddrpll: " + f"{vcc_psddrpll:.3f}" + " V")
                # print("vccpsintfpddr: " + f"{vccpsintfpddr:.3f}" + " V")
                # print("vccpsintlp: " + f"{vccpsintlp:.3f}" + " V")
                # print("vccpsintfp: " + f"{vccpsintfp:.3f}" + " V")
                # print("vccpsaux: " + f"{vccpsaux:.3f}" + " V")
                # print("vccpsddr: " + f"{vccpsddr:.3f}" + " V")
                # print("vccpsio3: " + f"{vccpsio3:.3f}" + " V")
                # print("vccpsio0: " + f"{vccpsio0:.3f}" + " V")
                # print("vccpsio1: " + f"{vccpsio1:.3f}" + " V")
                # print("vccpsio2: " + f"{vccpsio2:.3f}" + " V")
                # print("psmgtravcc: " + f"{psmgtravcc:.3f}" + " V")
                # print("psmgtravtt: " + f"{psmgtravtt:.3f}" + " V")
                # print("vccams(17): " + f"{vccams17:.3f}" + " V")
                # print("vccint(18): " + f"{vccint18:.3f}" + " V")
                # print("vccaux(19): " + f"{vccaux19:.3f}" + " V")
                # print("vccvrefp: " + f"{vccvrefp:.3f}" + " V")
                # print("vccvrefn: " + f"{vccvrefn:.3f}" + " V")
                # print("vccbram(22): " + f"{vccbram22:.3f}" + " V")
                # print("vccplintlp: " + f"{vccplintlp:.3f}" + " V")
                # print("vccplintfp: " + f"{vccplintfp:.3f}" + " V")
                # print("vccplaux: " + f"{vccplaux:.3f}" + " V")
                # print("vccams(26): " + f"{vccams26:.3f}" + " V")
                # print()

            if(msg_id == b"ADC"):
                # Increment message count
                bit_count = bit_count + (len(message) * 8)

                # Unpack message
                num_words, num_pkt, id, ts = struct.unpack_from("<HIxBQ", message, 0)
                num_samples = num_words - 1

                if(num_pkt != last_pkt + 1) & (last_pkt != 0):
                    dropped_pkts = dropped_pkts + 1
                    print("Streaming Packet Dropped")

                last_pkt = num_pkt

                # for n in range(num_samples):
                n = 0
                ch0, ch1 = struct.unpack_from("<ii", message, 16 + (n * 8))
                ch1_data = ch1 >> 14
                ch0_data = ch0 >> 14

                ch1_sel = ch1 & 0xF
                ch0_sel = ch0 & 0xF

                stream_div = ((ch0 >> 4) & 0x7F) + 1
                factor = 1

                if(last_ts != 0):
                    if(ch0_sel == ch1_sel):
                        factor = 2

                    if((ts - last_ts) != (num_samples * factor * TS_CONVERSION * stream_div)):
                        missed_ts = missed_ts + 1

                last_ts = ts

                sample_file.write(message)

                # Every 'sample' is two channels of data
                sample_count = sample_count + (num_samples * factor)
                msg_count = msg_count + 1

        except zmq.ZMQError:
            # No message received, keep looping
            pass

        delta = time.time() - start_time
        if (delta >= RATE):
            start_time = time.time()
            print("\x1b[2KRate " + f"{(bit_count / delta / 1000000):.3f}" + " mbit/s (" + f"{(bit_count / 8 / delta / 1000000):.3f}" + ") [" + f"{int((msg_count) / delta):d}" + " packets/s + " + f"{(int(sample_count) / delta / 1000000):.3f}" + " mSamples/s dropped: " + f"{(dropped_pkts / delta):.3f}" + " missed: " + f"{(missed_ts / delta):.3f}" + "]", end='\r')
            bit_count = 0
            msg_count = 0
            sample_count = 0
            dropped_pkts = 0
            missed_ts = 0

        if flag.exit():
            break

if __name__ == "__main__":
    main()