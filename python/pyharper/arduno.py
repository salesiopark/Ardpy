# eziopy.arduino
from . import Harper

class ArdUno(Harper):
	INPUT = 0
	OUTPUT = 1
	INPUT_PULLUP = 2

	A0 = 14
	A1 = 15
	A2 = 16
	A3 = 17

	def __init__(self, addr, port = 1):
		super().__init__(addr, port = port)

	def pinMode(self, pin, mode):
		self._send_byte(pin) # 1st argument
		self._send_byte(mode, index = 1) # 2nd argument
		return self._exec_func(0) # exec ditigtalWrite() function

	def digitalWrite(self, pin, val):
		self._send_byte(pin) # 1st argument
		self._send_byte(val, index = 1) # 2nd argument
		return self._exec_func(1) # exec ditigtalWrite() function

	def digitalRead(self, pin):
		self._send_byte(pin) 				# 1st argument
		return self._exec_func(2);

	def analogRead(self, pin):
		#aPin = pin + 14
		self._send_byte(pin) 				# 1st argument
		return self._exec_func(3);

	def analogWrite(self, pin, val): #PWM
		self._send_byte(pin) # 1st argument
		self._send_byte(val, index = 1) # 2nd argument
		return self._exec_func(4) # exec ditigtalWrite() function
