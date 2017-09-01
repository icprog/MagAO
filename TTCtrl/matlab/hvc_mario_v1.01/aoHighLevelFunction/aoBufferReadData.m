function x = aoBufferReadData(bufferStruct,varargin)
% aoBufferReadData(bufferStruct,[connectionNr]) reads buffer data 
% connectionNr default is 1


% Author(s): D. Pescoller
%
% Copyright 2004-2008 Microgate s.r.l.
% $Revision 0.1 $ $Date: 2004/08/09
% $Revision 0.2 $ $Date: 09/03/2007 - multiple Crate connection D. Pescoller


AO_VARIABLE_DEFINE();

connectionNr=1;
if nargin>=2
    connectionNr=varargin{1};
end

firstDsp=bufferStruct.firstDsp;
lastDsp=bufferStruct.lastDsp;
len=getNumeric(bufferStruct.len)*getNumeric(bufferStruct.nrItem);
pointer=bufferStruct.sdramPointerValue;
type=varTypeArray(bufferStruct.dataType).name;

x=[];
for ii=firstDsp:2:lastDsp,
    x=[x; mgp_op_rd_sdram(ii,ii,len,pointer,connectionNr,type)];
end;
