#include <Wire.h>
#include <Harper.h>

void func0() {  //pinMode(pin, mode=0(input)/1(output)/2(input_pullup)
	byte pin = Harper.get_byte();
	byte mode = Harper.get_byte(1);
	pinMode(pin, mode);
}

void func1() {  //digitalWrite(pin, val)
	byte pin = Harper.get_byte();
	byte val = Harper.get_byte(1);
	digitalWrite(pin, val);
}

void func2() { // digitalRead(pin, pullup = false)
	byte pin = Harper.get_byte();
	byte ret = digitalRead(pin);
	Harper.set_ret(ret);
}

void func3() { // analogRead()
	byte pin = Harper.get_byte();
	uint16_t usVal = analogRead(pin);
	Harper.set_ret(usVal);
}

void func4() { //analogWrite()
	byte pin = Harper.get_byte();
	byte val = Harper.get_byte(1);
	analogWrite(pin, val);
}


void setup() {
///*
	Harper.add_func(func0);
	Harper.add_func(func1);
	Harper.add_func(func2);
	Harper.add_func(func3);
	Harper.add_func(func4);
//*/

	byte addr = Harper.begin(0x10, 300);
}

void loop() {
	Harper.check();
}
