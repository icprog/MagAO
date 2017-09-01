function writeChannelVar(arrayIndex,startAddress,data,dataType,varargin)
% x=writeChannelVar(arrayIndex,startAddress,dataType,[dspConfCol],[len],[device])
% this routine writes DSP variables using a channel type indexing
% dspConfCol: default is column DSP_MY_INDEX (typically dsp actuator numbering) 
%             7 for mirror actuator ordering (see DSP_CONF_MAT for different column options)
% len: default is 1, number of items of selected var
% device: default is 0, DSP device

% Author(s): D. Pescoller
%
% Copyright 2004-2008 Microgate s.r.l.
% $Revision 0.1 $ $Date: 22/12/2004

% adjust arrayIndex to start from 1 instead of 0
% 12/06/09 modified to select different DSP_CONF_MAT column - by mario
% 24/06/09 modified to use in the M4 system - by mario
% 28/09/09 modified the definition of M4 system - by mario
% 31/05/10 modified to accept different data size for channels variables also for the "standard" systems - by mario
% 29/09/10 modified to accept DSP_CONF_MAT indexes (of any column) being larger than numel(DSP_CONF_MAT)-1. By Roby - see Roby wrong

CONF_MAT=evalin('base','DSP_CONF_MAT');
try
   DSP_NUM_CHANNELS=evalin('base','DSP_NUM_CHANNELS');
catch
   DSP_NUM_CHANNELS=4;
end;

MY_INDEX=evalin('base','DSP_MY_INDEX');
len=DSP_NUM_CHANNELS;
device=0;
if nargin>=5
   MY_INDEX=varargin{1};
end;
if nargin>=6
   len=varargin{2};
end;
if nargin>=7
   device=varargin{3};
end;

% arrayIndex=arrayIndex+1; ROBY wrong!
[tmp,arrayIndex,dataIndex]=intersect(CONF_MAT(:,MY_INDEX),arrayIndex);
data=data(dataIndex,:);
CONF_MAT1=CONF_MAT;

if length(unique(arrayIndex))~=length(arrayIndex)
    error('arrayIndex must have unique values')
end;

if device==0
   if rem(len,DSP_NUM_CHANNELS)
      error('Invalid data len, should be a multiple of number of channels');
   else
      len=len/DSP_NUM_CHANNELS;
   end
   if length(arrayIndex) > 1 && size(data,1)==1
       data=data(:);
   end
   if length(arrayIndex)~=size(data,1)
      error('arrayIndex len must be equal to dataLen rows')
   end;
   if len~=size(data,2)
      error('len must be equal to dataLen colums')
   end;
%    CONF_MAT1=sortrows(CONF_MAT,MY_INDEX); Roby wrong
   CONF_MAT1(arrayIndex,end+1:end+len)=data;
   %temporar improvement: if we write the same to all channels we make something easier
   try
      if max(abs(data(2:end)-data(1:end-1)))==0
          if max((arrayIndex(:)-1)-CONF_MAT(:,MY_INDEX))==0
              for connectionNr=min(CONF_MAT1(:,2)):max(CONF_MAT1(:,2))  
                  firstDsp=min(CONF_MAT1(:,3));
                  lastDsp=max(CONF_MAT1(:,3));
                  len=DSP_NUM_CHANNELS;
                  dataValue=ones(1,DSP_NUM_CHANNELS)*data(1);
                  mgp_op_wrsame_dsp(firstDsp,lastDsp,len,startAddress,dataValue,connectionNr,dataType);
              end
              return;
          end
      end
   end
   %per ora faccio una cosa poco efficiente ma sicura
   for i=arrayIndex(:)'
      connectionNr=CONF_MAT1(i,2);
      firstDsp=CONF_MAT1(i,3);
      lastDsp=CONF_MAT1(i,3);
      dataValue=CONF_MAT1(i,end-len+1:end);
      offset=CONF_MAT1(i,4);
      mgp_op_wrsame_dsp(firstDsp,lastDsp,len,startAddress+offset*len,dataValue,connectionNr,dataType);
   end
elseif device==1
%    CONF_MAT1=sortrows(CONF_MAT,MY_INDEX); Roby wrong
   CONF_MAT1(arrayIndex,end+1:end+len)=data;
   if length(arrayIndex)~=size(data,1)
      error('arrayIndex len must be equal to dataLen rows')
   end;
   if len~=size(data,2)
      error('len must be equal to dataLen colums')
   end;
   for i=arrayIndex
      connectionNr=CONF_MAT1(i,2);
      firstDsp=CONF_MAT1(i,3);
      lastDsp=CONF_MAT1(i,3);
      dataValue=CONF_MAT1(i,end-len+1:end);
      offset=CONF_MAT1(i,4);
      m4_op_write(firstDsp,lastDsp,len,startAddress+hex2dec('00000800')*offset,dataValue,connectionNr,dataType,7);
%       m4_op_write(firstDsp,lastDsp,len,startAddress+hex2dec('00001000')*offset,dataValue,connectionNr,dataType,7);
   end
else
   error('Invalid device selected');
end
