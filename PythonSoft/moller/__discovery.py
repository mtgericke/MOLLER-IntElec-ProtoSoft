#!/bin/python

import logging
import socket
import threading
import queue
import time
import struct

VERSION = "1.0"  # Program version
BEACON_PORT = 4200  # Shouldnt' be changed
BEACON_PROTOCOL_VERSION = "BEACON01".encode("UTF-8")  # Protocol version string
DEFAULT_BEACON_MAX_QUEUE_SIZE = 256

logger = logging.getLogger(__name__)

class DuplicateFilter(logging.Filter):
    def filter(self, record):
        # add other fields if you need more granular comparison, depends on your app
        current_log = (record.module, record.levelno, record.msg)
        if current_log != getattr(self, "last_log", None):
            self.last_log = current_log
            return True
        return False


def convert_id_to_hex(deviceID):
    s = ""
    for n in range(0, 8):
        s = s + format(deviceID & 0xFF, "02X")
        deviceID = deviceID >> 8
        if n < 7:
            s += ":"
    return s


class DeviceID:
    def __init__(self, message=None):
        (
            self.version,
            self.uptime,
            self.deviceID,
            self.logicalID,
            self.hwRev,
            self.fwRev,
            self.vendorName,
            self.productName,
        ) = struct.unpack("<8sQQQ32s32s64s96s", message)
        if (self.version != BEACON_PROTOCOL_VERSION) and (
            self.version[0:6].decode() != "RQUEST"
        ):
            raise ValueError(
                "Version mismatch! Expected "
                + BEACON_PROTOCOL_VERSION.decode()
                + ". Got "
                + self.version.decode()
            )

    def generateMsg(self, uptime):
        return struct.pack(
            "<8sQQQ32s32s64s96s",
            BEACON_PROTOCOL_VERSION,
            uptime,
            self.deviceID,
            self.logicalID,
            self.hwRev,
            self.fwRev,
            self.vendorName.encode("UTF-8"),
            self.productName.encode("UTF-8"),
        )


def beacon_thread(evt, config, found_queue):
    server = socket.socket(family=socket.AF_INET, type=socket.SOCK_DGRAM)
    try:
        server.bind((config.ip_addr, config.ip_port))
    except OSError as e:
        logger.error("Unable to bind to " + config.ip_addr + ":" + str(config.ip_port))
        return

    server.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    server.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, 1)
    server.setblocking(0)

    logger.info(
        "Discovery server (" + config.ip_addr + ":" + str(config.ip_port) + ") started"
    )

    # Send DISCOVERY request that matches all devices
    request = struct.pack(
        "<8sQQQ32s32s64s96s",
        "RQUEST01".encode("UTF-8"),
        0,
        0xFFFFFFFFFFFFFFFF,
        0xFFFFFFFFFFFFFFFF,
        "".encode("UTF-8"),
        "".encode("UTF-8"),
        "".encode("UTF-8"),
        "".encode("UTF-8"),
    )

    logger.info("Sending request")
    server.sendto((request), ("255.255.255.255", BEACON_PORT))

    while not evt.isSet():
        # Gather up client discovery requests
        try:
            data, addr = server.recvfrom(1500)
        except BlockingIOError as e:
            pass
        else:
            try:
                msg = DeviceID(data)
            except struct.error as e:
                # Failed to unpack the message, bad beacon packet
                logger.debug(
                    "Invalid beacon packet received from "
                    + str(addr[0])
                    + ":"
                    + str(addr[1])
                )
            except ValueError as e:
                # This gets raised on a version mismatch, DeviceID object gives detailed message so print that
                logger.debug(e)
            else:
                # Silently ignore RQUEST beacons
                if msg.version[0:6].decode() == "RQUEST":
                    pass
                else:
                    # Did we find an appropriate response?
                    response_str = (
                        "Beacon received from "
                        + str(addr[0])
                        + " [Vendor: '"
                        + msg.vendorName.decode()
                        + "', Product: '"
                        + msg.productName.decode()
                        + "', deviceID: "
                        + convert_id_to_hex(msg.deviceID)
                        + ", logicalID: "
                        + str(msg.logicalID)
                        + ", hwRev: "
                        + msg.hwRev.decode()
                        + ", fwRev: "
                        + msg.fwRev.decode()
                        + ", Uptime: "
                        + str(msg.uptime)
                        + "]"
                    )
                    print(response_str)
                    # logger.debug(response_str)
                    try:
                        found_queue.put_nowait((time.time(), addr, msg))
                    except queue.Full:
                        logger.warning(
                            "Beacon queue is full [" + str(found_queue.maxsize) + "]"
                        )
                        pass

    logger.info("Discovery server stopped")


class DiscoveryConfig:
    def __init__(
        self,
        ip_addr="0.0.0.0",
        ip_port=BEACON_PORT,
        max_queue_size=DEFAULT_BEACON_MAX_QUEUE_SIZE,
    ):
        self.ip_addr = ip_addr
        self.ip_port = ip_port
        self.max_queue_size = max_queue_size

    def __str__(self):
        return (
            "ip_addr: "
            + str(self.ip_addr)
            + "\nip_port: "
            + str(self.ip_port)
            + "\nmax_queue_size: "
            + str(self.max_queue_size)
        )


class DiscoveryController:
    def __init__(self, config=DiscoveryConfig()):
        self.config = config
        self.evt = threading.Event()
        self.found_queue = queue.Queue(maxsize=config.max_queue_size)
        self.found_list = []  # Devices that have replied to the beacon
        self.thread = None

    def __del__(self):
        self.stop()

    def start(self):
        self.thread = threading.Thread(
            target=beacon_thread,
            args=(
                self.evt,
                self.config,
                self.found_queue,
            ),
            daemon=True,
        )
        self.thread.start()

    def is_alive(self):
        return self.thread.is_alive()

    def get_found(self):
        while not self.found_queue.empty():
            timestamp, addr, msg = self.found_queue.get_nowait()
            self.found_queue.task_done()
            logger.info(
                "Beacon received at "
                + str(timestamp)
                + " from "
                + str(addr[0])
                + " [Vendor: '"
                + msg.vendorName.decode()
                + "', Product: '"
                + msg.productName.decode()
                + "', deviceID: "
                + hex(msg.deviceID)
                + ", logicalID: "
                + hex(msg.logicalID)
                + ", hwRev: "
                + hex(msg.hwRev)
                + ", fwRev: "
                + hex(msg.fwRev)
                + ", Uptime: "
                + str(msg.uptime)
                + "]"
            )
        return self.found_list

    def stop(self):
        if self.thread.is_alive():
            self.evt.set()
            self.thread.join(5)
            self.evt.clear()
