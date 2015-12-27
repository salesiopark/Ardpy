class Ard:

    CMD_READ_DATA   = 0
    CMD_READ_ID     = 1
    CMD_CHANGE_ADDR = 2
    CMD_SEND_DATA   = 3
    CMD_EXEC_FUNC   = 4
    CMD_READ_STAT   = 5
    CMD_CHECK_OK    = 6#CMD_READ_STAT_WRITE	= 6
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
    #STAT_ERR_DATA          = 2 		   # wrong data received in arduino
    STAT_ERR_FUNC_INDEX     = 3 # function index out of bound
    STAT_ERR_ARG_INDEX      = 4 # function index out of bound
    STAT_ERR_NO_ARG         = 5
    STAT_ERR_ARG_TYPE       = 6

    WRITE_SUCCESS   = 1
    #WRITE_FAIL     = 0

    EXE_ERR_RETRY	= 3 # maximum retry no when exec_func fails
    WAIT_COUNT 		= 20 #maximum trial number for i2c communication 
    TIMEOUT			= 2.0 #sec.
    MAX_RETRY_CNT	= 5 # maximum retry no when data corruptted
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

    def _exec_func(self, index):
        """execute function in the pyard (arduino) device"""
        if index >= self.__num_funcs:
            raise Exception('Index of func out of bound (dev:0x%x)'%self.__addr)

        retryCount = 0
        successExec = False

        while not successExec:
            self.__send_args() # 미리 저장된 args를 전송한다.
            send_pckt = [self.CMD_EXEC_FUNC, index]
            #send_pckt.append( self.__checksum(send_pckt, cmd=self.CMD_SEND_BACK) )				
            self.__write_i2c_cmd(send_pckt, self.CMD_EXEC_FUNC)
            stat = self.__wait_until_cmd_handled()

            if stat == self.STAT_CMD_COMPLETED:
                dtype, lst, info = self.__read_data()
                self.__reset_args()

                #print("ret type:%d"%dtype)
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
    #import time as __time

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

    def __send_args(self):
        for arg_pckt in self.__lst_args:
            retryCount = 0
            if arg_pckt != None:
                SuccessToSendArg = False
                while not SuccessToSendArg :
                    self.__write_i2c_cmd(arg_pckt, self.CMD_SEND_DATA)
                    stat = self.__wait_until_cmd_handled()
                    if stat == self.STAT_CMD_COMPLETED: SuccessToSendArg = True
                    else:
                        retryCount+=1
                        if retryCount > self.MAX_RETRY_CNT:

                            if stat == self.STAT_ERR_ARG_INDEX:	
                                raise Exception('Index of arg out of bound (dev:0x%x)'%self.__addr)
                            else:
                                type, ret, info = self.__read_data()
                                #print('cmd:%d'%info)
                                raise Exception('Unknown err(%d) in sending arg (dev:0x%x)'%(stat,self.__addr))

    def __reg_arg(self, index, dtype, lst_data):
        if index >= self.__max_arg_num:
            raise Exception('Arg index must be under %d.'%self.__max_arg_num)
        lst_pckt = [self.CMD_SEND_DATA, index, dtype ] 
        lst_pckt.extend(lst_data)
        self.__lst_args[index] = lst_pckt
        #print('lst_pckt:%s'%lst_pckt)

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

    def __read_stat(self):
        res = self.__read_i2c_data(cmd = self.CMD_READ_STAT, length =2)
        return res[0]

    def __read_confirm_ok(self):
        res = self.__read_i2c_data(cmd = self.CMD_CHECK_OK, length =2)
        return res[0]

    def __read_data(self):
        res = self.__read_i2c_data(cmd=self.CMD_READ_DATA, length = self.RET_DATA_LEN)
        return (	res[self.IDX_RET_TYPE],
                    res[self.IDX_RET_DATA_STRT:(self.IDX_RET_DATA_STRT+4)],
                    res[self.IDX_RET_INFO],)		

    def __checksum(self, lst_bytes, cmd=0):#motorola checksum
        cnt = len(lst_bytes)+1
        total_sum = sum(lst_bytes) + cmd
        lb_sum = total_sum.to_bytes(cnt, 'little')[0] #lowest byte
        return 0xff - lb_sum

    def __write_i2c_cmd(self, byte_list, cmd):
        writeSuccess = False
        tryCount = 0
        while not writeSuccess:
            try:
                self.__i2c.write_i2c_block_data(self.__addr, cmd, byte_list)
                #print('wrt_data (cmd:%d) >> lst:%s'%(cmd, byte_list))
                sent_back = self.__read_i2c_data(self.CMD_SEND_BACK, len(byte_list), checksum=False)
                #print('back_data << lst:%s'%sent_back)
                if byte_list == sent_back:
                    writeSuccess = True

                    notiSuccess = False
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

    def __read_i2c_data(self, cmd, length, checksum=True):
        readSuccess = False
        retryCount = 0
        res = None
        while not readSuccess:
            try:
                res = self.__i2c.read_i2c_block_data(self.__addr, cmd, length)
                if checksum:
                    csum = self.__checksum(res[:-1], cmd = cmd)
                    #print('rd_data (cmd:%d) << res:%s, csum:%s'%(cmd, res, csum))
                    readSuccess = ( csum == res[-1] )
                elif res != None: readSuccess = True
            except:
                retryCount += 1
                if (retryCount >= self.WAIT_COUNT):
                    raise Exception( 'i2c communication (read) error.' )
        return res

#from .arduno import ArdUno
#from .tlcd import TextLcd

