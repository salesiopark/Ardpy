from eziopy import ArdUno
import time

au = ArdUno(16)
au.pinMode(13, ArdUno.OUTPUT)
cnt = 0

while True:
	au.digitalWrite(13, 1)
	#time.sleep(0.5)
	au.digitalWrite(13, 0)
	#time.sleep(0.5)
	cnt+=1
	print(cnt)	

	
