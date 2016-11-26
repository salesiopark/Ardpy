class Ardpy:
    # version
    _VERSION = '1.1.0'
    
    # command constant to arduino
    __CMD_READ_DATA   = 0
    __CMD_READ_ID     = 1
    __CMD_CHANGE_ADDR = 2
    __CMD_SEND_ARG    = 3
    __CMD_EXEC_FUNC   = 4
    __CMD_READ_STAT   = 5
    __CMD_CHECK_OK    = 6 #CMD_READ_STAT_WRITE	= 6
    __CMD_SEND_BACK   = 7
    
    __CMD_CHECK_RET   =10 # 21/OCT/2016 추가

    # InDeX of ret data
    __IDX_RET_TYPE		= 0 # index of received data list
    __IDX_RET_DATA_STRT	= 1 # index of received data list
    __IDX_RET_INFO 		= 5 # index of received data list
 
	# Data Types
    __DT_NONE   = 0
    #__DT_CHAR	= 1
    __DT_SBYTE	= 2
    __DT_BYTE   = 3
    __DT_SHORT	= 5
    __DT_USHORT	= 6
    __DT_LONG   = 7
    __DT_ULONG	= 8
    __DT_FLOAT	= 9
    #__DT_DOUBLE= 10
    __DT_STR    = 11
	
    # 슬레이브의 상태코드들
    #__STAT_ERR_DATA          = 2 # wrong data received in arduino
    __STAT_UNDER_NORMAL_PROC  = 0 # 처리중
    __STAT_CMD_COMPLETED      = 1 # 처리(정상)완료
    __STAT_ERR_NO_ARG         = 5 # 에러:있어야할 arg가 없음
    __STAT_ERR_ARG_TYPE       = 6 # 에러:arg의 type 불일치

    #__EXE_ERR_RETRY = 3 # maximum retry no when exec_func fails
    __WAIT_I2C_COUNT  = 5 #
    __WAIT_COUNT    = 20 #maximum trial number for i2c communication 
    __TIMEOUT		= 2.0 #sec.
    __MAX_RETRY_CNT	= 5 # maximum retry number when data corruptted
    __RET_DATA_LEN	= 7

    # internal modules and functions =================================
    import smbus as __smbus
    import struct as __struct
    import datetime as __datetime

    """
    constructor
    """
    def __init__(self, addr, port=1, _showErr=False):
        self.__addr = addr
        self.__showErr = _showErr
        self.__i2c = self.__smbus.SMBus(port)
        self.__read_id_info()
        self.__reset_args()
        print('Ardpy device with address 0x%x and id:%d ready.'%(addr, self.__id))
        print('Number of functions:%d, max number of function args:%d '%(self.__num_funcs, self.__max_arg_num))
        print('Firmware ver. %s ("Ardpy.h" ver. %s)'%(self.__str_ver_firmw, self.__str_ver_Ardpy))
        
    @property
    def device_id(self):
        '''
        The original id number (read-only) of the device
        '''
        return self.__id

    @property
    def num_funcs(self):
        return self.__num_funcs

    @property
    def max_arg_num(self):
        return self.__max_arg_num

    @property
    def addr(self):
        return self.__addr

    @addr.setter
    def addr(self, addr):
        if (addr < 3 or addr >119):
            raise Exception('i2c address is out of bound.')
        ans = input('This will change the i2c address of the device. proceed? [y/n]')
        if ans!='y': return
        #byte_list = [self.__CMD_CHANGE_ADDR, addr] 
        byte_list = [addr] 
        self.__write_i2c_cmd(self.__CMD_CHANGE_ADDR, byte_list)
        self.__wait_until_cmd_handled()
        #self.__addr = addr
        print('i2c address of the device has changed to 0x%x.'%addr)
        print('The device MUST BE RESET.')

    """====================================================================
        슬레이브에 보낼 데이터 패킷을 구성한 후 __lst_args 리스트에 저장하는 함수들
        패킷 구성: [ (__CMD_SEND_ARG,) arg_index, data_type, data0, data1, ... ]    
        실제로 보내는 것은 *함수를 실행하는 시점*에서 한다.
        이렇게 한 이유는 만약 함수를 실행하는 시점에서 오류가 발생한다면
        처음부터(즉, 인자를 보내는 것 부터) 다시 수행할 수 있도록 하기 위함이다.
       ====================================================================
    """
    # 03/May/2016 replaced _send... funcs into the folllowing one func
    def _set_arg(self, val, index=0, type='byte'):
        """
        This method sets an input argument of the Arduino(using Ardpy lib) function.
        The type is one of followings:
            'int8'
            'uint8' or 'byte' (default value)
            'int16' or 'int'
            'uint16'
            'int32'
            'uint32'
            'float'
            'str'
        Note that in case of 'str', the maxium length of string is 27.
        """
        if type == 'int8':
            lst_data = list(self.__struct.pack('b', val))
            self.__reg_arg(index, self.__DT_SBYTE, lst_data)
        elif type == 'uint8' or type == 'byte':
            lst_data = list(self.__struct.pack('B', val)) 
            self.__reg_arg(index, self.__DT_BYTE, lst_data)
        elif type =='int16' or type == 'int' :
            lst_data =  list(self.__struct.pack('h', val)) 
            self.__reg_arg(index, self.__DT_SHORT, lst_data)
        elif type == 'uint16':
            lst_data = list(self.__struct.pack('H', val))
            self.__reg_arg(index, self.__DT_USHORT, lst_data)
        elif type == 'int32':
            lst_data =  list(self.__struct.pack('l', val))
            self.__reg_arg(index, self.__DT_LONG, lst_data)
        elif type == 'uint32':
            lst_data =  list(self.__struct.pack('L', val))
            self.__reg_arg(index, self.__DT_ULONG, lst_data)
        elif type == 'float':
            lst_data =  list(self.__struct.pack('f', fval)) 
            self.__reg_arg(index, self.__DT_FLOAT, lst_data)
        elif type == 'str':
            """
            Transmit string to the Ardpy (arduino) device.
            Up to 27 ASCII characters can be transmitted.
            """
            lst_data = list( val.encode('utf-8') ) 
            lst_data.append(0) # append null character in the end
            self.__reg_arg(index, self.__DT_STR, lst_data)
        else:
            raise Exception('Unknown arg type.')

    """====================================================================
        아두이노의 함수를 실행하는 메서드
        인자들 전송 -> 함수 실행 명령 전송 -> 함수 실행 종료 대기 -> 리턴값 반환
        만약 오류가 발생하면 인자를 전송하는 것 부터 다시 실행한다.
        반복적으로 오류가 발생하면 예외를 발생시킨다.
       ====================================================================
    """

    def _exec_func(self, index):
        """
        This method executes function in the Ardpy(arduino) device.
        If there are input arguments, they are must be set prior using _set_arg().
        For example, arduino (using Ardpy lib) function with one byte arguent,
            
            obj._set_arg(byte_val)
            obj._exec_func(0)

        If two arguments are requried,

            obj._set_arg(byte_val0)
            obj._set_arg(byte_val1, index = 1)
            obj._exec_func(0)

        As shown in these examples,
        input arguments are must be set
        before calling _exec_func() method.
        """
        if index >= self.__num_funcs:
            raise Exception('Index of func out of bound (dev:0x%x)'%self.__addr)
        retryCount = 0
        successExec = False

        while not successExec:
            # transmit all the pre-stored function arguments
            # after that, send func execution command
            # wait until func exec (slave) finishied
            # and then, read return data
            self.__send_args() 
            self.__write_i2c_cmd(self.__CMD_EXEC_FUNC, [index]) 
            stat = self.__wait_until_cmd_handled() 
            dtype, lst, info = self.__read_ret() # 리턴 데이터를 받는다

            # 함수 실행이 성공한 경우
            if stat == self.__STAT_CMD_COMPLETED: 
                self.__reset_args() # 모든 (함수)인자를 제거한다.
                #struc.unpact()함수가 튜플을 반환하기 때문에
                # 첫 번째 요소만 빼낸 다음 그것을 반환한다. (그래서 끝에 [0]이 붙은 것임)
                if dtype == self.__DT_BYTE:
                    return ( self.__struct.unpack('B', bytes([lst[0]])) )[0]
                elif dtype == self.__DT_SBYTE:
                    return ( self.__struct.unpack('b', bytes([ lst[0]])) )[0]
                elif dtype == self.__DT_USHORT:
                    return ( self.__struct.unpack('H', bytes(lst[0:2])) )[0]
                elif dtype == self.__DT_SHORT:
                    return ( self.__struct.unpack('h', bytes(lst[0:2])) )[0]
                elif dtype == self.__DT_LONG:
                    return ( self.__struct.unpack('l', bytes( lst )) )[0]
                elif dtype == self.__DT_ULONG:
                    return ( self.__struct.unpack('L', bytes( lst ) ) )[0]
                elif dtype == self.__DT_FLOAT:
                    return ( self.__struct.unpack('f', bytes(lst)) )[0]
                else: return None #self.DT_NONE

            # 함수 실행이 실패한 경우 정해진 횟수 만큼 다시 실행을 반복한 후
            # 여전히 에러가 발행하면 오류를 발생시킨다.
            else: 
                retryCount +=1
                if retryCount > self.__MAX_RETRY_CNT:
                    if stat == self.__STAT_ERR_NO_ARG: #5
                        #dtype, ret, info = self.__read_data()
                        raise Exception('Missing arg %d for func %d. (dev:0x%x)'%(info,ret[0],self.__addr))
                    elif stat == self.__STAT_ERR_ARG_TYPE: #6
                        #dtype, ret, info = self.__read_data()
                        raise Exception('Type of arg %d for func %d mismatch. (dev:0x%x)'%(info,ret[0],self.__addr))
                    else:
                        #dtype, ret, info = self.__read_data()
                        #print('info:%d'%info)
                        raise Exception('Unknown error(%d) occurred.(dev:0x%x)'%(stat, self.__addr))
    
    '''=================================================================
    (internal) functions from here
    ================================================================='''

    def __reg_arg(self, index, dtype, lst_data):
        if index >= self.__max_arg_num:
            raise Exception('argument index must be lower than or equal to %d.'%self.__max_arg_num)
        lst_pckt = [index, dtype]
        lst_pckt.extend(lst_data)
        self.__lst_args[index] = lst_pckt

    def __reset_args(self):
        """reset all the function argument(list) as None"""
        self.__lst_args = []
        for k in range(self.__max_arg_num):
            self.__lst_args.append(None)

    def __read_id_info(self):
        """read id, max_arg_num and num_funcs from Ardpy device"""
        info = self.__read_i2c_data(cmd=self.__CMD_READ_ID, length = 9)
        self.__id = self.__struct.unpack('H', bytes(info[0:2]))[0]
        self.__max_arg_num = info[2]
        self.__num_funcs = info[3]
        #18/Nov/2016 Ardpy의 버전을 받아오도록 했다.
        #20/Nov/2016 firmware의 버젼도 받아오도록 했다.
        self.__ver_apy = (self.__struct.unpack('H', bytes(info[4:6])))[0]
        self.__ver_firmw = (self.__struct.unpack('H', bytes(info[6:8])))[0]
        self.__str_ver_Ardpy = "%d.%d.%d"%self.__decode_ver(self.__ver_apy)
        self.__str_ver_firmw = "%d.%d.%d"%self.__decode_ver(self.__ver_firmw)

    # slave에서 넘어온 16비트 버전번호를 a,b,c로 변환하는 함수
    def __decode_ver(self, raw_num):
        return (
            int(raw_num/(2**12)),
            int((raw_num%(2**12))/(2**6)),
            int(raw_num%(2**12)%(2**6)),
        )
    """====================================================================
        __lst_args 리스트에 저장한 인자를 하나씩 꺼내서 전송한다.
       ====================================================================
    """
    def __send_args(self):
        for arg_pckt in self.__lst_args:
            retryCount = 0
            if arg_pckt != None:
                SuccessToSendArg = False
                while not SuccessToSendArg :
                    self.__write_i2c_cmd(self.__CMD_SEND_ARG, arg_pckt)
                    stat = self.__wait_until_cmd_handled()
                    if stat == self.__STAT_CMD_COMPLETED:
                        SuccessToSendArg = True
                    else:
                        retryCount += 1
                        if retryCount > self.__MAX_RETRY_CNT:
                            #type, ret, info = self.__read_ret()
                            #print('cmd:%d'%info)
                            raise Exception('Unknown err(%d) in sending arg (dev:0x%x)'%(stat,self.__addr))

    """====================================================================
        슬레이브가 보낸 명령을 처리하기를 기다린다.
        명령을 전송하면 STAT_UNDER_NORMAL_PROC 가 되었다가 처리가 끝나면 다른 값으로 바뀐다.
        즉, 아직 처리중이라면 STAT_UNDER_NORMAL_PROC 가 읽힌다.
        따라서 STAT_UNDER_NORMAL_PROC 가 안 읽힐 때까지 기다린다.
        정상이라면 :
        오류발새이라면:
       ====================================================================
    """
    def __wait_until_cmd_handled(self):
        tmStart = self.__datetime.datetime.now()
        cmd_completed = False
        while not cmd_completed:
            tmElapsed = self.__datetime.datetime.now() - tmStart
            if (tmElapsed.total_seconds() > self.__TIMEOUT):
                raise Exception('Timeout for waiting device ready.')
            #stat = self.__read_stat()
            #cmd_completed = ( stat != self.__STAT_UNDER_NORMAL_PROC )
            stat = self.__read_i2c_data(
                cmd = self.__CMD_READ_STAT,
                length =2   # [stat, checksum]
            )[0]            # 첫 번째 요소만 추출
            cmd_completed = ( stat != self.__STAT_UNDER_NORMAL_PROC )
        return stat

    """====================================================================
        함수의 실행 반환값을 아두이노에서 읽어오는 기능을 함.
       ====================================================================
    """
    def __read_ret(self):
        res = self.__read_i2c_data(cmd=self.__CMD_READ_DATA, length = self.__RET_DATA_LEN)
        return (res[self.__IDX_RET_TYPE],
                res[self.__IDX_RET_DATA_STRT:(self.__IDX_RET_DATA_STRT+4)],
                res[self.__IDX_RET_INFO],)		

    """====================================================================
        lst_bytes 리스트의 요소들로 부터 체크섬을 계산하는 함수
        cmd 까지 고려해서 체크섬을 계산한다.
       ====================================================================
    """
    '''
    def __checksum(self, lst_bytes, cmd=0):#motorola checksum
        cnt = len(lst_bytes)+1
        total_sum = sum(lst_bytes) + cmd
        lb_sum = total_sum.to_bytes(cnt, 'little')[0] #lowest byte
        return 0xff - lb_sum
    '''
    #22/Nov/2016 checksum 계산에서 cmd를 제거한다.
    def __checksum(self, lst_bytes):#motorola checksum
        cnt = len(lst_bytes)
        total_sum = sum(lst_bytes)
        lb_sum = total_sum.to_bytes(cnt, 'little')[0] #lowest byte
        return 0xff - lb_sum

    """====================================================================
        __read_라고 되어있지만 사실 슬레이브에 데이터가 정상이라고 confirm 하는 기능을 함.
        슬레이브에서는 아래와 같이 _stat 변수를 _STAT_UNDER_NORMAL_PROC 로 저장하고
        바로 그 _stat변수값을 체크썸과 같이 넘겨주게 되어 있음
        ----------------------------------------------------------
		case _CMD_CHECK_OK:
			_stat = _STAT_UNDER_NORMAL_PROC;
			_statArr[0] = _stat;
			_statArr[1]	=	0xff-(_CMD_CHECK_OK + _stat);
			Wire.write( (const byte*)_statArr, 2);
			break;
       ====================================================================
    """
    def __noti_ok(self):
        notiSuccess = False     # 그리고 OK신호를 보낸다.
        tryCount = 0
        while not notiSuccess:
            res = self.__read_i2c_data(cmd = self.__CMD_CHECK_OK, length =2)
            if res[0] == self.__STAT_UNDER_NORMAL_PROC:
                notiSuccess = True
            else:
                tryCount += 1
                if self.__showErr:
                    print( 'i2c (write) notifying OK error', tryCount)
                if (tryCount >= self.__WAIT_COUNT):
                    raise Exception( 'i2c write noti_OK error.' )

    """====================================================================
        아두이노에 명령을 전송하는 함수
        보낸 데이터를 그대로 다시 받아본 후 보낸 데이터와 일치하면
        아두이노에 CHECK_OK 신호를 보내고 아두이노는 CHECK_OK신호를 받아야 받은 명령을
        처리하는 구조로 되어 있어서 데이커가 손실없이 전송되었는지를 100% 보장할 수 있음.
       ====================================================================
    """
    def __write_i2c_cmd(self, cmd, byte_list):
        orgn_data = [cmd] + byte_list #[cmd, byte_list]로 하나의 리스트로 조합
        writeSuccess = False
        tryCount = 0
        while not writeSuccess:
            self.__raw_i2c_write(cmd, byte_list)
            # 보냈던 데이터를 그대로 다시 받는다
            sent_back = self.__read_i2c_data(
                self.__CMD_SEND_BACK,
                len(byte_list)+2, # cmd, checksum까지 포함한 길이
            )
            #print('back_data << lst:%s'%sent_back)
            if sent_back == orgn_data:  # 보낸 것과 받은 것이 같은 경우에만
                writeSuccess = True     # 성공한 것으로 판단하고 빠져나간다.
            else:
                tryCount += 1
                if self.__showErr:
                    print('%d: sent:%s, back:%s'%(tryCount, orgn_data, sent_back))
                if (tryCount >= self.__WAIT_COUNT):
                    raise Exception( 'i2c data confirm error.' )
        self.__noti_ok()

    """====================================================================
        아두이노에서 데이터를 읽어온다.
        아두이노에서 체크썸을 끝에다가 붙여서 오기 때문에 그것으로 데이터가 정상인지 판단한다.
    20/Nov/2016:읽어오는 모든 경우에 checksum을 사용한다.
    ====================================================================
    """
    def __read_i2c_data(self, cmd, length):
        readSuccess = False
        tryCount = 0
        while not readSuccess:
            tryCount += 1
            res = self.__raw_i2c_read(cmd, length)
            #res리스트의 마지막 요소(res[-1])가 slave에서 계산된 checksum
            # 계산된 체크섬과 일치하면 성공으로 간주
            readSuccess = ( res[-1] == self.__checksum(res[:-1]) )
            #print('rd_data (cmd:%d) << res:%s, csum:%s, cnt:%d'%(cmd, res, csum, tryCount))
            
            if self.__showErr and not readSuccess:
                print('i2c (read) checksum error:', tryCount)
                
            #(통신은 이루어졌으나) 체크썸이 정해진 횟수 이상 불일치하면 예외 발생
            if (tryCount >= self.__WAIT_COUNT):
                raise Exception('i2c read checksum error.')
        # 20/NOV/2016 checksum은 빼고 전송한다.
        return res[:-1]
    
    """===========================================================
    smbus 객체를 직접 접근하는 가장 하위의 통신 함수들
        여기에서 예외가 발생하면 주소나 *하드웨어가 잘못 되었을 가능성*이 크다.
        정해진 횟수만큼 기다린 다음 계속 오류가 나면 예회를 발생하고 해당 문구를 출력한다.
    ===========================================================
        아래 문구는 hardware 오류가 났을 때 체크해볼 것들을 보여주는 것이다.
    """
    __i2c_err_msg = """Check the followings:
    1. The interface circuit is correctily wired.
    2. The i2c address is correct.
    3. Arduino(i2c slave) is properly working."""

    # smbus함수에서 예외가 한 번 발생했다하더라도 몇 번 더 시도해 보아야 한다.
    # 즉, 예외 한 번에 멈추면 (드물게 발생하지만) 실행이 도중에 멈추는 경우가 있다.
    # 한 번 예외가 발생했다고 연결이 완전히 끊긴 것이 아닐 수 있다는 얘기다.
    # 따라서 반드시 아래와 같이 몇 번 더 시도해 보고 계속 예외가 발생하면 그때 멈춘다.
    
    def __raw_i2c_read(self, cmd, length):
        readSuccess = False
        tryCount = 0
        res = None
        while not readSuccess:
            try:
                res = self.__i2c.read_i2c_block_data(self.__addr, cmd, length)
                readSuccess = True
            except IOError: # hardware error
                tryCount += 1
                if self.__showErr:
                    print('i2c raw read error')
                if (tryCount >= self.__WAIT_I2C_COUNT):
                    raise Exception( 'i2c read error. '+self.__i2c_err_msg )
        return res
    
    def __raw_i2c_write(self, cmd, byte_list):
        writeSuccess = False
        tryCount = 0
        while not writeSuccess:
            try: 
                self.__i2c.write_i2c_block_data(self.__addr, cmd, byte_list)
                writeSuccess = True
            except IOError:
                tryCount += 1
                if self.__showErr:
                    print('i2c raw write error')
                if (tryCount >= self.__WAIT_I2C_COUNT):
                    raise Exception( 'i2c write error. '+self.__i2c_err_msg )
