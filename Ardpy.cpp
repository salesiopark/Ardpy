/***********************************************************************
* Ardpy.cpp
* Arduino library for i2c (slave) communication with Raspberry pi.
* by salesiopark(박장현, 국립목포대학교, 전기제어공학과)
* <Wire.h> must be included before including <Ardpy.h> in user .ino file
************************************************************************/
#define __VER_ARDPY__ __VER__(1,1,4)
// includes ------------------------------------------------------------------
#include "Ardpy.h"
#include "Wire.h"
#include <avr/eeprom.h>
// static memebers initialization --------------------------------------------
_HRP_::_U_Id            _HRP_::_u_id = {0xffffffff, };
volatile byte           _HRP_::_cmd = 0 ;
volatile byte           _HRP_::_cmd_i2c = 0 ;
volatile byte           _HRP_::_rcvBuf[ __MAX_I2C_READ_BUF_LEN__ ] = {0,};
//volatile byte           _HRP_::__sbuf__[ __MAX_I2C_READ_BUF_LEN__ ] = {0,};
byte                    _HRP_::_idx = 0;
volatile _HRP_::_U_Ret  _HRP_::_u_ret = {{1,} };
char                    _HRP_::_strBuf[ __STR_BUF_LENGTH__ ] = {0,};
byte                    _HRP_::_checksum = 0;
volatile byte           _HRP_::_stat = 0;
volatile byte           _HRP_::_statArr[2]={0,};
volatile _HRP_::_S_Arg* _HRP_::_args = NULL;
byte                    _HRP_::_max_arg_num = __INIT_MAX_ARG_NUM__;
pfunc_t*                _HRP_::_tmpFuncArr = NULL;
byte                    _HRP_::_num_funcs = 0;
pfunc_t*                _HRP_::_funcArr = NULL;

// ---------
void _HRP_::set_max_arg_num(byte num) {
///*
	if (_args == NULL) { // before calling begin() 
		_max_arg_num = num;
	} else { // after calling begin() 
		delete [] _args;
		_args = new _S_Arg[num];
	}
//*/
}

// set_ret() function overloading definitions -----------------------------------------

void _HRP_::set_ret(uint8_t byVal) {
	if (_stat == _STAT_UNDER_NORMAL_PROC ) {  //if no error occurred
		_u_ret.s_ret.type = _DTYPE_UINT8;
		_u_ret.s_ret.dat.ucVal = byVal;
	}
}	

void _HRP_::set_ret(int8_t sbVal) {
	if (_stat == _STAT_UNDER_NORMAL_PROC ) {  //if no error occurred
		_u_ret.s_ret.type = _DTYPE_INT8;
		_u_ret.s_ret.dat.sbVal = sbVal;
	}
}	

void _HRP_::set_ret(int16_t sVal) {
	if (_stat == _STAT_UNDER_NORMAL_PROC ) {  //if no error occurred
		_u_ret.s_ret.type = _DTYPE_INT16;
		_u_ret.s_ret.dat.u_s.val = sVal;
	}
}	

void _HRP_::set_ret(uint16_t usVal) {
	if (_stat == _STAT_UNDER_NORMAL_PROC ) {  //if no error occurred
		_u_ret.s_ret.type = _DTYPE_UINT16;
		_u_ret.s_ret.dat.u_us.val = usVal;
	}
}	

void _HRP_::set_ret(int32_t lVal) {
	if (_stat == _STAT_UNDER_NORMAL_PROC ) {  //if no error occurred
		_u_ret.s_ret.type = _DTYPE_INT32;
		_u_ret.s_ret.dat.u_l.val = lVal;
	}
}	

void _HRP_::set_ret(uint32_t ulVal) {
	if (_stat == _STAT_UNDER_NORMAL_PROC ) {  //if no error occurred
		_u_ret.s_ret.type = _DTYPE_UINT32;
		_u_ret.s_ret.dat.u_ul.val = ulVal;
	}
}	

void _HRP_::set_ret(float fVal) {
	if (_stat == _STAT_UNDER_NORMAL_PROC ) {  //if no error occurred
		_u_ret.s_ret.type = _DTYPE_FLOAT;
		_u_ret.s_ret.dat.u_f.val = fVal;
	}
}	

// public methods definition ------------------------------------------------------

byte _HRP_::begin(byte addr, uint32_t dev_id, uint16_t ver_firmw) {
	// store id and some infos
	_u_id.s_id.dev_id = dev_id;
	_u_id.s_id.numArgs = _max_arg_num;
	_u_id.s_id.numFuncs = _num_funcs;
    //_u_id.s_id.verArdpy = __VER__(__V_APY_A__,__V_APY_B__,__V_APY_C__);
    _u_id.s_id.verArdpy = __VER_ARDPY__;
    _u_id.s_id.verFirmw = ver_firmw;
	
    // 체크썸(_u_id.byArr[6])을 계산 
    _u_id.s_id.checksum = 0xff - (byte)( _CMD_READ_ID
        + _u_id.byArr[0] + _u_id.byArr[1] + _u_id.byArr[2] + _u_id.byArr[3]
        + _u_id.byArr[4]
        + _u_id.byArr[5]
        + _u_id.byArr[6] + _u_id.byArr[7]
        + _u_id.byArr[8] + _u_id.byArr[9]
    );

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
    
    #ifdef __DEBUG__ //################################
        Serial.begin(115200); // for debugging
    #endif //##########################################
    
	return addr_real;	
}

// user function adding : no num limit
void _HRP_::add_func( void (*func)(void) ){
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

// get_data(index, dtype) methods definition -----------------------------------
// 정확한 데이터형의 인자가 저장되어있는 지를 먼저 체크한다.
// 만약 그렇지 않다면 _u_ret.s_ret.info 에 인자의 인덱스를 저장하고
// stat에 에러 코드를 저장한다.

void _HRP_::_check_if_no_err_to_get_arg(byte index, byte dtype) {
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

int8_t _HRP_::get_int8(byte index) {
	_check_if_no_err_to_get_arg(index, _DTYPE_INT8);
  return _args[index].dat.sbVal;
}

uint8_t _HRP_::get_uint8(byte index) {
	_check_if_no_err_to_get_arg(index, _DTYPE_UINT8);
  return _args[index].dat.ucVal;
}


uint8_t _HRP_::get_byte(byte index) {
	_check_if_no_err_to_get_arg(index, _DTYPE_UINT8);
	return _args[index].dat.ucVal;
}

int16_t _HRP_::get_int16(byte index) {
  	_check_if_no_err_to_get_arg(index, _DTYPE_INT16);
  return _args[index].dat.u_s.val;
}

int16_t _HRP_::get_int(byte index) {
	_check_if_no_err_to_get_arg(index, _DTYPE_INT16);
  return _args[index].dat.u_s.val;
}

uint16_t _HRP_::get_uint16(byte index) {
	_check_if_no_err_to_get_arg(index, _DTYPE_UINT16);
  return _args[index].dat.u_us.val;
}

int32_t _HRP_::get_int32(byte index) {
	_check_if_no_err_to_get_arg(index, _DTYPE_INT32);
  return _args[index].dat.u_l.val;
}

uint32_t _HRP_::get_uint32(byte index) {
	_check_if_no_err_to_get_arg(index, _DTYPE_UINT32);
  return _args[index].dat.u_ul.val;
}

float _HRP_::get_float(byte index){
	_check_if_no_err_to_get_arg(index, _DTYPE_FLOAT);
  return _args[index].dat.u_f.val;
}
    
char* _HRP_::get_str(byte index){
	_check_if_no_err_to_get_arg(index, _DTYPE_STR);
  return _args[index].dat.str;
}


// 모든 함수 인자를 NONE으로 초기화한다.
// 맨 처음과 매 함수의 실행이 끝난 후 호출하여 인자들을 리셋한다.
void _HRP_:: _reset_all_args() {
	for(_idx=0; _idx < _max_arg_num; _idx++)
		_args[_idx].type = _DTYPE_NONE; 
}

// i2c로 지령(cmd)를 넘겨받은 직후 실행되는 함수.
// 모든 명령을 처리한 후 정상/에러발생 여부를 저장한다.
void _HRP_:: check() {
    // 만약 이 함수를 loop() 안에서 실행한다면 아래를 추가해야 한다.
    if (_stat != _STAT_UNDER_NORMAL_PROC ) return;
    
    byte index; // 이것 대신 _idx를 사용하면 안됨
    _cmd = _rcvBuf[0]; //ISR에서 사용되는 변수가 아님

    switch (_cmd) { 

        case _CMD_CHANGE_ADDR: // rcvBuf:[cmd, addr]
            eeprom_update_byte( (uint8_t *)__EEPROM_ADDR__, _rcvBuf[1]); 					
            //break;보다 아래와 같이 직접 하면 실행을 단축할 것 같다.
            _stat = STAT_CMD_COMPLETED;
            return;

        case _CMD_EXEC_FUNC: // revBuf:[cmd, func_index]
	 			index = _rcvBuf[1]; // index of func to execute
				_u_ret.s_ret.type = _DTYPE_NONE; //반환값을 먼저 NONE으로 리셋
				_funcArr[index](); //<= 이 안에서 오류가 발생할 수 있다.
                                   // 왜냐면 인자를 check하는 루틴을 거치므로
				_reset_all_args(); // 모든 인자를 NONE으로 리셋한다.
				if (_stat == _STAT_UNDER_NORMAL_PROC ) {					
					_stat = STAT_CMD_COMPLETED;
				} else { // some error in input args occurs
                    //_stat = STAT_ERR_NO_ARG 혹은 STAT_ERR_ARG_TYPE
					_u_ret.byArr[RET_BYTE0] = index; //function index
				}
				return;
    			//break;
 
        case _CMD_SEND_ARG : // rcvBuf:[cmd, index, dtype, data0, data1,...]
                index = _rcvBuf[1];
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
                        _u_ret.s_ret.info = index;
						_stat = _STAT_ERR_DATA_101;
						return;
						//break;

                } //  switch (_rcvBuf[2])
				//_u_ret.s_ret.info = index; // store index to info byte
                //break;
				_stat = STAT_CMD_COMPLETED;
				return;

        default:
				_u_ret.s_ret.info = _cmd;
				_stat = _STAT_ERR_DATA_102;
				return;
				//break;
	} // switch (_cmd)
	_stat = STAT_CMD_COMPLETED;
}

/* --------------------------------------------------------------
i2c ISR functions 
smbus.write_i2c_block_data()가 수행되면 _onReceive()함수만 호출된다.

smbus.read_i2c_block_data(addr.cmd,data)가 수행되면
_onReceive()함수가 호출된 후 _onRequest()함수도 호출된다.

count에는 cmd까지 포함된 길이 즉, 데이터길이+1 이다. len(list)+1
--------------------------------------------------------------*/
void _HRP_::_onReceive(int count) { //static function
    _cmd_i2c = Wire.read(); // 첫 바이트는 *항상* command
    //만약 smbus.read_i2c_block_data()호출이라면 (count==1)
    // 여기서 종료되고 바로 _onRequest() 함수가 호출된다.
    
    #ifdef __DEBUG__ //#######################
        Serial.print("->[");
        Serial.print(_cmd_i2c);
    #endif //#################################
    
    if (count > 1) {
        // _cmd를 _rcvBuf[0]와 중복 저장하는 이유는
        // 이후에 _CMD_SEND_BACK 요구에 의해서 _cmd_i2c가 변경되기 때문이다.
        _rcvBuf[0] = _cmd_i2c;
        _idx = 1;
        //while(Wire.available()){ //이게 더 통신 오류 빈도가 작은 것 같다.
        while(--count > 0) { // 통신 오류 빈도가 더 높음. 차이없음
            _rcvBuf[_idx++] = Wire.read();

            #ifdef __DEBUG__ //########################
                Serial.print(", ");
                Serial.print(_rcvBuf[_idx-1]);
            #endif //##################################
        }
        return;
    } else { //<- smbus.read_i2c_block_data() 호출인 경우
        // 17Nov2016 아래 지연코드가 있으면 통신오류가 많이 줄어든다.
        // 이유를 모르겠다. 다만 주로 read 에서 오류가 발생하는 걸 보니
        // RPi의 i2c stretching sclk bug 와 관련이 있겠거니 짐작만.
        delayMicroseconds(200); //opt:500 min: 200
    }
    
    #ifdef __DEBUG__ //###########################
        Serial.print("]");
        Serial.println(_idx);
    #endif //#####################################

    // 이 시점에서 _idx에는 받은 데이터(cmd포함)의 길이가 남는다.
}

volatile byte __csu;
void _HRP_::_onRequest() { 
    switch(_cmd_i2c) {
        case _CMD_SEND_BACK:

            #ifdef __DEBUG__ //#######################
                Serial.print("<-[");
                for(byte k=0; k<_idx; k++){
                    Serial.print(_rcvBuf[k]);
                    if (k<_idx-1)
                        Serial.print(", ");
                }
                Serial.println("]");
            #endif //##################################
            
            // _idx는 직전 데이터의 길이가 기록되어 있다.
            
            ///*
            _checksum = _CMD_SEND_BACK;
            for(byte k=0; k<_idx; k++) {
                _checksum += _rcvBuf[k];
            }
            _rcvBuf[_idx] = 0xff - _checksum;
            //*/
            
            // 이유를 짐작해보면 Wire.write()함수를 수행하는 도중에 onReceive()가
            // 호출되어 )_rcvBuf, _idx 내용이 바뀔 수도 있지 않을까 싶다.
            // 따라서 아래와 같이 Wire.write()함수에 _rcvBuf를 넘겨주면 안된다.
            // 19Nov2016 (위의 짐작과 틀리게)그냥 아래와 같이 해도 되는 것 같다.
            // 20Nov2016 끝에 checksum을 붙여서 보낸다.
            Wire.write( (const byte*)_rcvBuf, _idx+1);
            return;

		case _CMD_CHECK_OK:
			_stat = _STAT_UNDER_NORMAL_PROC;//0
			_statArr[0] = _stat;
			//_statArr[1]=0xff-(_CMD_CHECK_OK+_stat);//255-(6+0)=249
			_statArr[1]	= 249;
			Wire.write( (const byte*)_statArr, 2);
            // 19Nov2016 ISR은 단순하게 유지해야 하으므로
            //  아래는 loop() 안에서 사용자가 추가하는 것으로 한다.
            // check();//<- place here instead of in loop() function ***
			return;

		case _CMD_READ_STAT:
			_statArr[0] = _stat;
			_statArr[1]	= 0xff - (_CMD_READ_STAT + _stat);//checksum
			Wire.write( (const byte*)_statArr, 2);
			return;

		case _CMD_READ_DATA:
			_checksum = _CMD_READ_DATA;
			for(_idx=0; _idx < sizeof(_S_Ret) - 1; _idx++)
				_checksum += _u_ret.byArr[_idx];
			_u_ret.s_ret.checksum = 0xff - _checksum;
			Wire.write( (const byte*)_u_ret.byArr, sizeof(_S_Ret) );
			return;

		case _CMD_READ_ID:
			Wire.write( (const byte*)_u_id.byArr, sizeof(_S_Id) );
			return;
			
        // 21/Oct/2016 추가 ret데이터도 원본비교
        case _CMD_CHECK_RET:
            return;
            
		default:
			Wire.write(255);
			return;
	} 
}

_HRP_   Ardpy;//외부에서 사용할 객체를 생성한다.
