from pyharper import ArdUno
import time

au = ArdUno(16)
au.pinMode(13, ArdUno.OUTPUT)
au.pinMode(2, ArdUno.INPUT)

while True:
	ul = au.getULong()
	dr = au.digitalRead(2)
	ar = au.analogRead(0)
	awval = ul%256
	au.analogWrite(3, awval)
	if ul%100>50:	
		au.digitalWrite(13, 1)
	else :
		au.digitalWrite(13, 0)
	print( ul, dr, ar, awval)

