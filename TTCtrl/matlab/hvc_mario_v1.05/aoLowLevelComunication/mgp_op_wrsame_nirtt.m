function mgp_op_wrsame_nirtt(firstDsp,lastDsp,len,startAddress,data,varargin)
% mgp_op_wrsame_nirtt(firstDsp,lastDsp,len,startAddress,data,[connectionNr],[dataType])
% [connectionNr]: default is 1
% [dataType]:     default is 'uint32'
% len must be less then 364 dword
%
% Author(s): D. Pescoller
%
% Copyright 2004-2008 Microgate s.r.l.
% $Revision 0.1 $ $Date: 22/12/2004
%            modified by mario to support tcp connection via LCU

MGP_DEFINE();

%get parameters
firstDsp=getNumeric(firstDsp);
lastDsp=getNumeric(lastDsp);
len=getNumeric(len);
startAddress=getNumeric(startAddress);
connectionNr=1;
dataType='uint32';
data=getNumeric(data);

%check variable parameters
if nargin==6
   if(isnumeric(varargin{1}))
      connectionNr=varargin{1};
      dataType='uint32';
   else
      connectionNr=1;
      dataType=varargin{1};
   end
elseif nargin==7
   connectionNr=varargin{1};
   dataType=varargin{2};
elseif nargin>7
   error('wrong number of parameters');
end

%convert data to datatype
data=eval([dataType,'(data)']);

[socket,trasport]=getSocket(connectionNr);

switch lower(trasport)
   case 'udp'
      pnet(socket,'WRITEMGPPACKET',firstDsp,lastDsp,startAddress,170,1,len,data,'intel');
   otherwise
      error('Invalid trasport socket type');
end
