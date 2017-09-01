function [resData,resStatus]=mgp_op_tcp_command(command,cmd_arg,varargin)
% [resData,resStatus] = mgp_op_tcp_command(command,cmd_arg,[data_len],[start_address],[tx_data],[connectionNr],[flags])
% command:        command string as defined on TCP/MGP command list
% cmd_arg:        command argument (if any), this is not an optional argument
% [data_len]:     number of dwords (at 32bit) to read or write to the device (uint32)
% [start_addr]:   start address (at 32bit), zero means the first location of the device/module (uint32)
% [tx_data]:      data buffer to send as a vector of uin32 elements (should match data_len value)
% [connectionNr]: default is 1
% [flags]:        command flags, bit#0 should be set to 1 to receive a reply (default value)
% [use_py]:       if 'yes' the function simply create the sharing file to notify the command request to python socket process
% 
% Returns:
% resData:   data returned if any
% resStatus: status return, 'SUCCESS' if OK, 'FAULT' if an error occurred
% 
% Author(s): Mario
%
% Copyright 2011 Microgate s.r.l.
% 
persistent tag;

RET_STRING_LEN_BYTE = 48;

if strcmpi(cmd_arg,'FLASH')
   MAX_TX_SUB_LENGTH = 4400000;
else
   MAX_TX_SUB_LENGTH = 65536;
end
MAX_RX_SUB_LENGTH = 65536*2;

if isempty(tag)
   tag=0;
end

data_len = 0;
start_address = 0;
tx_data = [];
connectionNr = 1;
flags=1;
use_py='no';
resData=[];

if nargin>=3
	data_len=varargin{1};
end
if nargin>=4
	start_address=varargin{2};
end
if nargin>=5
	tx_data=varargin{3};
end
if nargin>=6
	connectionNr=varargin{4};
end
if nargin>=7
	flags=varargin{5};
end
if nargin>=8
	use_py=varargin{6};
end

if isempty(data_len)
   data_len=0;
end
%if data_len>4456448
%   error('Data buffer is too long: maximum allowed value is 4456448');
%end
if length(connectionNr)>1
   error('This function doesn''t support multiple connections argument, please call the function inside a loop');
end
if isempty(start_address)
   start_address=0;
end

% check of data consistency
if ~isempty(tx_data) && length(tx_data)~=data_len
   resStatus='INVALID_DATA_LENGTH';
   tag=tag+1;
   if tag>=10000
       tag=0;
   end
   return;
end
if ~strcmp(command,'MGP_SEND_PIC_COMMAND')
   if ~isempty(tx_data) && ~(isa(tx_data,'single') || isa(tx_data,'uint32') || isa(tx_data,'int32'))
      resStatus='INVALID_DATA_CLASS';
      tag=tag+1;
      if tag>=10000
          tag=0;
      end
      return;
   end
   tx_data=typecast(tx_data,'uint32');
elseif ~isempty(tx_data) && ~isa(tx_data,'char')
   resStatus='INVALID_DATA_CLASS';
   tag=tag+1;
   if tag>=10000
       tag=0;
   end
   return;
end

if length(command)>20
   resStatus='COMMAND_LEN_NOT_VALID';
   tag=tag+1;
   if tag>=10000
       tag=0;
   end
   return;
end

if length(cmd_arg)>12
   resStatus='CMD_ARG_LEN_NOT_VALID';
   tag=tag+1;
   if tag>=10000
       tag=0;
   end
   return;
end

%get communication socket
[socket,trasport]=getSocket(connectionNr);
if ~strcmpi(trasport,'tcp')
   resStatus='INVALID_CONNECTION';
   tag=tag+1;
   if tag>=10000
       tag=0;
   end
   return;
end

if strcmpi(use_py,'yes') && strcmpi(command,'MGP_MEM_READ') && strcmpi(cmd_arg,'SDRAM')
   str=sprintf('tcpmgp_cmd_#%02d_in.dat',socket);
   fd=fopen(str,'wt');
else
   fd=0;
end

tot_tx_len=0;
sub_data_len=-1;
while tot_tx_len<data_len || sub_data_len<0
   if ~isempty(tx_data)
      sub_data_len=min(MAX_TX_SUB_LENGTH,data_len-tot_tx_len);
      sub_start_addr=start_address+tot_tx_len;
      dataCksum=bitand(sum(tx_data(tot_tx_len+1:tot_tx_len+sub_data_len)),hex2dec('FFFFFFFF'));
   else
      sub_data_len=data_len;
      sub_start_addr=start_address;
      dataCksum=0;
   end
   if fd
      fprintf(fd,'$%04d %-20s %-12s %s %08x %08x %-11s %08x\n',tag,command,cmd_arg,dec2bin(flags,8),sub_data_len,sub_start_addr,' ',dataCksum);
      fclose(fd);
      resStatus='MGP_CMD_SUCCESS';
      return;
   end

   sendStr=sprintf('$%04d %-20s %-12s %s %08x %08x %-11s %08x\n',tag,command,cmd_arg,dec2bin(flags,8),sub_data_len,sub_start_addr,' ',dataCksum);
   pnet(socket,'write',sendStr);
   if ~isempty(tx_data)
      pnet(socket,'write',tx_data(tot_tx_len+1:tot_tx_len+sub_data_len),'intel');
   end
   tot_tx_len=tot_tx_len+sub_data_len;

   if ~bitand(flags,1)
      resStatus='CMD_SUCCESS';
      tag=tag+1;
      if tag>=10000
          tag=0;
      end
      return;
   end
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
   resStatus=sscanf(repStr,'%s',1);
   if ~strcmpi(resStatus,'MGP_CMD_SUCCESS')
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
   if ~strcmp(command,'MGP_SEND_PIC_COMMAND')
      rx_dataLen=rx_dataLen*4;
   end

   rx_data=uint8(zeros(1,rx_dataLen));
   for ii=0:MAX_RX_SUB_LENGTH:rx_dataLen
      sub_rx_dataLen = min(MAX_RX_SUB_LENGTH, rx_dataLen - ii);
      rx_data(ii+1:ii+sub_rx_dataLen)=uint8(pnet(socket,'read',sub_rx_dataLen,'uint8'));
   end
%    rx_data=pnet(socket,'read',rx_dataLen,'uint8');
   if length(rx_data)<rx_dataLen
      resStatus='CMD_TIMEOUT';
      tag=tag+1;
      if tag>=10000
          tag=0;
      end
      return;
   end
   % temporaneamente tralascio il check della checksum, infatti queso non va bene se faccio una lettura da una memoria che puo'
   % cambiare "sotto il culo" come ad esempio la lettura degli ADC, quindi per ora va bene cosi'...
   % if ~strcmp(command,'MGP_SEND_PIC_COMMAND')
   %    dataCksum=bitand(sum(typecast(rx_data,'uint32')),hex2dec('FFFFFFFF'));
   % else
   %    dataCksum=bitand(sum(rx_data),hex2dec('FFFFFFFF'));
   % end
   % if rx_dataCksum~=dataCksum
   %    resStatus='CMD_CHECKSUM_FAULT';
   %    tag=tag+1;
   %    if tag>=10000
   %        tag=0;
   %    end
   %    return;
   % end

   rx_data=[rx_data zeros(1,4*ceil(length(rx_data)/4)-length(rx_data))];
   resData=typecast(rx_data,'uint32')';

   tag=tag+1;
   if tag>=10000
       tag=0;
   end
end

return;
