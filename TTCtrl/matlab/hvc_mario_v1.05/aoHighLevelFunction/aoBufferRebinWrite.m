function [evenData,oddData,nrItem] = aoBufferRebinWrite(inData,nActDSP,varargin)
% [evenData,oddData,nrItem] = aoBufferRebinWrite(inData,nActDSP,[DSP_CONF_MAT_col])
%
% Organizes data to be written to the dynamic buffers
% 
% Parameters:
% inData       = data to be uploaded organized as follows:
%                if nItems == 1, data can be organized in a 2D array of size [nAct * nSamples]
%                if nItems >=  1, data are organized in a 3D array of size [nItems x nAct * nSamples]
%                nItems is the length of the vector to be read for each sample on a single channel
% nActDSP      = number of channels per DSP (4 for LBT, 8 for VLT)
% [DSP_CONF_MAT_col]
%              = optional parameter. Column of DSP_CONF_MAT in which the
%                input data are organized. If not provided, it is considered that data are 
%                already in DSP order. 
%
% NOTE: if the optional parameter DSP_CONF_MAT_col is provided,
% DSP_CONF_MAT must be present in the workspace
%
% Returns:
% evenData = data vector to be written on the even DSPs 
% oddData  = data vector to be written on the odd DSPs 
% nrItem   = buffeStruct.nrItem (=nItems*nActDSP) 
%
% Author(s): R.Biasi
%
% Copyright 2013-2020 Microgate s.r.l.
% $Revision 0.1 $ $Date: 2013/08/18

if ndims(inData) == 2,
    tmp(1,:,:)=inData;
    inData=tmp;
    clear tmp;
end;

if nargin == 3,
    DSP_CONF_MAT_col=varargin{1};
    DSP_CONF_MAT=evalin('base','DSP_CONF_MAT');
    inData=cat(2,inData,zeros(size(inData,1),size(DSP_CONF_MAT,1)-size(inData,2),size(inData,3)));
    inData=inData(:,DSP_CONF_MAT(:,DSP_CONF_MAT_col)+1,:);
end;
nAct=size(inData,2);
nDsp=nAct/nActDSP;
nrItem=size(inData,1)*nActDSP;

evenData=[];
oddData=[];
for ii=0:nActDSP*2:nAct-1,
    tmp=inData(:,(1:nActDSP)+ii,:);
    evenData=[evenData; tmp(:)];
    tmp=inData(:,(nActDSP+1:2*nActDSP)+ii,:);
    oddData=[oddData; tmp(:)];
end;
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

