import eziopy as ep
import time

dev = ep.Device(0x10)
dev.send_uchar(13)

while True:
	dev.send_uchar(1, index = 1)
	dev.exec_func(0)
	time.sleep(0.5)
	dev.send_uchar(0, index = 1)
	dev.exec_func(0)
	time.sleep(0.5)
