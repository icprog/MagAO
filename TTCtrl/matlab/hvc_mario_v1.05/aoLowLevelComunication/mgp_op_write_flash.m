function mgp_op_write_flash(firstDsp,lastDsp,len,startAddress,data,varargin)
% mgp_op_write_flash(firstDsp,lastDsp,len,startAddress,data,[connectionNr],[dataType])
% connectionNr: default is 1
% dataType: default is 'uint32'
% len must be less then 364 


% Author(s): D. Pescoller
%
% Copyright 2004-2008 Microgate s.r.l.
% $Revision 0.1 $ $Date: 22/12/2004
MGP_DEFINE();

%get parameters
firstDsp=getNumeric(firstDsp);
lastDsp=getNumeric(lastDsp);
len=getNumeric(len);
startAddress=getNumeric(startAddress);
data=getNumeric(data);

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
%convert data to datatype
data=eval([dataType,'(data)']);
udpSocket=getSocket(connectionNr);

pnet(udpSocket,'setwritetimeout',MGP_WRITETIMEOUT_LONG);
pnet(udpSocket,'WRITEMGPPACKET',firstDsp,lastDsp,startAddress,131,1,len,data,'intel')';
pnet(udpSocket,'setwritetimeout',MGP_WRITETIMEOUT_SHORT);
