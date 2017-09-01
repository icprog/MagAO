function data=mgp_op_rd_sfpdpi(firstDsp,lastDsp,len,startAddress,varargin)
% mgp_op_rd_sfpdpi(firstDsp,lastDsp,len,startAddress,[connectionNr],[dataType])
% connectionNr: default is 1
% dataType: default is 'uint32'

% Author(s): M. Andrighettoni
%
% Copyright 2004-2008 Microgate s.r.l.
% $Revision 0.1 $ $Date: 21/11/2008

% MGP_DEFINE();

% get parameters
firstDsp=getNumeric(firstDsp);
lastDsp=getNumeric(lastDsp);
len=getNumeric(len);
startAddress=getNumeric(startAddress);

% check variable parameters
if nargin==0+4
    connectionNr=1;
    dataType='uint32';
elseif nargin==1+4
    if(isnumeric(varargin{1}))
        connectionNr=varargin{1};
        dataType='uint32';
    else
        connectionNr=1;
        dataType=varargin{1};
    end
elseif nargin==2+4
    connectionNr=varargin{1};
    dataType=varargin{2};
else
    error('wrong number of parameters');
end;

if len>32
    error('wrong data length, should be lower than 16');
end;
if startAddress+len>32
    error('wrong start address vs length, should be lower than 16');
end;

udpSocket=getSocket(connectionNr);
data=pnet(udpSocket,'READMGPPACKET',firstDsp,lastDsp,startAddress,166,1,len,dataType,'intel')';
data=double(data);
