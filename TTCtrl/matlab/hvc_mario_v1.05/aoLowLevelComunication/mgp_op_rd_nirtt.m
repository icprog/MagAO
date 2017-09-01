function data=mgp_op_rd_nirtt(firstDsp,lastDsp,len,startAddress,varargin)
% mgp_op_rd_nirtt(firstDsp,lastDsp,len,startAddress,[connectionNr],[dataType])
% [connectionNr]: default is 1
% [dataType]:     default is 'uint32'
%
% Author(s): D. Pescoller
%
% Copyright 2004-2008 Microgate s.r.l.
% $Revision 0.1 $ $Date: 22/12/2004
% 13/03/2012           modified by mario to support tcp connection via LCU

%get parameters
firstDsp=getNumeric(firstDsp);
lastDsp=getNumeric(lastDsp);
len=getNumeric(len);
startAddress=getNumeric(startAddress);
connectionNr=1;
dataType='uint32';
data=[];

%check variable parameters
if     nargin==5
   if(isnumeric(varargin{1}))
      connectionNr=varargin{1};
      dataType='uint32';
   else
      connectionNr=1;
      dataType=varargin{1};
   end
elseif nargin==6
   connectionNr=varargin{1};
   dataType=varargin{2};
elseif nargin>6
   error('wrong number of parameters');
end

[socket,trasport]=getSocket(connectionNr);

switch lower(trasport)
   case 'udp'
      for dsp=firstDsp:2:lastDsp
          dataBlock=pnet(socket,'READMGPPACKET',dsp,dsp,startAddress,171,1,len,dataType,'intel')';
          data=[data;dataBlock];
      end
      data=double(data);
   otherwise
      error('Invalid trasport socket type');
end
