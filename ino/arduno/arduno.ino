/***********************************************************************
* (ardpy)Ledm device firmware
************************************************************************/
#define __VER_AUNO_A 0 // max 255
#define __VER_AUNO_B 8 // max 15
#define __VER_AUNO_C 0 // max 15
//======================================================================
#include <Wire.h>
#include <Ardpy.h>

void set_mode() {  //pinMode(pin, mode=0(input)/1(output)/2(input_pullup)
	byte pin = Ardpy.get_byte();
	byte mode = Ardpy.get_byte(1);
	pinMode(pin, mode);
}
//*/

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

void setup() {
	Ardpy.add_func( set_mode ); //0
	Ardpy.add_func( set_digital ); //1
	Ardpy.add_func( get_digital ); //2
	Ardpy.add_func( get_analog );  //3
	Ardpy.add_func( set_pwm ); //4
	Ardpy.begin(0x10, 0);
}

void loop() {
}

/*
#define sbi(sfr,bit) (_SFR_BYTE(sfr) |=  _BV(bit))
#define cbi(sfr,bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#define bit_is_set(sfr,bit) (_SFR_BYTE(sfr) & _BV(bit)) 
#define bit_is_clear(sfr,bit) (!(_SFR_BYTE(sfr) & _BV(bit)))

volatile uint8_t portM[20] = {
    PORTD, //D0
    PORTD, //D1
    PORTD, //D2
    PORTD, //D3
    PORTD, //D4
    PORTD, //D5
    PORTD, //D6
    PORTD, //D7

    PORTB, //D8
    PORTB, //D9
    PORTB, //D10
    PORTB, //D11
    PORTB, //D12
    PORTB,  //D13

    PORTC, //D14 == A0
    PORTC, //D15 == A1
    PORTC, //D16 == A2
    PORTC, //D17 == A3
    PORTC, //D18 == A4
    PORTC  //D19 == A5
};

volatile uint8_t ddrM[20] = {
    DDRD, //D0
    DDRD, //D1
    DDRD, //D2
    DDRD, //D3
    DDRD, //D4
    DDRD, //D5
    DDRD, //D6
    DDRD, //D7

    DDRB, //D8
    DDRB, //D9
    DDRB, //D10
    DDRB, //D11
    DDRB, //D12
    DDRB,  //D13

    DDRC, //D14 == A0
    DDRC, //D15 == A1
    DDRC, //D16 == A2
    DDRC, //D17 == A3
    DDRC, //D18 == A4
    DDRC  //D19 == A5
};

volatile uint8_t pinM[20] = {
    PIND, //D0
    PIND, //D1
    PIND, //D2
    PIND, //D3
    PIND, //D4
    PIND, //D5
    PIND, //D6
    PIND, //D7

    PINB, //D8
    PINB, //D9
    PINB, //D10
    PINB, //D11
    PINB, //D12
    PINB,  //D13

    PINC, //D14 == A0
    PINC, //D15 == A1
    PINC, //D16 == A2
    PINC, //D17 == A3
    PINC, //D18 == A4
    PINC  //D19 == A5
};


uint8_t bitM[20] = {0,1,2,3,4,5,6,7, 0,1,2,3,4,5, 0,1,2,3,4,5};


void set_digital() {  //digitalWrite(pin, val)
	byte pin = Ardpy.get_byte();
	byte val = Ardpy.get_byte(1);
    digitalWrite(pin, val);

    // set DDR?.pin (to OUTPUT mode)
    // thus, there is no need to call pinMode() before
    sbi( ddrM[pin], bitM[pin] );
    // change PORT?.pin as val
    (val)? sbi(portM[pin], bitM[pin]):cbi(portM[pin], bitM[pin]);

}

void get_digital() { // digitalRead(pin, pullup = false)
	byte pin = Ardpy.get_byte();
    Ardpy.set_ret( (byte) digitalRead(pin) );

    
    // reset DDR?.pin (to INPUT mode)
    // thus, there is no need to call pinMode() before
    cbi( ddrM[pin], bitM[pin] );
    // change PORT?.pin as val
    if ( bit_is_set(pinM[pin], bitM[pin]) ) {
    	Ardpy.set_ret((byte)1);
    } else {
    	Ardpy.set_ret((byte)0);
    }
    
}
/*
void dreadpu() { // digitalRead(pin, pullup = false)
	byte pin = Ardpy.get_byte();

    // reset DDR?.pin (to INPUT mode)
    // thus, there is no need to call pinMode() before
    cbi( ddrM[pin], bitM[pin] );   //set as INPUT mode
    sbi( portM[pin], bitM[pin] ); // connect internal pullup resister

    // read PIN? register
    if ( bit_is_set(pinM[pin], bitM[pin]) ) {
    	Ardpy.set_ret((byte)1);
    } else {
    	Ardpy.set_ret((byte)0);
    }
}
*/

