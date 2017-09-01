function mgp_op_wr_sfpdpi(firstDsp,lastDsp,len,startAddress,data,varargin)
% mgp_op_wr_mfci(firstDsp,lastDsp,len,startAddress,data,[connectionNr],[dataType])
% connectionNr: default is 1
% dataType: default is 'uint32'

% Author(s): M. Andrighettoni
%
% Copyright 2004-2008 Microgate s.r.l.
% $Revision 0.1 $ $Date: 21/11/2008

% MGP_DEFINE();

%get parameters
firstDsp=getNumeric(firstDsp);
lastDsp=getNumeric(lastDsp);
len=getNumeric(len);
startAddress=getNumeric(startAddress);
data=getNumeric(data);
COMMAND=165;

%check variable parameters
if nargin==0+5
    connectionNr=1;
    dataType='uint32';
elseif nargin==1+5
    if(isnumeric(varargin{1}))
        connectionNr=varargin{1};
        dataType='uint32';
    else
        connectionNr=1;
        dataType=varargin{1};
    end
elseif nargin==2+5
    connectionNr=varargin{1};
    dataType=varargin{2};
else
    error('wrong number of parameters');
end;

if len>32
    error('wrong data length, should be lower than 32');
end;
if startAddress+len>32
    error('wrong start address vs length, should be lower than 32');
end;

% get communication socket
[socket,trasport]=getSocket(connectionNr);

switch lower(trasport)
   case 'udp'
      %convert data to datatype
      data=eval([dataType,'(data)']);
      pnet(socket,'WRITEMGPPACKET',firstDsp,lastDsp,startAddress,COMMAND,1,len,data,'intel');
   case 'tcp'
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
      buff=[bitshift(COMMAND,24)+bitshift(lastDsp,16)+bitshift(firstDsp,4) bitshift(1,16)+len startAddress];
      buff =[0;0;typecast(uint32(buff(:)),'uint8')];
      buff1=typecast(data(:),'uint8');
      [res_data,res_stat]=m2dsmHKLCommand('DSM_MGP',connectionNr,[buff;buff1(:)]);
      if     strcmp(res_stat(1:2),'NO') || strcmp(res_stat(1:3),'BAD')
         error('Invalid LCU reply: %s\n',res_stat);
      end
   otherwise
      error('Invalid trasport socket type');
end
