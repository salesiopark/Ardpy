from . import Ardpy

class Tlcd(Ardpy):
    '''
    Tlcd class for controlling (ardpy)Tlcd device.
    '''
    _VERSION = '0.4.1'

    def __init__(self, addr, port = 1, _showErr=False):
        super().__init__(addr, port = port, _showErr = _showErr)
        self.clear()
        self.show()

    def clear(self):
        return self._exec_func(0)

    def puts(self, text):#putString
        self._set_arg(text, type='str')
        return self._exec_func(1)

    def show(self, ok=True):
        if ok:
            return self._exec_func(2)
        else:
            return self._exec_func(3)

    def setCursor(self, x, y):
        self._set_arg(x, type='byte')
        self._set_arg(y, index=1, type='byte')
        return self._exec_func(4)

    def _test(self):
        cnt = 0;
        while True:
            self.setCursor(0,0)
            self.puts(str(cnt))
            cnt += 1
