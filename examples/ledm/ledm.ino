/***********************************************************************
* ardpy.Ledm device firmware
************************************************************************/
#include <Ardpy.h>
#define VER __VER__(1,0,3) // Ardpy.check()함수의 세 번째 인자
//---------------------------------------------------------------------
#define X0 A0
#define X1 A1
#define X2 A2
#define X3 A3
#define X4 12

#define Y0 4
#define Y1 5
#define Y2 6
#define Y3 7
#define Y4 8
#define Y5 9
#define Y6 10

volatile byte mat[7][5] = {
    {LOW,   HIGH,   HIGH,   HIGH,   LOW},
    {HIGH,  LOW,    LOW,    LOW,    HIGH},
    {HIGH,  LOW,    LOW,    LOW,    HIGH},
    {HIGH,  LOW,    LOW,    LOW,    HIGH},
    {HIGH,  LOW,    LOW,    LOW,    HIGH},
    {HIGH,  LOW,    LOW,    LOW,    HIGH},
    {LOW,   HIGH,   HIGH,   HIGH,   LOW},
};

const byte _XS[5] = {X0, X1, X2, X3, X4};
const byte _YS[7] = {Y0, Y1, Y2, Y3, Y4, Y5, Y6};

void ledon() {  //pinMode(pin, mode=0(input)/1(output)/2(input_pullup)
    byte x = Ardpy.get_byte();
    byte y = Ardpy.get_byte(1);
	mat[y][x] = HIGH;
}

void ledoff() {  //pinMode(pin, mode=0(input)/1(output)/2(input_pullup)
    byte x = Ardpy.get_byte();
    byte y = Ardpy.get_byte(1);
	mat[y][x] = LOW;
}

void clear() {
    for(byte y=0;y<7;y++)
        for(byte x=0;x<5; x++)
            mat[y][x] = LOW;
}

/* 24/Nov/2016 
 dat:   |3|2|1|0|
 ax : |4|3|2|1|0|
 즉, 하위 4바이트는 서로 공유하고 ax에는 extra byte가 하나 더 있는 것이다.
*/
union UDat {
    uint32_t    dat;
    byte        ax[5]; // 이 크기가 더 크다
} udat;

void setall(){
    udat.dat = Ardpy.get_uint32();
    udat.ax[4] = Ardpy.get_byte(1);

    for(byte y=0;y<7;y++)
        for(byte x=0;x<5;x++)
            mat[y][x]= (udat.ax[x]>>y) & 1 ;
}

void setup() {
    pinMode(Y0, OUTPUT);
    pinMode(Y1, OUTPUT);
    pinMode(Y2, OUTPUT);
    pinMode(Y3, OUTPUT);
    pinMode(Y4, OUTPUT);
    pinMode(Y5, OUTPUT);
    pinMode(Y6, OUTPUT);

    pinMode(X0, OUTPUT);
    pinMode(X1, OUTPUT);
    pinMode(X2, OUTPUT);
    pinMode(X3, OUTPUT);
    pinMode(X4, OUTPUT);
////////////////////////////////////////////////
    pinMode(13, OUTPUT);

	Ardpy.add_func(ledon);// _exec_func(0)
	Ardpy.add_func(ledoff);// _exec_func(1)
    Ardpy.add_func(clear);// _exec_func(2)
    Ardpy.add_func(setall);// _exec_func(3)

	Ardpy.begin(0x11, 101, VER);
}

byte ix   = 0; // index of x
byte ix_1 = 4 ;// ix[k-1]
void loop() { 
    Ardpy.check(); // 
    
    digitalWrite(_XS[ix_1], HIGH); // turn off prev column
    for(int iy=0;iy<7;iy++) { // set current column dots
        digitalWrite(_YS[iy], mat[iy][ix]);
    }
    digitalWrite(_XS[ix], LOW); // turn on cur column
    ix_1 = ix;
    if (++ix == 5) ix = 0;
    
    /*
    switch(col) {
        case 0:
            digitalWrite(X4, HIGH);
            digitalWrite(Y0, mat[0][0]);
            digitalWrite(Y1, mat[1][0]);
            digitalWrite(Y2, mat[2][0]);
            digitalWrite(Y3, mat[3][0]);
            digitalWrite(Y4, mat[4][0]);
            digitalWrite(Y5, mat[5][0]);
            digitalWrite(Y6, mat[6][0]);
            digitalWrite(X0, LOW);
            col = 1;
            break;

        case 1:
            digitalWrite(X0, HIGH);
            digitalWrite(Y0, mat[0][1]);
            digitalWrite(Y1, mat[1][1]);
            digitalWrite(Y2, mat[2][1]);
            digitalWrite(Y3, mat[3][1]);
            digitalWrite(Y4, mat[4][1]);
            digitalWrite(Y5, mat[5][1]);
            digitalWrite(Y6, mat[6][1]);
            digitalWrite(X1, LOW);
            col = 2;
            break;
            
        case 2:
            digitalWrite(X1, HIGH);
            digitalWrite(Y0, mat[0][2]);
            digitalWrite(Y1, mat[1][2]);
            digitalWrite(Y2, mat[2][2]);
            digitalWrite(Y3, mat[3][2]);
            digitalWrite(Y4, mat[4][2]);
            digitalWrite(Y5, mat[5][2]);
            digitalWrite(Y6, mat[6][2]);
            digitalWrite(X2, LOW);
            col = 3;
            break;

        case 3:
            digitalWrite(X2, HIGH);
            digitalWrite(Y0, mat[0][3]);
            digitalWrite(Y1, mat[1][3]);
            digitalWrite(Y2, mat[2][3]);
            digitalWrite(Y3, mat[3][3]);
            digitalWrite(Y4, mat[4][3]);
            digitalWrite(Y5, mat[5][3]);
            digitalWrite(Y6, mat[6][3]);
            digitalWrite(X3, LOW);
            col = 4;
            break;

        case 4:
            digitalWrite(X3, HIGH);
            digitalWrite(Y0, mat[0][4]);
            digitalWrite(Y1, mat[1][4]);
            digitalWrite(Y2, mat[2][4]);
            digitalWrite(Y3, mat[3][4]);
            digitalWrite(Y4, mat[4][4]);
            digitalWrite(Y5, mat[5][4]);
            digitalWrite(Y6, mat[6][4]);
            digitalWrite(X4, LOW);
            col = 0;
            break;
    }
    //*/
}




