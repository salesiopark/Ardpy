from . import Dev

class TextLcd(Dev):

	def __init__(self, addr, port = 1):
		super().__init__(addr, port = port)
		self.clear()
		self.display()

	def clear(self):
		return self._exec_func(0)

	def print(self, text):
		self._send_str(text) # 1st argument
		return self._exec_func(1) # exec ditigtalWrite() function

	def display(self):
		return self._exec_func(2)

	def noDisplay(self):
		return self._exec_func(3)

	def setCursor(self, x, y):
		self._send_byte(x)
		self._send_byte(y, 1)
		return self._exec_func(4)

	def countUp(self):
		return self._exec_func(5)
