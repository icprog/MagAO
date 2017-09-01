
pro testBCU

WR_SDRAM = 141
RD_SDRAM = 142

data = fltarr(100)

for i=0,99 do data[i] = 200-i

BCU_num=0
firstDSP=255
lastDSP=255
bcu_address=0L
opcode = WR_SDRAM
datalen = 100L*4
timeout = 1000L


res = call_external('idl_msglib.so','idl_SendBCUCommand', BCU_num, firstDSP, lastDSP, $
			opcode, bcu_address, data, datalen, timeout)

print,"Write result :",res


data2 = fltarr(100)

opcode = RD_SDRAM

res = call_external('idl_msglib.so','idl_SendBCUCommand', BCU_num, firstDSP, lastDSP, $
			opcode, bcu_address, data2, datalen, timeout)

print,"Read result:", res

print,data2

end

