from pyharper import ArdUno

au = ArdUno(16)
au.pinMode(3, ArdUno.OUTPUT)

while True:
	for val in range(0,256):
		au.analogWrite(3, val)
		print(val)
	for val in range(254,0,-1):
		au.analogWrite(3, val)
		print(val)

	
