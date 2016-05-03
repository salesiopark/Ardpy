class Ard:

    CMD_READ_DATA   = 0
    CMD_READ_ID     = 1
    CMD_CHANGE_ADDR = 2
    CMD_SEND_DATA   = 3
    CMD_EXEC_FUNC   = 4
    CMD_READ_STAT   = 5
    CMD_CHECK_OK    = 6 #CMD_READ_STAT_WRITE	= 6
    CMD_SEND_BACK   = 7

    # InDeX of ret data
    IDX_RET_TYPE		= 0 # index of received data list
    IDX_RET_DATA_STRT	= 1 # index of received data list
    IDX_RET_INFO 		= 5
 
	# Data Types
    DT_NONE		= 0
    DT_CHAR		= 1
    DT_SBYTE	= 2
    DT_BYTE		= 3
    DT_SHORT	= 5
    DT_USHORT	= 6
    DT_LONG		= 7
    DT_ULONG	= 8
    DT_FLOAT	= 9
    DT_DOUBLE	= 10
    DT_STR		= 11
	
    # dev status error code
    STAT_UNDER_NORMAL_PROC  = 0
    STAT_CMD_COMPLETED      = 1
    #STAT_ERR_DATA          = 2 # wrong data received in arduino
    STAT_ERR_NO_ARG         = 5
    STAT_ERR_ARG_TYPE       = 6

    WRITE_SUCCESS   = 1
    #WRITE_FAIL     = 0

    EXE_ERR_RETRY	= 3 # maximum retry no when exec_func fails
    WAIT_COUNT 		= 20 #maximum trial number for i2c communication 
    TIMEOUT			= 2.0 #sec.
    MAX_RETRY_CNT	= 5 # maximum retry number when data corruptted
    RET_DATA_LEN	= 7

    def __init__(self, addr, port = 1):
        self.__addr = addr
        self.__i2c = Ard.__smbus.SMBus(port)
        self.__read_id_info()
        self.__reset_args()
        print('pyard device (addr:0x%x, id:%d) ready.'%(addr,self.__id))

    @property
    def device_id(self):
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
        ans = input('This will change the i2c addr of the device. proceed? [y/n]')
        if ans!='y': return
        byte_list = [self.CMD_CHANGE_ADDR, addr] 
        self.__write_i2c_cmd(byte_list, self.CMD_CHANGE_ADDR)
        self.__wait_until_cmd_handled()
        print('Address of the device has changed to 0x%x.'%self.addr)
        print('The device must be reset to use new address.')

    """====================================================================
        슬레이브에 보낼 데이터 패킷을 구성한 후 __lst_args 리스트에 저장하는 함수들
        패킷 구성: [CMD_SEND_DATA, arg_index, data_type, data0, data1, ... ]    
        실제로 보내는 것은 *함수를 실행하는 시점*에서 한다.
        이렇게 한 이유는 만약 함수를 실행하는 시점에서 오류가 발생한다면
        처음부터(즉, 인자를 보내는 것 부터) 다시 수행할 수 있도록 하기 위함이다.
       ====================================================================
    """
    def __reg_arg(self, index, dtype, lst_data):
        if index >= self.__max_arg_num:
            raise Exception('Arg index must be under %d.'%self.__max_arg_num)
        lst_pckt = [self.CMD_SEND_DATA, index, dtype ] 
        lst_pckt.extend(lst_data)
        self.__lst_args[index] = lst_pckt
        #print('lst_pckt:%s'%lst_pckt)

    def _send_int8(self, val, index = 0):
        lst_data = list(Ard.__struct.pack('b', val))
        self.__reg_arg(index, self.DT_SBYTE, lst_data)

    def _send_uint8(self, val, index = 0):
        lst_data = list(Ard.__struct.pack('B', val)) 
        self.__reg_arg(index, self.DT_BYTE, lst_data)

    def _send_byte(self, val, index = 0):
        lst_data = list(Ard.__struct.pack('B', val)) 
        self.__reg_arg(index, self.DT_BYTE, lst_data)

    def _send_int16(self, val, index = 0):
        lst_data =  list(Ard.__struct.pack('h', val)) 
        self.__reg_arg(index, self.DT_SHORT, lst_data)

    def _send_int(self, val, index = 0):
        lst_data =  list(Ard.__struct.pack('h', val)) 
        self.__reg_arg(index, self.DT_SHORT, lst_data)

    def _send_uint16(self, val, index = 0):
        lst_data = list(Ard.__struct.pack('H', val))
        self.__reg_arg(index, self.DT_USHORT, lst_data)

    def _send_int32(self, val, index = 0):
        lst_data =  list(Ard.__struct.pack('l', val))
        self.__reg_arg(index, self.DT_LONG, lst_data)

    def _send_uint32(self, val, index = 0):
        lst_data =  list(Ard.__struct.pack('L', val))
        self.__reg_arg(index, self.DT_ULONG, lst_data)

    def _send_float(self, fval, index = 0):
        lst_data =  list(Ard.__struct.pack('f', fval)) 
        self.__reg_arg(index, self.DT_FLOAT, lst_data)

    def _send_str(self, string, index = 0):
        """
        Transmit string to the pyard (arduino) device.
        Up to 27 ASCII characters can be transmitted.
        """
        lst_data = list( string.encode('utf-8') ) 
        lst_data.append(0) # append null character in the end
        self.__reg_arg(index, self.DT_STR, lst_data)

    """====================================================================
        아두이노의 함수를 실행하는 메서드
        인자들 전송 -> 함수 실행 명령 전송 -> 함수 실행 종료 대기 -> 리턴값 반환
        만약 오류가 발생하면 인자를 전송하는 것 부터 다시 실행한다.
        반복적으로 오류가 발생하면 예외를 발생시킨다.
       ====================================================================
    """

    def _exec_func(self, index):
        """execute function in the pyard (arduino) device"""
        if index >= self.__num_funcs:
            raise Exception('Index of func out of bound (dev:0x%x)'%self.__addr)

        retryCount = 0
        successExec = False

        while not successExec:
            # transmit pre-stored function arguments
            # and then, send func execution command
            # wait until func finishied
            self.__send_args() 
            send_pckt = [self.CMD_EXEC_FUNC, index] 
            self.__write_i2c_cmd(send_pckt, self.CMD_EXEC_FUNC) 
            stat = self.__wait_until_cmd_handled() 

            # 함수 실행이 성공한 경우
            if stat == self.STAT_CMD_COMPLETED: 
                dtype, lst, info = self.__read_data() # 리턴 데이터를 받는다
                self.__reset_args() # 모든 (함수)인자를 제거한다.

                #struc.unpact()함수가 튜플을 반환하기 때문에
                # 첫 번째 요소만 빼낸 다음 그것을 반환한다. (그래서 끝에 [0]이 붙은 것임)

                if dtype == self.DT_BYTE:
                    return ( Ard.__struct.unpack('B', bytes([lst[0]])) )[0]
                elif dtype == self.DT_SBYTE:
                    return ( Ard.__struct.unpack('b', bytes([ lst[0]])) )[0]
                elif dtype == self.DT_USHORT:
                    return ( Ard.__struct.unpack('H', bytes(lst[0:2])) )[0]
                elif dtype == self.DT_SHORT:
                    return ( Ard.__struct.unpack('h', bytes(lst[0:2])) )[0]
                elif dtype == self.DT_LONG:
                    return ( Ard.__struct.unpack('l', bytes( lst )) )[0]
                elif dtype == self.DT_ULONG:
                    return ( Ard.__struct.unpack('L', bytes( lst ) ) )[0]
                elif dtype == self.DT_FLOAT:
                    return ( Ard.__struct.unpack('f', bytes(lst)) )[0]
                else : return None #self.DT_NONE

            # 함수 실행이 실패한 경우 정해진 횟수 만큼 다시 실행을 반복한 후
            # 여전히 에러가 발행하면 오류를 발생시킨다.
            else: 
                retryCount +=1
                if retryCount > self.MAX_RETRY_CNT:

                    if stat == self.STAT_ERR_NO_ARG: #5
                        dtype, ret, info = self.__read_data()
                        raise Exception('Missing arg %d for func %d. (dev:0x%x)'%(info,ret[0],self.__addr))
                    elif stat == self.STAT_ERR_ARG_TYPE: #6
                        dtype, ret, info = self.__read_data()
                        raise Exception('Type of arg %d for func %d mismatch. (dev:0x%x)'%(info,ret[0],self.__addr))
                    else:
                        dtype, ret, info = self.__read_data()
                        #print('info:%d'%info)
                        raise Exception('Unknown error(%d) occurred.(dev:0x%x)'%(stat, self.__addr))

    # internal modules and functions =================================
    import smbus as __smbus
    import struct as __struct
    import datetime as __datetime

    def __reset_args(self):
        """reset all the function argument(list) as None"""
        self.__lst_args = []
        for k in range(self.__max_arg_num):
            self.__lst_args.append(None)

    def __read_id_info(self):
        """read id, max_arg_num and num_funcs from pyard device"""
        info = self.__read_i2c_data(cmd=self.CMD_READ_ID, length = 7)
        self.__id = Ard.__struct.unpack('L', bytes(info[0:4]))[0]
        self.__max_arg_num = info[4]
        self.__num_funcs = info[5]

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
                    self.__write_i2c_cmd(arg_pckt, self.CMD_SEND_DATA)
                    stat = self.__wait_until_cmd_handled()
                    if stat == self.STAT_CMD_COMPLETED:
                        SuccessToSendArg = True
                    else:
                        retryCount += 1
                        if retryCount > self.MAX_RETRY_CNT:
                            type, ret, info = self.__read_data()
                            #print('cmd:%d'%info)
                            raise Exception('Unknown err(%d) in sending arg (dev:0x%x)'%(stat,self.__addr))

    """====================================================================
        슬레이브가 보낸 명령을 처리하기를 기다린다.
        명령을 전송하면 STAT_UNDER_NORMAL_PROC 가 되었다가 처리가 끝나면 다른 값으로 바뀐다.
        즉, 아직 처리중이라면 STAT_UNDER_NORMAL_PROC 가 읽힌다.
        따라서 STAT_UNDER_NORMAL_PROC 가 안 읽힐 때까지 기다린다.
       ====================================================================
    """

    def __wait_until_cmd_handled(self):
        tmStart = Ard.__datetime.datetime.now()
        cmd_completed = False
        while not cmd_completed:
            tmElapsed = Ard.__datetime.datetime.now() - tmStart
            if (tmElapsed.total_seconds() > self.TIMEOUT):
                raise Exception('timeout for waiting device ready.')
            stat = self.__read_stat()
            cmd_completed = ( stat != self.STAT_UNDER_NORMAL_PROC )
        return stat

    """====================================================================
        슬레이브에서 stat 값을 읽는다.
       ====================================================================
    """

    def __read_stat(self):
        res = self.__read_i2c_data(cmd = self.CMD_READ_STAT, length =2)
        return res[0]

    """====================================================================
        __read__라고 되어있지만 사실 슬레이브에 데이터가 정상이라고 confirm 하는 기능을 함.
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

    def __read_confirm_ok(self):
        res = self.__read_i2c_data(cmd = self.CMD_CHECK_OK, length =2)
        return res[0]

    """====================================================================
        함수의 실행 반환값을 아두이노에서 읽어오는 기능을 함.
       ====================================================================
    """

    def __read_data(self):
        res = self.__read_i2c_data(cmd=self.CMD_READ_DATA, length = self.RET_DATA_LEN)
        return (	res[self.IDX_RET_TYPE],
                    res[self.IDX_RET_DATA_STRT:(self.IDX_RET_DATA_STRT+4)],
                    res[self.IDX_RET_INFO],)		

    """====================================================================
        lst_bytes 리스트의 요소들로 부터 체크섬을 계산하는 함수
       ====================================================================
    """

    def __checksum(self, lst_bytes, cmd=0):#motorola checksum
        cnt = len(lst_bytes)+1
        total_sum = sum(lst_bytes) + cmd
        lb_sum = total_sum.to_bytes(cnt, 'little')[0] #lowest byte
        return 0xff - lb_sum

    """====================================================================
        아두이노에 명령을 전송하는 함수
        보낸 데이터를 그대로 다시 받아본 후 보낸 데이터와 일치하면
        아두이노에 CHECK_OK 신호를 보내고 아두이노는 CHECK_OK신호를 받아야 받은 명령을
        처리하는 구조로 되어 있어서 데이커가 손실없이 전송되었는지를 100% 보장할 수 있음.
       ====================================================================
    """
    def __write_i2c_cmd(self, byte_list, cmd):
        writeSuccess = False
        tryCount = 0
        while not writeSuccess:
            try:
                self.__i2c.write_i2c_block_data(self.__addr, cmd, byte_list)
                #print('wrt_data (cmd:%d) >> lst:%s'%(cmd, byte_list))
                sent_back = self.__read_i2c_data(self.CMD_SEND_BACK, len(byte_list), checksum=False) # 보냈던 데이터를 그대로 다시 받는다
                #print('back_data << lst:%s'%sent_back)
                if byte_list == sent_back:  # 보낸 것과 받은 것이 같은 경우에만
                    writeSuccess = True     # 성공한 것으로 

                    notiSuccess = False     # 그리고 OK신호를 보낸다.
                    while not notiSuccess:
                        if self.__read_confirm_ok() == STAT_UNDER_NORMAL_PROC :
                            notiSuccess = True
                        else:
                            tryCount += 1
                            if (tryCount >= self.WAIT_COUNT):
                                raise Exception( 'i2c communication (noti) error.' )
                else:
                    tryCount += 1
                    if (tryCount >= self.WAIT_COUNT):
                        raise Exception( 'i2c communication (write) error.' )
            except:
                tryCount += 1
                if (tryCount >= self.WAIT_COUNT):
                    raise Exception( 'i2c communication (write comm) error.' )

    """====================================================================
        아두이노에서 데이터를 읽어온다.
        아두이노에서 체크썸을 끝에다가 붙여서 오기 때문에 그것으로 데이터가 정상인지 판단한다.
        단 보낸 데이터를 받는 경우에는 체크썸을 사용하지 않는다. (checksum = False)
       ====================================================================
    """

    def __read_i2c_data(self, cmd, length, checksum=True):
        readSuccess = False
        retryCount = 0
        res = None
        while not readSuccess:
            try:
                res = self.__i2c.read_i2c_block_data(self.__addr, cmd, length)
                if checksum: #체크썸을 사용하는 경우
                    csum = self.__checksum(res[:-1], cmd = cmd)
                    #print('rd_data (cmd:%d) << res:%s, csum:%s'%(cmd, res, csum))
                    readSuccess = ( csum == res[-1] )
                elif res != None: readSuccess = True
            except:
                retryCount += 1
                if (retryCount >= self.WAIT_COUNT):
                    raise Exception( 'i2c communication (read) error.' )
        return res
