import eziopy as ep
import time

dev = ep.Device(0x10)

while True:
	dev.exec_func(3)
	print(dev.get_return_data())
	#time.sleep(0.01)
