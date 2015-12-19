#include <Wire.h>
#include "/home/pi/eziopy/pyharper/firmware/harper.h"
#include <LiquidCrystal.h>

LiquidCrystal lcd(8,9,4,5,6,7);

void clear() {
	lcd.clear();
}

void print() {
	char *str = Harper.get_str(); 
	lcd.print(str);
	lcd.setCursor(0,1);
}

void display() {
	lcd.display();
}

void noDisplay() {
	lcd.noDisplay();
}

void setCursor() {
	byte x = Harper.get_byte();
	byte y = Harper.get_byte(1);

	lcd.setCursor(x,y);
}


void countUp() {
	static uint32_t cnt = 0;
	Harper.set_ret(cnt++);
}

void test1() {
	int8_t x = Harper.get_int8();
	Harper.set_ret(x+1);
}

void test2() {
	int16_t x = Harper.get_int8();
	Harper.set_ret(x+1);
}

void test2() {
	int16_t x = Harper.get_int8();
	Harper.set_ret(x+1);
}

void setup() {
	Harper.add_func(clear);// 0
	Harper.add_func(print);// 1
	Harper.add_func(display);// 2
	Harper.add_func(noDisplay);// 3
	Harper.add_func(setCursor);// 4
	Harper.add_func(countUp);// 5
	Harper.add_func(test);// 6

	Harper.begin(0x11, 1);//0x04030201);
	lcd.begin(16, 2);
	lcd.clear();
	lcd.print("ready41");
}


void loop() {
	Harper.check();
	//static unsigned long k=0;
	//lcd.setCursor(0,0);
	//lcd.print(k++);	

}
