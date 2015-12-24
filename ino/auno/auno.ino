#include <Wire.h>
#include <Pyard.h>

void func0() {  //pinMode(pin, mode=0(input)/1(output)/2(input_pullup)
	byte pin = Pyard.get_byte();
	byte mode = Pyard.get_byte(1);
	pinMode(pin, mode);
}

void func1() {  //digitalWrite(pin, val)
	byte pin = Pyard.get_byte();
	byte val = Pyard.get_byte(1);
	digitalWrite(pin, val);
}

void func2() { // digitalRead(pin, pullup = false)
	byte pin = Pyard.get_byte();
	byte ret = digitalRead(pin);
	Pyard.set_ret(ret);
}

void func3() { // analogRead()
	byte pin = Pyard.get_byte();
	uint16_t usVal = analogRead(pin);
	Pyard.set_ret(usVal);
}

void func4() { //analogWrite()
	byte pin = Pyard.get_byte();
	byte val = Pyard.get_byte(1);
	analogWrite(pin, val);
}

void setup() {
	Pyard.add_func(func0);
	Pyard.add_func(func1);
	Pyard.add_func(func2);
	Pyard.add_func(func3);
	Pyard.add_func(func4);

	Pyard.begin(0x10, 0);
}

void loop() {
	Pyard.check();
}
