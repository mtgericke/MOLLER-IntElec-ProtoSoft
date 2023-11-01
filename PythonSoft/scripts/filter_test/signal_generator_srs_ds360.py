import pyvisa
from signal_generator import *

class SFSDS360(SignalGenerator):
    name = "srs_ds360"
    desc = "Stanford Research Systems DS360"
    max_frequency = 200000

    def __init__(self, name):
        pass

    def setup(self, resource_addr, amplitude, offset):
        self.amplitude = float(amplitude)
        self.offset = float(offset)

        resources = pyvisa.ResourceManager('@py')
        self.res = resources.open_resource(resource_addr)

        self.res.write("FUNC 0")
        self.res.write("OFFS " + str(self.offset))
        self.res.write("AMPL " + str(self.amplitude) + " VP")
        self.res.write("OUTM 0")
        self.res.write("OUTE 1")
        self.res.write("TERM 0")


    def set_frequency(self, frequency):
        self.frequency = float(frequency)
        self.res.write("FREQ " + str(self.frequency))

