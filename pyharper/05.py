from eziopy import ArdUno

au = ArdUno(16)
au.pinMode(13, ArdUno.OUTPUT)

while True:
	btn = au.digitalRead(2)
	print(btn)	

	
