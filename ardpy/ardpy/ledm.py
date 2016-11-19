from . import Ardpy
#import time

class Ledm(Ardpy):
    '''
    on(index)
    off(index)
    on_xy(x,y)
    off_xy(x,y)
    clear()
    write(str)
    '''
    _VERSION = '1.0.0'
    
    import struct as __struct
    __table = {
        '0':[0b0111110, 0b1010001, 0b1001001, 0b1000101, 0b0111110],
        '1':[0b1000100, 0b1000010, 0b1111111, 0b1000000, 0b1000000],
        '2':[0b1000010, 0b1100001, 0b1010001, 0b1001001, 0b1000110],
        '3':[0b0100010, 0b1000001, 0b1001001, 0b1001001, 0b0110110],
        '4':[0b0011000, 0b0010100, 0b0010010, 0b1111111, 0b0010000],
        '5':[0b0100111, 0b1000101, 0b1000101, 0b1000101, 0b0111001],
        '6':[0b0111110, 0b1001001, 0b1001001, 0b1001001, 0b0110010],
        '7':[0b0000001, 0b1110001, 0b0001001, 0b0000101, 0b0000011],
        '8':[0b0110110, 0b1001001, 0b1001001, 0b1001001, 0b0110110],
        '9':[0b0100110, 0b1001001, 0b1001001, 0b1001001, 0b0111110],
    }
    

    def __init__(self, addr, port = 1, _showErr = False):
        super().__init__(addr, port = port, _showErr = _showErr)

    def on(self, index):
        self.on_xy(index%5, index//5)

    def off(self, index):
        self.off_xy(index%5, index//5)

    def on_xy(self, x, y):
        self._set_arg(x)
        self._set_arg(y, 1)
        self._exec_func(0)
        #time.sleep(self.__DELAYTM)

    def off_xy(self, x, y):
        self._set_arg(x)
        self._set_arg(y, 1)
        self._exec_func(1)
        #time.sleep(self.__DELAYTM)

    def clear(self):
        self._exec_func(2)

    def _set_all(self, lstx):
        arg0 = self.__struct.unpack('L', bytes(lstx[0:4]) )
        self._set_arg(arg0[0], type='uint32' )
        self._set_arg(lstx[4], index=1)
        self._exec_func(3) # setall()

    def write(self, key):
        if key in self.__table.keys():
            self._set_all(self.__table[key])
        else:
            pass

    def _test(self):
        import time
        while True:
            for k in range(10):
                self.write(str(k))
                time.sleep(0.1)
                
