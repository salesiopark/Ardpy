from eziopy import ArdUno

au = ArdUno(16)
au.pinMode(13, ArdUno.OUTPUT)

while True:
	print( au.getULong() )	

	
