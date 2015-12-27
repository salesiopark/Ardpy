from pyard import AUno
import time
au = AUno(16)
led = 13
au.pinMode(led, 1)
while True:
	au.digitalWrite(led, 1)
	time.sleep(0.5)
	au.digitalWrite(led, 0)
	time.sleep(0.5)
