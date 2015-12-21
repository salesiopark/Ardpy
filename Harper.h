/***********************************************************************
* Harper.h ver 1.0
* Arduino library for i2c (slave) communication with Raspberry pi.
* by salesioPark(박장현, 국립목포대학교, 전기제어공학과)
* Note: <Wire.h> must be included before including <Harper.h>
************************************************************************/
#ifndef __Harper_h__
#define __Harper_h__

#include "Arduino.h"

typedef void(*pfunc_t)(void);

#define __INIT_MAX_ARG_NUM__ 		4
#define __STR_BUF_LENGTH__			32
#define __MAX_I2C_READ_BUF_LEN__	32

class _HRP_ {

	public:

		static byte 		begin(byte addr, uint32_t dev_id = 0xffffffff);
		static void 		check();
		static void 		add_func( void (*func)(void) );
  
		static int8_t		get_int8(byte index=0);
		static uint8_t		get_uint8(byte index=0);
		static int16_t		get_int16(byte index=0);
		static uint16_t	get_uint16(byte index = 0);
		static int32_t		get_int32(byte index = 0);
		static uint32_t	get_uint32(byte index = 0);
		static float		get_float(byte index = 0);
		static char*		get_str(byte index = 0);

		static uint8_t		get_byte(byte index=0); // same as get_uint8() (unsigned)
		static int16_t		get_int(byte index=0);	// same as get_int16() (signed)

		static void 		set_ret(int8_t);
		static void 		set_ret(uint8_t); //static void setReturn(bool);
		static void 		set_ret(int16_t);
		static void			set_ret(uint16_t);
		static void			set_ret(int32_t);
		static void 		set_ret(uint32_t);
		static void 		set_ret(float);

		static void			set_max_arg_num(byte num);

   private:

		enum _E_IN { // invariable numbers i.e. constants
			__RET_DATA_LENGTH__	= 7,
			__EEPROM_ADDR__		= 1, // eeprom addr for storing i2c addr
			__MIN_I2C_ADDR__		= 3,
			__MAX_I2C_ADDR__		= 119,
		};

		enum _E_DTYPE {
			_DTYPE_NONE		= 0,
			//_DTYPE_CHAR		= 1,
			_DTYPE_INT8		= 2, //_DATA_SBYTE
			_DTYPE_UINT8	= 3, //_DATA_BYTE
			//_DTYPE_BOOL = 4,
			_DTYPE_INT16	= 5,
			_DTYPE_UINT16	= 6, //_DATA_USHORT
			_DTYPE_INT32	= 7, //_DATA_LONG
			_DTYPE_UINT32	= 8, //_DATA_ULONG
			_DTYPE_FLOAT	= 9, //_DATA_FLOAT
			//_DTYPE_DOUBLE	= 10,
			_DTYPE_STR		= 11, //_DATA_STR
		};

		enum _E_STATUS {
			_STAT_UNDER_NORMAL_PROC	= 0,
			STAT_CMD_COMPLETED		= 1,
			_STAT_ERR_DATA_RCVD		= 2, // wrong data received
			_STAT_ERR_FUNC_INDEX		= 3, // function index out of bound
			//_STAT_ERR_ARG_INDEX		= 4, // argument index out of bound
			STAT_ERR_NO_ARG			= 5,
			STAT_ERR_ARG_TYPE			= 6,
			//_STAT_ERR_DATA_CSUM		= 7, // wrong data received

			_STAT_ERR_DATA_100		= 100, // wrong data received
			_STAT_ERR_DATA_101		= 101, // wrong data received
			_STAT_ERR_DATA_102		= 102, // wrong data received

		};

		enum _E_COMMAND {
			_CMD_READ_DATA 		= 0,
			_CMD_READ_ID 		= 1, //<- //#define CMD_IS_READY					2
			_CMD_CHANGE_ADDR		= 2,
			_CMD_SEND_DATA 		= 3,
			_CMD_EXEC_FUNC		= 4,
			_CMD_READ_STAT			= 5,
			_CMD_CHECK_OK 			=6, //_CMD_READ_STAT_WRITE	= 6,
			_CMD_SEND_BACK		= 7,
		};

		enum _E_RET_DATA_IDX { //byte index of data in _u_ret
			RET_BYTE0 = 1,
			RET_BYTE1 = 2,
			RET_BYTE2 = 3,
			RET_BYTE3 = 4,
		};

		enum _E_WRT {
			_WRT_FAIL = 0,
			_WRT_SUCCESS = 1,

			_WRT_FAIL2 = 2,
			_WRT_FAIL3 = 3,
		};

		union _U_Short	{
			volatile int16_t	val;
			volatile byte byArr[2];
		};

		union _U_UShort {
		  volatile uint16_t	val;
		  volatile byte byArr[2];
		};

		union _U_Long {
		  volatile int32_t	val;
		  volatile byte byArr[4];
		};

		union _U_ULong {
		  volatile uint32_t	val;
		  volatile byte byArr[4];
		};

		union _U_Float {
			volatile float val;
			volatile byte byArr[4];
		};

		union _U_Data {
			volatile int8_t		sbVal;
			volatile uint8_t		ucVal;
			volatile _U_Short		u_s;
			volatile _U_UShort	u_us;
			volatile _U_Long		u_l;
			volatile _U_ULong		u_ul;
			volatile _U_Float		u_f;
			char*			str;
		};

		struct _S_Arg { // structure for function argument
			volatile byte			type;
			volatile _U_Data		dat;
		};

		// data structures for the return data to the master ---------
		struct _S_Ret {
			//volatile byte			stat;
			volatile byte			type;
			volatile _U_Data		dat;
			volatile byte 			info;
			volatile byte			checksum;
		};

		union _U_Ret {
			volatile _S_Ret		s_ret;
			volatile byte				byArr [ __RET_DATA_LENGTH__ ];
		};

		// data structure for the device id (ulong) -------------------
		struct _S_Id {
			uint32_t		val;
			byte			numArgs;	// maximnum arg num
			byte			numFuncs;// number of functions
			//uint16_t		numInfo; // number of info characters
			byte 			checksum;
		};

		union _U_Id {
			_S_Id 	s_id;
			byte 		byArr[7];
		};

		volatile static byte		_stat;
		//volatile static byte 	_stat_write;
		volatile static byte		_statArr[2];
		volatile static byte		_cmd; // command from master device
		volatile static byte		_cmd_i2c; // command from master device
		volatile static byte		_rcvBuf[ __MAX_I2C_READ_BUF_LEN__ ];

		static pfunc_t*			_tmpFuncArr;
		static pfunc_t*			_funcArr; //__funcs_i2c__;
		static byte					_num_funcs;

		static _U_Id				_u_id;

		static void _onRequest();
		static void _onReceive(int);

		volatile static _U_Ret	_u_ret;
		volatile static _S_Arg*	_args;

		static char 			_strBuf[ __STR_BUF_LENGTH__ ];
		static void 			_reset_all_args();

		//tmp varialbes
		static byte _checksum;
		static byte _idx;

		static void _check_if_no_err_to_get_arg(byte, byte);

		static byte _max_arg_num;
		volatile static byte _len_just_rcvd;// = 0;

}; // closing of *class _HRP_*

// define externally used object name as Harper
extern _HRP_ Harper;

#endif
