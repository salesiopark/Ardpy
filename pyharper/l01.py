from pyharper import TextLcd
lcd = TextLcd(16)
import time
k = 0
while True:#for k in range(1000000):
	print('----------------');print(k)
	#lcd.clear()
	lcd.setCursor(0,0)
	lcd.print( str(k) )
	k+=1
	#time.sleep(0.25)
