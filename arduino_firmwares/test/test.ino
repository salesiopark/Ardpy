#include <Wire.h>
//#include "/home/pi/eziopy/pyharper/firmware/Harper/Harper.h"
#include <Harper.h>

void test0() {
	int8_t x = Harper.get_int8();
	Harper.set_ret(x+1);
}

void test1() {
	uint8_t x = Harper.get_uint8();
	Harper.set_ret(x+1);
}

void test2() {
	int16_t x = Harper.get_int16();
	Harper.set_ret(x+1);
}

void test3() {
	uint16_t x = Harper.get_uint16();
	Harper.set_ret(x+1);
}

void test4() {
	int32_t x = Harper.get_int32();
	Harper.set_ret(x+1);
}

void test5() {
	uint32_t x = Harper.get_uint32();
	Harper.set_ret(x+1);
}

void test6() {
	float x = Harper.get_float();
	Harper.set_ret(x+1);
}

void test7() {
	char *str = Harper.get_str();
	Serial.println(str);
}

void setup() {
	Harper.add_func(test0);// 0
	Harper.add_func(test1);// 0
	Harper.add_func(test2);// 0
	Harper.add_func(test3);// 0
	Harper.add_func(test4);// 0
	Harper.add_func(test5);// 0
	Harper.add_func(test6);// 0
	Harper.add_func(test7);// 0

	Harper.begin(0x11, 1);//0x04030201);

	Serial.begin(115200);
	Serial.println("ready01");
}


void loop() {
	Harper.check();
}
