#include <Wire.h>
#include <Ardpy.h>

void ledon(){
    digitalWrite(13, HIGH);
}

void ledoff(){
    digitalWrite(13, LOW);
}

void setup() {
    pinMode(13, OUTPUT);
    Ardpy.add_func(ledon);
    Ardpy.add_func(ledoff);
    Ardpy.begin(0x11, 108);
}

void loop() {

}
