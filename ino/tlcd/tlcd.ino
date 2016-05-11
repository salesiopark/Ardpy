#include <Wire.h>
#include <Ardpy.h>
#include <LiquidCrystal.h>

LiquidCrystal lcd(8,9,4,5,6,7);

void clear() {
	lcd.clear();
}

void print() {
	char *str = Ardpy.get_str(); 
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
	byte x = Ardpy.get_byte();
	byte y = Ardpy.get_byte(1);

	lcd.setCursor(x,y);
}

void setup() {
	Ardpy.add_func(clear);// 0
	Ardpy.add_func(print);// 1
	Ardpy.add_func(display);// 2
	Ardpy.add_func(noDisplay);// 3
	Ardpy.add_func(setCursor);// 4

	Ardpy.begin(0x11, 1);//0x04030201);
	lcd.begin(16, 2);
	lcd.clear();
	lcd.print("ready42");
}


void loop() {
}
