/***********************************************************************
* Harper.h ver 1.0
* Arduino library for i2c (slave) communication with Raspberry pi.
* by salesioPark(박장현, 국립목포대학교, 전기제어공학과)
* <Wire.h> must be included before including <Harper.h> in user .ino file
************************************************************************/

//#include "Arduino.h"
#include "Harper.h"
#include "Wire.h"
#include <avr/eeprom.h>


// static memeber initialization --------------------------------------------

Eziopy::_U_Id				Eziopy::_u_id = {0xffffffff, };
volatile byte				Eziopy::_cmd = 0 ;
volatile byte				Eziopy::_cmd_i2c = 0 ;
volatile byte				Eziopy::_rcvBuf[ __MAX_I2C_READ_BUF_LEN__ ] = {0,};
volatile Eziopy::_U_Ret	Eziopy::_u_ret = {{1,} };
char							Eziopy::_strBuf[ __STR_BUF_LENGTH__ ] = {0,};
byte							Eziopy::_checksum = 0;
byte							Eziopy::_idx = 0;
volatile byte				Eziopy::_stat = 0;
//volatile byte 				Eziopy::_stat_write = 1;
volatile byte 				Eziopy::_statArr[2]={0,};
volatile Eziopy::_S_Arg*	Eziopy::_args = NULL;
byte 							Eziopy::_max_arg_num = __INIT_MAX_ARG_NUM__;
pfunc_t*						Eziopy::_tmpFuncArr = NULL;
byte							Eziopy::_num_funcs = 0;
pfunc_t*						Eziopy::_funcArr = NULL;
volatile byte 				Eziopy::_len_just_rcvd = 0;
// ---------
void Eziopy::set_max_arg_num(byte num) {
///*
	if (_args == NULL) // before calling begin() 
		_max_arg_num = num;
	else { // after calling begin() 
		delete [] _args;
		_args = new _S_Arg[num];
	}
//*/
}

// set_ret() functions overloading definitions -----------------------------------------

void Eziopy::set_ret(uint8_t byVal) {
	if (_stat == _STAT_UNDER_NORMAL_PROC ) {  //if no error occurred
		_u_ret.s_ret.type = _DTYPE_UINT8;
		_u_ret.s_ret.dat.ucVal = byVal;
	}
}	

void Eziopy::set_ret(int8_t sbVal) {
	if (_stat == _STAT_UNDER_NORMAL_PROC ) {  //if no error occurred
		_u_ret.s_ret.type = _DTYPE_INT8;
		_u_ret.s_ret.dat.sbVal = sbVal;
	}
}	

void Eziopy::set_ret(int16_t sVal) {
	if (_stat == _STAT_UNDER_NORMAL_PROC ) {  //if no error occurred
		_u_ret.s_ret.type = _DTYPE_INT16;
		_u_ret.s_ret.dat.u_s.val = sVal;
	}
}	

void Eziopy::set_ret(uint16_t usVal) {
	if (_stat == _STAT_UNDER_NORMAL_PROC ) {  //if no error occurred
		_u_ret.s_ret.type = _DTYPE_UINT16;
		_u_ret.s_ret.dat.u_us.val = usVal;
	}
}	

void Eziopy::set_ret(int32_t lVal) {
	if (_stat == _STAT_UNDER_NORMAL_PROC ) {  //if no error occurred
		_u_ret.s_ret.type = _DTYPE_INT32;
		_u_ret.s_ret.dat.u_l.val = lVal;
	}
}	

void Eziopy::set_ret(uint32_t ulVal) {
	if (_stat == _STAT_UNDER_NORMAL_PROC ) {  //if no error occurred
		_u_ret.s_ret.type = _DTYPE_UINT32;
		_u_ret.s_ret.dat.u_ul.val = ulVal;
	}
}	

void Eziopy::set_ret(float fVal) {
	if (_stat == _STAT_UNDER_NORMAL_PROC ) {  //if no error occurred
		_u_ret.s_ret.type = _DTYPE_FLOAT;
		_u_ret.s_ret.dat.u_f.val = fVal;
	}
}	

// public methods definition ------------------------------------------------------

byte Eziopy::begin(byte addr, uint32_t dev_id) {
	// store id and some infos
	_u_id.s_id.val = dev_id;
	_u_id.s_id.numArgs = _max_arg_num;
	_u_id.s_id.numFuncs = _num_funcs;
	//_u_id.s_id.numInfo = 0; //reserved
	_u_id.s_id.checksum = 0xff - (byte)(_CMD_READ_ID
			+ _u_id.byArr[0] + _u_id.byArr[1] + _u_id.byArr[2] + _u_id.byArr[3]
			+ _u_id.byArr[4]
			+ _u_id.byArr[5] + _u_id.byArr[6]);

	// if addr in EEPROM is valid, use that.
	byte addr_real = addr;
	byte addr_in_eeprom = eeprom_read_byte( (uint8_t *) __EEPROM_ADDR__ );
	if ( __MIN_I2C_ADDR__ <= addr_in_eeprom && addr_in_eeprom <= __MAX_I2C_ADDR__ )
		addr_real = addr_in_eeprom;

	// malloc argArr and set all as NONE
	_args = new _S_Arg [_max_arg_num];
	_reset_all_args();

	Wire.onReceive( _onReceive );
	Wire.onRequest( _onRequest );

	_stat = STAT_CMD_COMPLETED;
	Wire.begin( addr_real );
	return addr_real;	
}

// user function adding : no num limit
void Eziopy::add_func( void (*func)(void) ){
	_num_funcs ++;
	_tmpFuncArr = new pfunc_t[_num_funcs];

	// copy all the previous function pointers
	for( _idx=0; _idx < _num_funcs -1; _idx++) {
		_tmpFuncArr[_idx] = _funcArr[_idx];
	}
	// add newly received function pointer at the end of array
	_tmpFuncArr[_num_funcs -1] = func;

	// update function pointer array
	delete [] _funcArr;
	_funcArr = _tmpFuncArr;
}

// get_data() methods definition -----------------------------------
// 정확한 데이터형의 인자가 저장되어있는 지를 먼저 체크한다.
// 만약 그렇지 않다면 _u_ret.s_ret.info 에 인자의 인덱스를 저장하고
// stat에 에러 코드를 저장한다.

void Eziopy::_check_if_no_err_to_get_arg(byte index, byte dtype) {
	if (_stat == _STAT_UNDER_NORMAL_PROC ) {  //if currently no error occurred
		if ( _args[index].type == _DTYPE_NONE) { // if there is no arg received
			_u_ret.s_ret.info = index;
			_stat = STAT_ERR_NO_ARG;
		} else if (_args[index].type != dtype) { // arg type mismatch
			_u_ret.s_ret.info = index;
			_stat = STAT_ERR_ARG_TYPE;
		}		
	}
}

int8_t Eziopy::get_int8(byte index) {
	_check_if_no_err_to_get_arg(index, _DTYPE_INT8);
  return _args[index].dat.sbVal;
}

uint8_t Eziopy::get_uint8(byte index) {
	_check_if_no_err_to_get_arg(index, _DTYPE_UINT8);
  return _args[index].dat.ucVal;
}


uint8_t Eziopy::get_byte(byte index) {
	_check_if_no_err_to_get_arg(index, _DTYPE_UINT8);
	return _args[index].dat.ucVal;
}

int16_t Eziopy::get_int16(byte index) {
  	_check_if_no_err_to_get_arg(index, _DTYPE_INT16);
  return _args[index].dat.u_s.val;
}

int16_t Eziopy::get_int(byte index) {
	_check_if_no_err_to_get_arg(index, _DTYPE_INT16);
  return _args[index].dat.u_s.val;
}

uint16_t Eziopy::get_uint16(byte index) {
	_check_if_no_err_to_get_arg(index, _DTYPE_UINT16);
  return _args[index].dat.u_us.val;
}

int32_t Eziopy::get_int32(byte index) {
	_check_if_no_err_to_get_arg(index, _DTYPE_INT32);
  return _args[index].dat.u_l.val;
}

uint32_t Eziopy::get_uint32(byte index) {
	_check_if_no_err_to_get_arg(index, _DTYPE_UINT32);
  return _args[index].dat.u_ul.val;
}

float Eziopy::get_float(byte index){
	_check_if_no_err_to_get_arg(index, _DTYPE_FLOAT);
  return _args[index].dat.u_f.val;
}
    
char* Eziopy::get_str(byte index){
	_check_if_no_err_to_get_arg(index, _DTYPE_STR);
  return _args[index].dat.str;
}


// 모든 함수 인자를 NONE으로 초기화한다.
// 맨 처음과 매 함수의 실행이 끝난 후 호출하여 인자들을 리셋한다.
void Eziopy:: _reset_all_args() {
	for(_idx=0; _idx < _max_arg_num; _idx++)
		_args[_idx].type = _DTYPE_NONE; 
}

// i2c로 지령(cmd)를 넘겨받은 직후 실행되는 함수.
// 모든 명령을 처리한 후 정상/에러발생 여부를 저장한다.
void Eziopy:: check() {
	if  ( _stat == _STAT_UNDER_NORMAL_PROC ) { 
		byte index; // 이것 대신 _idx를 사용하면 안됨
		_cmd = _rcvBuf[0]; //ISR에서 사용되는 변수가 아님

		switch (_cmd) { 

			case _CMD_CHANGE_ADDR: // rcvBuf:[cmd, addr]
				eeprom_update_byte( (uint8_t *)__EEPROM_ADDR__, _rcvBuf[1]); 					
				_stat = STAT_CMD_COMPLETED;
				return;//break;

        	case _CMD_EXEC_FUNC: // revBuf:[cmd, func_index]
	 			index = _rcvBuf[1]; // index of func to execute
				//if (index < _num_funcs ) { // 파이썬에서 처리함
					_u_ret.s_ret.type = _DTYPE_NONE; //반환값을 먼저 NONE으로 리셋
					_funcArr[index](); //<= 이 안에서 오류가 발생할 수 있다.
					_reset_all_args(); // 모든 인자를 NONE으로 리셋한다.
					if (_stat == _STAT_UNDER_NORMAL_PROC ) {					
						_stat = STAT_CMD_COMPLETED;
					} else { // some error in input args occurs
						_u_ret.byArr[RET_BYTE0] = index; //function index
					}
					return;
				//} else {
				//	_stat = _STAT_ERR_FUNC_INDEX;
				//	return;
				// }
    			break;
 
		case _CMD_SEND_DATA : // rcvBuf:[cmd, index, dtype, data0, data1,...]
		   index = _rcvBuf[1];
			//if ( index < _max_arg_num ) { // 인덱스는 pythone에서 체크한다.

				switch (_rcvBuf[2]) {  // dtype

					case _DTYPE_INT8:	
						_args[index].type = _DTYPE_INT8;
						_args[index].dat.sbVal = _rcvBuf[3];
						break;

					case _DTYPE_UINT8:
						_args[index].type = _DTYPE_UINT8;
						_args[index].dat.ucVal = _rcvBuf[3];
						break;

					case _DTYPE_INT16:
						_args[index].type = _DTYPE_INT16;
						_args[index].dat.u_s.byArr[0]= _rcvBuf[3];
						_args[index].dat.u_s.byArr[1]= _rcvBuf[4];
						break;

					case _DTYPE_UINT16:
						_args[index].type = _DTYPE_UINT16;
						_args[index].dat.u_us.byArr[0]= _rcvBuf[3];
						_args[index].dat.u_us.byArr[1]= _rcvBuf[4];
						break;
						
					case _DTYPE_INT32:
						_args[index].type = _DTYPE_INT32;
						_args[index].dat.u_l.byArr[0]= _rcvBuf[3];
						_args[index].dat.u_l.byArr[1]= _rcvBuf[4];
						_args[index].dat.u_l.byArr[2]= _rcvBuf[5];
						_args[index].dat.u_l.byArr[3]= _rcvBuf[6];
						break;

					case _DTYPE_UINT32:
						_args[index].type = _DTYPE_UINT32;
						_args[index].dat.u_ul.byArr[0]= _rcvBuf[3];
						_args[index].dat.u_ul.byArr[1]= _rcvBuf[4];
						_args[index].dat.u_ul.byArr[2]= _rcvBuf[5];
						_args[index].dat.u_ul.byArr[3]= _rcvBuf[6];
						break;

					case _DTYPE_FLOAT:
						_args[index].type = _DTYPE_FLOAT;
						_args[index].dat.u_f.byArr[0]= _rcvBuf[3];
						_args[index].dat.u_f.byArr[1]= _rcvBuf[4];
						_args[index].dat.u_f.byArr[2]= _rcvBuf[5];
						_args[index].dat.u_f.byArr[3]= _rcvBuf[6];
						break;

					case _DTYPE_STR:
						_args[index].type = _DTYPE_STR;
						for ( _idx=0; _idx < __STR_BUF_LENGTH__  ; _idx++) {
							_strBuf[_idx]= _rcvBuf[_idx+3];
							if ( _strBuf [_idx] == 0 ) break;
						}
						_args[index].dat.str = _strBuf;
						break;

					default:
						_stat = _STAT_ERR_DATA_101;
						return;
						break;

					} //  switch (_rcvBuf[2])
					_u_ret.s_ret.info = index; // store index to info byte
					_stat = STAT_CMD_COMPLETED;
					return;
				//}  else { // if (index < _MAX_ARG_NUM__)
				//	_stat= _STAT_ERR_ARG_INDEX;
				//	return;
				//}
				break;

			default:
				_u_ret.s_ret.info = _cmd;
				_stat = _STAT_ERR_DATA_102;
				return;
				break;
		} // switch (_cmd)
		_stat = STAT_CMD_COMPLETED;
		return;
	
	} // if  (! _stat == _STAT_UNDER_NORMAL_PROC ) { 
}

// i2c ISR functions ----------------------------------------------------------------------

void Eziopy::_onReceive(int count) { //static function
	_cmd_i2c = Wire.read(); // first byte is ALWAYS command
	if (count > 1) {
		_len_just_rcvd = (byte)count-1;

		_idx = 0;
		while( Wire.available() )
			_rcvBuf[_idx++] = Wire.read();
	} //if (count>1)
}

void Eziopy::_onRequest() {

	switch(_cmd_i2c) {

		case _CMD_SEND_BACK:
			Wire.write( (const byte*)_rcvBuf, _len_just_rcvd);
			//_stat_write = _WRT_FAIL;//<-
			break;

		case _CMD_CHECK_OK:
			_stat = _STAT_UNDER_NORMAL_PROC;
			_statArr[0] = _stat;
			_statArr[1]	=	0xff-(_CMD_CHECK_OK + _stat);
			Wire.write( (const byte*)_statArr, 2);
			break;

		case _CMD_READ_STAT:
			_statArr[0] = _stat;
			_statArr[1]	=	0xff-(_CMD_READ_STAT + _stat);
			Wire.write( (const byte*)_statArr, 2);
			break;

		case _CMD_READ_DATA:
			_checksum = _CMD_READ_DATA;
			for(_idx=0; _idx<__RET_DATA_LENGTH__-1; _idx++)
				_checksum += _u_ret.byArr[_idx];
			_u_ret.s_ret.checksum = 0xff - _checksum;
			Wire.write( (const byte*)_u_ret.byArr, __RET_DATA_LENGTH__ );
			break;

		case _CMD_READ_ID:
			Wire.write( (const byte*)_u_id.byArr, 7);
			break;
			
		default:
			Wire.write(255);
			break;
	} 
}

Eziopy Harper;
