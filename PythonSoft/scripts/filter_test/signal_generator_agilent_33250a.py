import pyvisa
from signal_generator import *

class Agilent33250A(SignalGenerator):
    name = "agilent_33250a"
    desc = "Agilent 33250A"
    max_frequency = 80000000

    def __init__(self, name):
        pass

    def setup(self, resource_addr, amplitude, offset):
        self.amplitude = float(amplitude)
        self.offset = float(offset)

        resources = pyvisa.ResourceManager('@py')
        self.res = resources.open_resource(resource_addr)

        self.res.write("APPL:SIN 1 HZ, " + str(amplitude*2) + " VPP, " + str(offset) + " V")
        self.res.write("OUTPut:LOAD 50")

    def set_frequency(self, frequency):
        self.frequency = float(frequency)
        self.res.write("FREQ " + str(self.frequency))




