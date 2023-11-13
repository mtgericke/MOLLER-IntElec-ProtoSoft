import pyvisa

class SignalGenerator(object):
    class NoAccess(Exception): pass
    class Unknown(Exception): pass

    max_frequency = 0
    visa_resource = None
    res = None

    @classmethod
    def _get_all_subclasses(cls):
        for subclass in cls.__subclasses__():
            yield subclass
            for subclass in subclass._get_all_subclasses():
                yield subclass

    @classmethod
    def _get_name(cls, s):
        return s.lower()

    def __new__(cls, name):
        name = cls._get_name(name)
        for subclass in cls._get_all_subclasses():
            if subclass.name == name:
                # Using "object" base class method avoids recursion here.
                return object.__new__(subclass)
        else:  # no subclass with matching name found (and no default defined)
            raise SignalGenerator.Unknown('name "{}" has no known type'.format(name))

    def set_remote(self, value):
        if self.res:
            if(value):
                self.res.control_ren(1)
            else:
                self.res.control_ren(0)

    def get_max_frequency(self):
        return self.max_frequency

    def set_frequency(self, frequency):
        raise NotImplementedError

    def setup(self, resource_addr, amplitude, offset):
        raise NotImplementedError

    def get_description(self):
        return self.desc