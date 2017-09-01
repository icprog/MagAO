function data=mgp_op_reset_devices(firstDsp,lastDsp,data,varargin)
% mgp_op_reset_devices(first,last,data,[connectionNr],[dataType],[replyFlag])
% connectionNr: default is 1
% dataType: default is 'uint32'

% Author(s): D. Pescoller
%
% Copyright 2004-2008 Microgate s.r.l.
% $Revision 0.1 $ $Date: 22/12/2004 added a flag to skip reply command (usedful when you perform a system reset)
% $Revision 0.2 $ $Date: 25/11/2008 modified the data length check (2 or 3 is accepted)
% 09/03/2012 modified by mario to support tcp connection via LCU

MGP_DEFINE();

%get parameters
firstDsp=getNumeric(firstDsp);
lastDsp=getNumeric(lastDsp);
data=getNumeric(data);
len=length(data);
startAddress=0;

if (len<2 || len>3), error('wrong data length'); end;
   
%check variable parameters
connectionNr=1;
dataType='uint32';
reply=1;

if nargin>3 && isnumeric(varargin{1})
   connectionNr=varargin{1};
end

if nargin>4
	dataType=varargin{2};
end

if nargin>5 && isnumeric(varargin{3})
   reply=varargin{3};
end

% convert data to datatype
data=eval([dataType,'(data)']);
   
% split up writes length
i=0;
lenLeft=mod(len,MAX_DWORD_LEN);
dataBlock=data((i)*MAX_DWORD_LEN+1:len);
data=reset_devices(firstDsp,lastDsp,lenLeft,startAddress+i*MAX_DWORD_LEN,dataBlock,connectionNr,dataType,reply);

function data=reset_devices(firstDsp,lastDsp,len,startAddress,data,connectionNr,dataType,reply)
global COMMAND_ID
MGP_DEFINE();

if len>MAX_DWORD_LEN
    error(['len must be <=',num2str(MAX_DWORD_LEN)]);
end;

if(isempty(COMMAND_ID))
    COMMAND_ID=0;
end;
COMMAND=10;
FLAGS=1;

% get communication socket
[socket,trasport]=getSocket(connectionNr);

switch lower(trasport)
   case 'udp'
   %create header
   %dword 1
   dword=uint32([0,0,0]);
   dword(1)=bitshift(uint32(COMMAND),24);
   dummy=bitshift(uint32(lastDsp),16);
   dword(1)=bitor(dword(1),dummy);
   dummy=bitshift(uint32(firstDsp),4);
   dword(1)=bitor(dword(1),dummy);

   COMMAND_ID=uint8(COMMAND_ID+1);
   if(COMMAND_ID==255)
       COMMAND_ID=0;
   end;
   %dword 2
   len=uint32(len);
   dword(2)=bitand(len,hex2dec('FF'));
   dword(2)=bitor(bitshift(dword(2),8),bitshift(len,-8));
   dword(2)=bitshift(dword(2),16);
   dummy=bitshift(uint32(FLAGS),8);
   dword(2)=bitor(dword(2),dummy);
   dword(2)=bitor(dword(2),uint32(COMMAND_ID));
   %dword 3
   dword(3)=uint32(startAddress);

   %write data
   pnet(socket,'write',uint8([0,0])); % write dummy
   pnet(socket,'write',uint32(dword(1)),'intel'); % write header
   pnet(socket,'write',uint32(dword(2))); % write header
   pnet(socket,'write',uint32(dword(3)),'intel'); % write header
   pnet(socket,'write',data,'intel'); % write header
   pnet(socket,'writepacket');   % Send buffer as UDP packet
   if reply
      %read back data
      pnet(socket,'readpacket');
      header=pnet(socket,'read',14,'uint8','noblock');
      data=pnet(socket,'read',1000,dataType,'intel');
      if isempty(header)
         error('communication failed NO REPLY');
      end
      header(1:2)=[];%remove dummy
      %test header if it is ok
      if(header(4)~=MGP_OP_CMD_SUCCESS)
         header(4);
         error('communication failed MGP_OP_CMD_FAULT');
      end
      if(header(8)~=COMMAND_ID)
          header(8);
          error('communication failed: wrong command ID');
      end
   end
   case 'hkl'
      len=ceil(len/2*strcmp(dataType,'double')+...
               len*1*strcmp(dataType,'single')+...
               len*4*strcmp(dataType,'char')+...
               len*4*strcmp(dataType,'int8')+...
               len*4*strcmp(dataType,'uint8')+...
               len*2*strcmp(dataType,'int16')+...
               len*2*strcmp(dataType,'uint16')+...
               len*1*strcmp(dataType,'int32')+...
               len*1*strcmp(dataType,'uint32')+...
               len/2*strcmp(dataType,'int64')+...
               len/2*strcmp(dataType,'uint64'));
      buff=[bitshift(COMMAND,24)+bitshift(lastDsp,16)+bitshift(firstDsp,4) bitshift(1,16)+len startAddress data];
      buff=[0 0 typecast(uint32(buff),'uint8')];
      [data,res_stat]=m2dsmHKLCommand('DSM_MGP',connectionNr,buff);
      if     strcmp(res_stat(1:2),'NO') || strcmp(res_stat(1:3),'BAD')
         error('Invalid LCU reply: %s\n',res_stat);
      end
   otherwise
      error('Invalid trasport socket type');
end
