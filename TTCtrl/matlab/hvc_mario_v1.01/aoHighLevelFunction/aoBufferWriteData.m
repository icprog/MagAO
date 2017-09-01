function x = aoBufferWriteData(bufferStruct,data,varargin)
% aoBufferWriteData(bufferStruct,data,[connectionNr]) writes buffer data 
% connectonNr default = 1


% Author(s): D. Pescoller
%
% Copyright 2004-2008 Microgate s.r.l.
% $Revision 0.1 $ $Date: 2004/08/09
% $Revision 0.2 $ $Date: 09/03/2007 - multiple Crate connection D. Pescoller


AO_VARIABLE_DEFINE();

connectionNr=1;
if nargin>=3
    connectionNr=varargin{1};
end

firstDsp=bufferStruct.firstDsp;
lastDsp=bufferStruct.lastDsp;
len=getNumeric(bufferStruct.len)*getNumeric(bufferStruct.nrItem);
pointer=bufferStruct.sdramPointerValue;
type=varTypeArray(bufferStruct.dataType).name;
if prod(size(data)) ~= len*(floor((lastDsp-firstDsp)/2)+1),
    error('Wrong number of elements in parameter data');
end;


data=reshape(data,len,prod(size(data))/len);
i2=1;
for ii=firstDsp:2:lastDsp,
    mgp_op_wrsame_sdram(ii,ii,len,pointer,data(:,i2),connectionNr,type);
    i2=i2+1;
end;
