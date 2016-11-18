from . import Ardpy

class Arduno(Ardpy):
    '''
    Arduno class for controlling Arduno device.
    The Arduno device is composed of Arduino nano and I2C sockets.
    '''
    _VERSION = '0.8.0'
    
    INPUT           = 0
    OUTPUT          = 1
    INPUT_PULLUP    = 2

    HIGH = 1
    LOW = 0

    A0 = 14
    A1 = 15
    A2 = 16
    A3 = 17
    A6 = 20 # actually A6
    A7 = 21 # actually A7

    def __init__(self, addr, port = 1):
        super().__init__(addr, port = port)

    def pinMode(self, pin, mode):
        self._set_arg(pin) # 1st argument
        self._set_arg(mode, index = 1) # 2nd argument
        return self._exec_func(0) # pinMode()

    def digitalWrite(self, pin, val):
        self._set_arg(pin) # 1st argument
        self._set_arg(val, index = 1) # 2nd argument
        return self._exec_func(1) # exec ditigtalWrite() function

    def digitalRead(self, pin):
        self._set_arg(pin) 				# 1st argument
        return self._exec_func(2);

    def analogRead(self, pin):
        self._set_arg(pin) 				# 1st argument
        return self._exec_func(3);

    def analogWrite(self, pin, val): #PWM
        self._set_arg(pin) # 1st argument
        self._set_arg(val, index = 1) # 2nd argument
        return self._exec_func(4) # call awrite() function
