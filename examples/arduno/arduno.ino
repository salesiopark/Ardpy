/***********************************************************************
* (ardpy)Ledm device firmware
************************************************************************/
#include <Ardpy.h>
#define VER __VER__(1,0,0) // Ardpy.check()함수의 세 번째 인자

void set_mode() {  //pinMode(pin, mode=0(input)/1(output)/2(input_pullup)
	byte pin = Ardpy.get_byte();
	byte mode = Ardpy.get_byte(1);
	pinMode(pin, mode);
}

void set_digital() {  //digitalWrite(pin, val)
	byte pin = Ardpy.get_byte();
	byte val = Ardpy.get_byte(1);
    digitalWrite(pin, val);
}

void get_digital() { // digitalRead(pin, pullup = false)
	byte pin = Ardpy.get_byte();
    Ardpy.set_ret( (byte) digitalRead(pin) );
}

void get_analog() { // analogRead()
	byte pin = Ardpy.get_byte();
	Ardpy.set_ret( (uint16_t)analogRead(pin) );
}

void set_pwm() { //analogWrite()
	byte pin = Ardpy.get_byte();
	byte val = Ardpy.get_byte(1);
	analogWrite(pin, val);
}

/*
void get_int0() { // int0 = d2
    Ardpy.set_ret( (byte)digitalRead(2) );
}

void get_int1() { // int1 = d3
    Ardpy.set_ret( (byte)digitalRead(3) );
}
*/

void setup() {
	Ardpy.add_func( set_mode ); //0
	Ardpy.add_func( set_digital ); //1
	Ardpy.add_func( get_digital ); //2
	Ardpy.add_func( get_analog );  //3
	Ardpy.add_func( set_pwm ); //4

//	Ardpy.add_func( get_int0 ); //5
//	Ardpy.add_func( get_int1 ); //6

	Ardpy.begin(0x10, 0, VER);
}

void loop() {
    Ardpy.check();
}
