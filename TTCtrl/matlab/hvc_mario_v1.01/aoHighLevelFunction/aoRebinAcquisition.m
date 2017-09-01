function acq=aoRebinAcquisition(evenDspData,oddDspData,Nsample)
% acq=aoRebinAcquisition(evenDspData,oddDspData,Nsample)
% Resamples data from dynamic buffers organized in vectors 
% of 4 for each DSP (e.g. 'distance' or 'float_ADC_value')
% Parameters: 
% evenDspData = data acquired from even DSPs (0, 2, 4, ...)
% oddDspData = data acquired from odd DSPs (1, 3, 5, ...)
% Nsample = number of samples for each channel
% Returns:
% acq = acquired data orgnized as matrix of size Nsample,Nch
% Author(s): R. Biasi
%
% Copyright 2004-2008 Microgate s.r.l.
% $Revision 0.1 $ $Date: 28/12/2005


evenDspData=reshape(evenDspData,prod(size(evenDspData)),1);
oddDspData=reshape(oddDspData,prod(size(oddDspData)),1);
len=length(evenDspData);
if length(oddDspData) ~= len,
    error('Even-Odd data length mismatch');
    return;
end;

% total number of channels (even+odd)
Nch=len/Nsample*2;
a=[0:Nsample-1];
a=[a; Nsample+a; 2*Nsample+a; 3*Nsample+a;];
a0=a;
for i=8:8:Nch-1, a=[a Nsample*i+a0]; end;
a=reshape(a,prod(size(a)),1);
acq(a+1)=evenDspData;
a=a0+Nsample*4;
for i=12:8:Nch-1, a=[a Nsample*i+a0]; end;
a=reshape(a,prod(size(a)),1);
acq(a+1)=oddDspData;
acq=reshape(acq,Nsample,Nch);
return;

% test code
% this code works properly if a dynamic buffer of BUFF_LENGTH data has been
% previously started

readData0=double(aoBufferReadData(aoBufferArray(1)));
readData1=double(aoBufferReadData(aoBufferArray(2)));
readData=aoRebinAcquisition(readData0,readData1,BUFF_LENGTH);

% readData will have size BUFF_LENGTH,Nch

