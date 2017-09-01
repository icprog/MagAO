function [resData,resStatus]=mgp_op_tcp_mem_read(device,data_len,start_address,varargin)
% [resData,resStatus]=mgp_op_tcp_mem_read(device,data_len,start_address,[,connectionNr])
% device:       mnemonic of the device/module to be adressed (string)
% data_len:     number of dwords (at 32bit) to read from the device (uint32)
% start_addr:   start address (at 32bit), zero means the first location of the device/module (uint32)
% connectionNr: default is 1
% 
% Returns:
% resData:   returned data in uint32
% resStatus: status return, 'SUCCESS' if OK, 'FAULT' if an error occurred
% 
% Author(s): Mario
%
% Copyright 2011 Microgate s.r.l.
% 
persistent tag;

RET_STRING_LEN_BYTE = 48;

if isempty(tag)
   tag=0;
end

connectionNr=1;
if nargin>=4
    connectionNr=varargin{1};
end

%get communication socket
[socket,trasport]=getSocket(connectionNr);

if ~strcmpi(trasport,'tcp')
   error('Invalid connection type: the transport is not ''tcp''');
end

sendStr=sprintf('$%04d %-20s %-12s %s %08x %08x %-20s\n',tag,'MGP_MEM_READ',device,dec2bin(1,8),data_len,start_address,' ');
pnet(socket,'write',sendStr);

repStr=pnet(socket,'read',RET_STRING_LEN_BYTE,'char');
if length(repStr)<RET_STRING_LEN_BYTE
   error('Timeout error from TCP/MGP connection');
end
if repStr(1)~='&'
   error('Invalid reply special char received');
end
repStr(1)=' ';
rx_cmdID=sscanf(repStr,'%d',1);
if rx_cmdID~=tag
   error('Invalid command ID received');
end
repStr(2:5)=' ';
rx_cmdRep=sscanf(repStr,'%s',1);
if ~strcmpi(rx_cmdRep,'MGP_CMD_SUCCESS')
   resData=[];
   resStatus='FAULT';
   tag=tag+1;
   if tag>=10000
       tag=0;
   end
   return;
end
repStr(7:26)=' ';
rx_dataLen=sscanf(repStr,'%x',1);
repStr(28:36)=' ';
rx_dataCksum=sscanf(repStr,'%x',1);
data=pnet(socket,'read',rx_dataLen*4,'uint8');
if length(data)<rx_dataLen*4
   error('Timeout error from TCP/MGP connection');
end
resData=typecast(data,'uint32');
dataCksum=bitand(sum(resData),hex2dec('FFFFFFFF'));
if rx_dataCksum~=dataCksum
   error('Checksum check failed');
end

tag=tag+1;
if tag>=10000
    tag=0;
end
    
resStatus='SUCCESS';
return;
