from pyharper import TextLcd
lcd = TextLcd(16)
import time

lcd.print('warning !!!')

while True:
	lcd.noDisplay()
	time.sleep(0.5)
	lcd.display()
	time.sleep(0.5)

