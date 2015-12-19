from pyharper import Dev
dev = Dev(16)
cnt = 0
while True:
	dev._send_int8(0);dev._exec_func(0)
	dev._send_uint8(1);dev._exec_func(1)
	dev._send_int16(1);dev._exec_func(2)
	dev._send_uint16(1);dev._exec_func(3)
	dev._send_int32(1);dev._exec_func(4)
	dev._send_uint32(cnt);cnt=dev._exec_func(5)
	dev._send_float(1.0);dev._exec_func(6)

	dev._send_str(str(cnt));dev._exec_func(7)
	pass
