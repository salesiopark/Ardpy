/***********************************************************************
 Harper.h ver 1.0
- This is an arduino library for i2c communication with Raspberry pi.
- developed by : Jang-Hyun Park (SalesioPark)
************************************************************************/

#include <Wire.h>
#include <EEPROM.h>

typedef signed char sbyte;
typedef unsigned char byte;
typedef unsigned short ushort;
typedef unsigned long ulong;

#define CMD_READ				0
#define CMD_CHANGE_ADDR		1
#define CMD_IS_READY			2
#define CMD_SEND_DATA          	3
#define CMD_EXEC_FUNC          	5
//#define CMD_CHECK_RET_DATA	6

// internally used constants ===================================
#define __CHANGE_ADDR_CONFIRM__		254
#define __EXEC_FUNC_CONFIRM__  			250
#define __SEND_DATA_CONFIRM__				252
#define __CHECK_RET_DATA_CONFIRM__	249

#define __STR_BUF_LENGTH__			64
#define __RET_DATA_LENGTH__		9
#define __MAX_I2C_DATA_LENGTH__	31
#define __EEPROM_ADDR__			1
#define __MAX_ARG_NUM__ 			4
#define __MAX_FUNC_NUM__ 			16

#define __MIN_I2C_ADDR__			3
#define __MAX_I2C_ADDR__			119

#define __STAT_NOT_READY__			0
#define __STAT_READY__				1
#define __STAT_ERR_DATA__			2 // wrong data sent
#define __STAT_ERR_FUNC_INDEX__	3 // function index out of bound
#define __STAT_ERR_ARG_INDEX__		4 // argument index out of bound

// internally used global variables --------------------------------------------------------
union __U_Short__	{  short val;  byte byArr[2]; };
union __U_UShort__	{  ushort val;  byte byArr[2]; };
union __U_Long__	{  long val;  byte byArr[4]; };
union __U_ULong__	{ ulong val;  byte byArr[4]; };
union __U_Float__	{  float val; byte byArr[4]; };
union __U_Data__		{
	sbyte			sbVal;
	byte			ucVal;
	//bool			bVal;
	__U_Short__		u_s;
	__U_UShort__	u_us;
	__U_Long__		u_l;
	__U_ULong__		u_ul;
	__U_Float__		u_f;
	char*			str;
};

struct __S_ARG__ { // structure for functio argument
	byte					type;
	__U_Data__	dat;
};

struct __S_Ret__ {
	ushort				id;
	byte					stat;
	byte					type_ret_data;
	__U_Data__	dat;
	byte					checksum;
};

union __U_Ret__ {
	__S_Ret__		s_ret;
	byte			byArr [ __RET_DATA_LENGTH__ ];
};

volatile byte			__cmd_i2c__;
volatile byte			__rcvBuf_i2c__[ __MAX_I2C_DATA_LENGTH__ ];
volatile __U_Ret__	u_ret = {{0,} };
__S_ARG__	__args_i2c__[ __MAX_ARG_NUM__ ] = {{0,}};

void		__onReceive_i2c__(int);
void		__onRequest_i2c__();

void(*__funcs_i2c__[ __MAX_FUNC_NUM__ ] )(void)  = { NULL, };
byte __func_indexes_i2c__[ __MAX_FUNC_NUM__ ] = {0, };
byte __no_funcs_i2c__ = 0;

char __strBuf_i2c__[ __STR_BUF_LENGTH__ ] = {0,};

boolean __bool_tmp__ = false;
byte __order__ = 0;

class Eziopy {
	public:
		static byte begin(byte addr, ushort device_id=65535);
		static void check();
		static void attachFunc(byte index, void (*func)(void));
		//void printToSerial(byte index);
  
		static byte getByte(byte index, boolean& bIsByte = __bool_tmp__);
		static sbyte getSByte(byte index, boolean& bIsSByte = __bool_tmp__);
		//static bool getBool(byte ineex, boolean& bIsBool = __bool_tmp__);
		static short getShort(byte index, boolean& bIsShort = __bool_tmp__);
		static short getUShort(byte index, boolean& bIsUShort = __bool_tmp__);
		static long getLong(byte index, boolean& bIsLong = __bool_tmp__);
		static long getULong(byte index, boolean& bIsULong = __bool_tmp__);
		static float getFloat(byte index, boolean& bIsFloat = __bool_tmp__);
		static char *getStr(byte index, boolean& bIsStr = __bool_tmp__);

		static void setReturn(byte);
		static void setReturn(sbyte); //static void setReturn(bool);
		static void setReturn(short);
		static void setReturn(ushort);
		static void setReturn(long);
		static void setReturn(ulong);
		static void setReturn(float);

		static void _set_stat_checksum(byte);

   private:
		//static bool _cmd_exec();
		static bool _is_valid_func_Index (byte);

		enum E_DATA {
			_DATA_NONE = 0,
			_DATA_CHAR = 1,
			_DATA_SBYTE = 2,
			_DATA_BYTE = 3,			//_DATA_BOOL = 4,
			_DATA_SHORT = 5,
			_DATA_USHORT = 6,
			_DATA_LONG = 7,
			_DATA_ULONG = 8,
			_DATA_FLOAT = 9,
			_DATA_DOUBLE = 10,
			_DATA_STR = 11
		};
};

// setReturn() function definitions ----------------------------------------------------------

void Eziopy::setReturn(byte byVal) {
	      u_ret.s_ret.type_ret_data = _DATA_BYTE;
	      u_ret.s_ret.dat.ucVal = byVal;
}	

void Eziopy::setReturn(sbyte sbVal) {
	      u_ret.s_ret.type_ret_data = _DATA_SBYTE;
	      u_ret.s_ret.dat.sbVal = sbVal;
}	

void Eziopy::setReturn(short sVal) {
	      u_ret.s_ret.type_ret_data = _DATA_SHORT;
	      u_ret.s_ret.dat.u_s.val = sVal;
}	

void Eziopy::setReturn(ushort usVal) {
	      u_ret.s_ret.type_ret_data = _DATA_USHORT;
	      u_ret.s_ret.dat.u_us.val = usVal;
}	

void Eziopy::setReturn(long lVal) {
	      u_ret.s_ret.type_ret_data = _DATA_LONG;
	      u_ret.s_ret.dat.u_l.val = lVal;
}	

void Eziopy::setReturn(ulong ulVal) {
	      u_ret.s_ret.type_ret_data = _DATA_ULONG;
	      u_ret.s_ret.dat.u_ul.val = ulVal;
}	

void Eziopy::setReturn(float fVal) {
	      u_ret.s_ret.type_ret_data = _DATA_FLOAT;
	      u_ret.s_ret.dat.u_f.val = fVal;
}	

// public methods definitino ------------------------------------------------------

byte Eziopy::begin(byte addr, ushort device_id) {
	u_ret.s_ret.id = device_id;
	_set_stat_checksum( __STAT_READY__ );

	// if addr in EEPROM is valid, use that.
	byte addr_real = addr;
	byte addr_in_eeprom = EEPROM.read( __EEPROM_ADDR__ );
	if ( __MIN_I2C_ADDR__ <= addr_in_eeprom &&  addr_in_eeprom <= __MAX_I2C_ADDR__ )
		addr_real = addr_in_eeprom;
	Wire.begin( addr_real );

	Wire.onRequest(__onRequest_i2c__);
	Wire.onReceive(__onReceive_i2c__);

	return addr_real;	
}

void Eziopy::attachFunc( byte index, void (*func)(void) ){
//void(*__funcs_i2c__[ __MAX_FUNC_NUM__ ] )(void)  = { NULL, };
//byte __func_indexes_i2c__[ __MAX_FUNC_NUM__ ] = {0, };
//byte __no_funcs_i2c__ = 0;
	__func_indexes_i2c__[ __no_funcs_i2c__ ++ ] = index;
	__funcs_i2c__[index] = func;
}

bool Eziopy::_is_valid_func_Index (byte index) {
	for (byte k=0; k<__no_funcs_i2c__; k++) {
		if ( __func_indexes_i2c__[ k ] == index ) return true;
	}
	return false;
}

sbyte Eziopy::getSByte(byte index, boolean& bIsSByte) {
  bIsSByte = __args_i2c__[index].type == _DATA_SBYTE;
  return __args_i2c__[index].dat.sbVal;
}

byte Eziopy::getByte(byte index, boolean& bIsByte) {
  bIsByte = __args_i2c__[index].type == _DATA_BYTE;
  return __args_i2c__[index].dat.ucVal;
}

short Eziopy::getShort(byte index, boolean& bIsShort) {
  bIsShort = __args_i2c__[index].type == _DATA_SHORT;
  return __args_i2c__[index].dat.u_s.val;
}

short Eziopy::getUShort(byte index, boolean& bIsUShort) {
  bIsUShort = __args_i2c__[index].type == _DATA_USHORT;
  return __args_i2c__[index].dat.u_us.val;
}

long Eziopy::getLong(byte index, boolean& bIsLong) {
  bIsLong = __args_i2c__[index].type == _DATA_LONG;
  return __args_i2c__[index].dat.u_l.val;
}

long Eziopy::getULong(byte index, boolean& bIsULong) {
  bIsULong = __args_i2c__[index].type == _DATA_ULONG;
  return __args_i2c__[index].dat.u_ul.val;
}

float Eziopy::getFloat(byte index, boolean& bIsFloat){
  bIsFloat = __args_i2c__[index].type == _DATA_FLOAT;
  return __args_i2c__[index].dat.u_f.val;
}
    
char *Eziopy::getStr(byte index, boolean& bIsStr){
  bIsStr = __args_i2c__[index].type == _DATA_STR;
  return __args_i2c__[index].dat.str;
}

void Eziopy:: _set_stat_checksum(byte stat) { 
	u_ret.s_ret.stat = stat;
	// motolora checksum
	byte csum = 0;
	for(byte k=0; k<__RET_DATA_LENGTH__-1; k++)
		csum += u_ret.byArr[k];
	u_ret.s_ret.checksum = 0xff - csum;
}

void Eziopy:: check() {
    if  ( u_ret.s_ret.stat == __STAT_NOT_READY__ ) { 
	byte index;

        switch (__cmd_i2c__) { 

		case CMD_CHANGE_ADDR:
			if ( __rcvBuf_i2c__[0] == __CHANGE_ADDR_CONFIRM__ ) {
				byte newAddr = __rcvBuf_i2c__[1];
				if (newAddr == __rcvBuf_i2c__[2]) {
					EEPROM.write( __EEPROM_ADDR__, __rcvBuf_i2c__[1] );
					_set_stat_checksum(__STAT_READY__); return;
					//Serial.println("chngd");
				} else {
					//u_ret.s_ret.stat = __STAT_ERR_DATA__ ;return true;
					 _set_stat_checksum(__STAT_ERR_DATA__); return;
				}
			} else {
				//u_ret.s_ret.stat = __STAT_ERR_DATA__ ; return  true;
				_set_stat_checksum(__STAT_ERR_DATA__); return;
			}
			break;

        	case CMD_EXEC_FUNC: // revBuf = [cnfrm, index, index]
    			index = __rcvBuf_i2c__[0]; // index of func to execute
			if ( _is_valid_func_Index(index) ) {
				u_ret.s_ret.type_ret_data = _DATA_NONE;
				__funcs_i2c__[index]();
				_set_stat_checksum(__STAT_READY__); return;
			} else { 	//Serial.println("func arg err");
				_set_stat_checksum(__STAT_ERR_FUNC_INDEX__); return;
			}
    			break;
 
		case CMD_SEND_DATA: // revBuf = [cnfrm, index, index, dtype, data0, data1, data2, data3]
		        index = __rcvBuf_i2c__[0];
			if ( index < __MAX_ARG_NUM__ ) {

				switch (__rcvBuf_i2c__[1]) {  // dtype

					case _DATA_SBYTE:	
						__args_i2c__[index].type = _DATA_SBYTE;
						__args_i2c__[index].dat.sbVal = __rcvBuf_i2c__[2];
						setReturn( __args_i2c__[index].dat.sbVal ) ;
						break;

					case _DATA_BYTE:
						__args_i2c__[index].type = _DATA_BYTE;
						__args_i2c__[index].dat.ucVal = __rcvBuf_i2c__[2];
						setReturn( __args_i2c__[index].dat.ucVal );
						break;

					case _DATA_SHORT:
						__args_i2c__[index].type = _DATA_SHORT;
						__args_i2c__[index].dat.u_s.byArr[0]= __rcvBuf_i2c__[2];
						__args_i2c__[index].dat.u_s.byArr[1]= __rcvBuf_i2c__[3];
						setReturn( __args_i2c__[index].dat.u_s.val);
						break;

					case _DATA_USHORT:
						__args_i2c__[index].type = _DATA_USHORT;
						__args_i2c__[index].dat.u_us.byArr[0]= __rcvBuf_i2c__[2];
						__args_i2c__[index].dat.u_us.byArr[1]= __rcvBuf_i2c__[3];
						setReturn ( __args_i2c__[index].dat.u_us.val);
						break;
						
					case _DATA_LONG:
						__args_i2c__[index].type = _DATA_LONG;
						__args_i2c__[index].dat.u_l.byArr[0]= __rcvBuf_i2c__[2];
						__args_i2c__[index].dat.u_l.byArr[1]= __rcvBuf_i2c__[3];
						__args_i2c__[index].dat.u_l.byArr[2]= __rcvBuf_i2c__[4];
						__args_i2c__[index].dat.u_l.byArr[3]= __rcvBuf_i2c__[5];
						setReturn( __args_i2c__[index].dat.u_l.val);
						break;

					case _DATA_ULONG:
						__args_i2c__[index].type = _DATA_ULONG;
						__args_i2c__[index].dat.u_ul.byArr[0]= __rcvBuf_i2c__[2];
						__args_i2c__[index].dat.u_ul.byArr[1]= __rcvBuf_i2c__[3];
						__args_i2c__[index].dat.u_ul.byArr[2]= __rcvBuf_i2c__[4];
						__args_i2c__[index].dat.u_ul.byArr[3]= __rcvBuf_i2c__[5];
						setReturn( __args_i2c__[index].dat.u_ul.val);
						break;

					case _DATA_FLOAT:
						__args_i2c__[index].type = _DATA_FLOAT;
						__args_i2c__[index].dat.u_f.byArr[0]= __rcvBuf_i2c__[2];
						__args_i2c__[index].dat.u_f.byArr[1]= __rcvBuf_i2c__[3];
						__args_i2c__[index].dat.u_f.byArr[2]= __rcvBuf_i2c__[4];
						__args_i2c__[index].dat.u_f.byArr[3]= __rcvBuf_i2c__[5];
						setReturn( __args_i2c__[index].dat.u_f.val);
						break;

					case _DATA_STR:
						__args_i2c__[index].type = _DATA_STR;
						for (byte k=0; k< __STR_BUF_LENGTH__  ; k++) {
							__strBuf_i2c__[k]= __rcvBuf_i2c__[k+2];
							if ( __strBuf_i2c__ [k] == 0 ) break;
						}
						__args_i2c__[index].dat.str = __strBuf_i2c__;
						break;

					default:
						//Serial.print("unknown dtype:"); Serial.println( __rcvBuf_i2c__[3] );
						_set_stat_checksum(__STAT_ERR_DATA__); return;
						break;

					} //  switch (__rcvBuf_i2c__[2])
					_set_stat_checksum(__STAT_READY__); return;
				}  else { // if (index < _MAX_ARG_NUM__)
					_set_stat_checksum(__STAT_ERR_ARG_INDEX__); return;
				}
				break;

			default:
				//Serial.print("unknown cmd:"); Serial.println(__cmd_i2c__);
				_set_stat_checksum(__STAT_ERR_DATA__); return;
				break;
		} // switch (__cmd_i2c__)
		_set_stat_checksum(__STAT_READY__); return;
	} //else {// if  (! u_ret.s_ret.stat == __STAT_NOT_READY__ ) { 
}

/*
void Eziopy::printToSerial(byte index) {
	//__b_is_ready_i2c__ = false;
	u_ret.s_ret.stat = __STAT_NOT_READY__; // bDeviceReady	= false;

	Serial.print(index);
	switch( __args_i2c__[index].type ) {

		case _DATA_SBYTE:
			Serial.print(" (sbyte) ");
			Serial.println( __args_i2c__[index].dat.sbVal);
			break;

		case _DATA_BYTE:
			Serial.print(" (byte) ");
			Serial.println( __args_i2c__[index].dat.ucVal);
			break;

		case _DATA_BOOL:
			Serial.print(" (bool) ");
			Serial.println( __args_i2c__[index].dat.bVal);
			break;

		case _DATA_SHORT:
			Serial.print(" (short) ");
			Serial.println( __args_i2c__[index].dat.u_s.val);
			break;

		case _DATA_USHORT:
			Serial.print(" (ushort) ");
			Serial.println( __args_i2c__[index].dat.u_us.val);
			break;

		case _DATA_LONG:
			Serial.print(" (long) ");
			Serial.println( __args_i2c__[index].dat.u_l.val);
			break;

		case _DATA_ULONG:
			Serial.print(" (ulong) ");
			Serial.println( __args_i2c__[index].dat.u_ul.val);
			break;

		case _DATA_FLOAT:
			Serial.print( " (float) ");
			Serial.println( __args_i2c__[index].dat.u_f.val);
			break;

		case _DATA_STR:
			Serial.print( " (str) ");
			Serial.println( __strBuf_i2c__ );
			break;
	}
	//__b_is_ready_i2c__ = true;
	 u_ret.s_ret.stat = __STAT_READY__; // bDeviceReady	= true;
}
//*/

Eziopy Ezic;

// i2c ISR functions ----------------------------------------------------------------------
byte __byIndex_i2c__ = 0;
byte __csum_i2c__ = 0;

void __onReceive_i2c__(int count) { //static function
	__cmd_i2c__ = Wire.read(); // first byte is ALWAYS command
							// when requesting, only CMD_READ. is received
	if (count > 1) {   //
		__byIndex_i2c__ = 0;
		while( Wire.available() )
			__rcvBuf_i2c__[ __byIndex_i2c__ ++] = Wire.read();

		__csum_i2c__ = __cmd_i2c__; // inlcude cmd in checksum
		for(__byIndex_i2c__ = 0; __byIndex_i2c__< count-2; __byIndex_i2c__ ++)
			__csum_i2c__ += __rcvBuf_i2c__[ __byIndex_i2c__ ] ;
		__csum_i2c__ = 0xff - __csum_i2c__;

		if ( __csum_i2c__ == __rcvBuf_i2c__[count-2] )
			u_ret.s_ret.stat = __STAT_NOT_READY__;
		else
			Ezic._set_stat_checksum( __STAT_ERR_DATA__ );
    	}
}

void __onRequest_i2c__() {
	Wire.write( (const byte*)u_ret.byArr, __RET_DATA_LENGTH__ );
}

//===============================================
// loop() ==========================================
//===============================================
///*
void func0() {  //pinMode(pin, mode=0(input)/1(output)/2(input_pullup)
	byte pin = Ezic.getByte(0);
	byte mode = Ezic.getByte(1);
	pinMode(pin, mode);
}

void func1() {  //digitalWrite(pin, val)
	byte pin = Ezic.getByte(0);
	byte val = Ezic.getByte(1);
	digitalWrite(pin, val);
}

void func2() { // digitalRead(pin, pullup = false)
	byte pin = Ezic.getByte(0);
	byte ret = digitalRead(pin);
	Ezic.setReturn(ret);
//	Serial.print("func1:");
//	Serial.println(ret);
}

void func3() { // analogRead()
	byte pin = Ezic.getByte(0);
	ushort usVal = analogRead(pin);
	Ezic.setReturn(usVal);
}

void func4() { //analogWrite()
	byte pin = Ezic.getByte(0);
	byte val = Ezic.getByte(1);
	analogWrite(pin, val);
}

ulong cnt = 0;
void func5() { //analogWrite()
	Ezic.setReturn(cnt++);
}


void setup() {
///*
	Ezic.attachFunc(0, func0);
	Ezic.attachFunc(1, func1);
	Ezic.attachFunc(2, func2);
	Ezic.attachFunc(3, func3);
	Ezic.attachFunc(4, func4);
	Ezic.attachFunc(5, func5);
//*/

	byte addr = Ezic.begin(0x10, 300);
  
	Serial.begin(115200);
	Serial.print("052 : i2c addr is " );
	Serial.println(addr);
}

void loop() {
	Ezic.check();
	//if (cmd == CMD_SEND_DATA ) {	Ezic.printToSerial(index);	}
}
