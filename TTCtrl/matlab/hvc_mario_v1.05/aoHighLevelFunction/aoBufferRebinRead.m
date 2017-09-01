function x = aoBufferRebinRead(evenData,oddData,bufferStruct,nDsp,nActDSP,varargin)
% x = aoBufferRebinRead(evenData,oddData,bufferStruct,nDsp,nActDSP,[DSP_CONF_MAT_col,nUsedAct])
%
% Reorganizes data read from the dynamic buffers
% 
% Parameters:
% evenData     = data read from the even DSPs (0,2,4,...)
% oddData      = data read from the odd DSPs (1,3,5,...)
% bufferStruct = buffer structure. Notice that the sizes of the buffer must 
%                be identical for both the 'odd' and 'even' buffers!
% nDsp         = number of DSPs to read from
% nActDSP      = number of channels per DSP (4 for LBT, 8 for VLT)
% [DSP_CONF_MAT_col]
%              = optional parameter. Column of DSP_CONF_MAT in which the
%                output data shall be reorganized. If not provided, data are 
%                not reorganized. 
% [nUsedAct]   = optional parameter. Number of used actuators. The output is 
%                truncated after nUsedAct.
%
% NOTE: if the optional parameter DSP_CONF_MAT_col is provided,
% DSP_CONF_MAT must be present in the workspace
%
% Returns:
% x = output data array
% Said nItems=bufferStruct.nrItem/nActDSP:
%   if nItems == 1, data are organized in a 2D array of size [nAct * nSamples]
%   if nItems >  1, data are organized in a 3D array of size [nItems * nAct * nSamples]
%
% Author(s): R.Biasi
%
% Copyright 2013-2020 Microgate s.r.l.
% $Revision 0.1 $ $Date: 2013/08/18


nItems=bufferStruct.nrItem/nActDSP;
nAct=nDsp*nActDSP;

x=zeros(nAct*nItems,bufferStruct.len);
evenData=reshape(evenData,bufferStruct.nrItem*bufferStruct.len,nDsp/2);
for i=1:nDsp/2
   x((2*i-2)*bufferStruct.nrItem+1:(2*i-1)*bufferStruct.nrItem,:)=reshape(evenData(:,i),bufferStruct.nrItem,bufferStruct.len);
end
oddData=reshape(oddData,bufferStruct.nrItem*bufferStruct.len,nDsp/2);
for i=1:nDsp/2
   x((2*i-1)*bufferStruct.nrItem+1:(2*i)*bufferStruct.nrItem,:)=reshape(oddData(:,i),bufferStruct.nrItem,bufferStruct.len);
end

% re-organizes data
x=reshape(x,nItems,bufferStruct.len*nAct);
x=reshape(x,nItems,nAct,bufferStruct.len);
% reorders data according to the required DSP_CONF_MAT column
if nargin > 5,
    DSP_CONF_MAT_col=varargin{1};
    DSP_CONF_MAT=evalin('base','DSP_CONF_MAT');
    x(:,DSP_CONF_MAT(:,DSP_CONF_MAT_col)+1,:)=x;
end;
% eliminates non-used actuators
if nargin == 7,
    nUsedAct=varargin{2};
    x=x(:,1:nUsedAct,:);
end;
% if nItems == 1, data are organized in a 2D array of size [nSamples x nAct]
% if nItems >  1, data are organized in a 3D array of size [nItems x nSamples x nAct]
if nItems == 1,
    x = squeeze(x(1,:,:));
end
return;

% test code
% generates random data:
%  - 1024 samples
%  - each sample and each actuator contains a vector with 13 elements
%  - 45 used actuators
%
% loads DSP_CONF_MAT
run('DSP_CONF');
myBuffer.len=60000;
data=rand(2,1170,myBuffer.len);
% nActDSP = 8 (VLT). Data are 'mirror' numbered (col.7 of DSP_CONF) 
[evenData,oddData,nrItem] = aoBufferRebinWrite(data,8,7);
myBuffer.nrItem=nrItem;
% 156 DSPs, nActDSP = 8 (VLT). 
% Data are 'mirror' numbered (col.7 of DSP_CONF) 
% 1170 actuators used
x=aoBufferRebinRead(evenData,oddData,myBuffer,156,8,7,1170);
if isempty(find(x-data ~= 0)),
    disp('Test successful');
else
    disp('Test failed');
end;

