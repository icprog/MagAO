function [evenDspData,oddDspData]=aoRebinStimulus(stimVector,Nsample)
% [evenDspData,oddDspData]=aoRebinStimulus(stimVector,Nsample)
% Generates correct stimulus data for dynamic buffers writing to variables 
% organized in 4 for each DSP (e.g. 'float_DAC_value')
% Parameters: 
% stimVector = stimulus data organized as matrix of size Nsample,Nch.
%   ch follows the DSP order
% Nsample = number of samples for each channel
% Returns:
% evenDspData = buffer to upload to even DSPs (0, 2, 4, ...)
% oddDspData = buffer to upload to odd DSPs (1, 3, 5, ...)
% Author(s): R. Biasi
%
% Copyright 2004-2008 Microgate s.r.l.
% $Revision 0.1 $ $Date: 28/12/2005


len=prod(size(stimVector));
Nch=len/Nsample;
stimVector=reshape(stimVector,len,1);

% total number of channels (even+odd)
a=[0:(Nsample-1)];
a=[a; a+Nsample; a+2*Nsample; a+3*Nsample];
a=reshape(a,Nsample*4,1);
a0=a;
for i=8:8:Nch-1, a=[a; a0+i*Nsample]; end;
evenDspData=stimVector(a+1);
a=a+Nsample*4;
oddDspData=stimVector(a+1);
return;

% test code
% this code works properly if a dynamic buffer of BUFF_LENGTH data has been
% previously started

BUFF_LENGTH=1024;
Nact=24;
stim_idx_dsp=7;
stim_s=rand(BUFF_LENGTH,1)*1000;

data=zeros(BUFF_LENGTH,Nact);
% writes a stimulus on a single actuator (7 according to DSP numbering)
data(:,stim_idx_dsp+1)=stim_s;

[evenData,oddData]=aoRebinStimulus(data,BUFF_LENGTH);
aoBufferWriteData(aoBufferArray(3),evenData);
aoBufferWriteData(aoBufferArray(4),oddData);


