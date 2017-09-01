function x=readChannelVar(arrayIndex,startAddress,dataType,varargin)
% x=readChannelVar(arrayIndex,startAddress,dataType,[dspConfCol],[len],[device])
% this routine read DSP variables using a channel type indexing
% dspConfCol: default is column DSP_MY_INDEX (typically dsp actuator numbering) 
%             7 for mirror actuator ordering (see DSP_CONF_MAT for different column options)
% len: default is 1, number of items of selected var
% device: default is 0, DSP device

% Author(s): D. Pescoller
%
% Copyright 2004-2008 Microgate s.r.l.
% $Revision 0.1 $ $Date: 22/12/2004
% $Revision 0.2 $ $Date: 09/03/2007 - multiple Crate connection D. Pescoller
% Mario: modificata la funzione per accettare anche numero di canal per DSP diverso da 4
% 12/06/09 modified to select different DSP_CONF_MAT column - by mario
% 24/06/09 modified to use in the M4 system - by mario
% 28/09/09 modified the definition of M4 system - by mario
% 31/05/10 modified to accept different data size for channels variables also for the "standard" systems - by mario
% 29/09/10 modified to accept DSP_CONF_MAT indexes (of any column) being larger than numel(DSP_CONF_MAT)-1. By Roby - see Roby wrong


% adjust arrayIndex to start from 1 instead of 0
try
    CONF_MAT=evalin('base','DSP_CONF_MAT');
catch
    evalin('base','DSP_CONF');
    CONF_MAT=evalin('base','DSP_CONF_MAT');
end;
% find the number of channels serviced by each connection
[a,i,j]=unique(CONF_MAT(:,2),'rows');
CONNECTION_NR_2_CHANNEL_NR=i(1);
try
   DSP_NUM_CHANNELS=evalin('base','DSP_NUM_CHANNELS');
catch
   DSP_NUM_CHANNELS=4;
end;
MY_INDEX=evalin('base','DSP_MY_INDEX');
len=DSP_NUM_CHANNELS;
device=0;
if nargin>=4
   MY_INDEX=varargin{1};
end;
if nargin>=5
   len=varargin{2};
end;
if nargin>=6
   device=varargin{3};
end;

if device==0
   if rem(len,DSP_NUM_CHANNELS)
      error('Invalid data len, should be a multiple of number of channels');
   else
      len=len/DSP_NUM_CHANNELS;
   end
   % Roby wrong!!!
   % arrayIndex=arrayIndex+1;
   % CONF_MAT=sortrows(CONF_MAT,MY_INDEX);
   % list=CONF_MAT(arrayIndex,2:4);
   % add value column
   [tmp,arrayIndex,dataIndex]=intersect(CONF_MAT(:,MY_INDEX),arrayIndex);
   list=CONF_MAT(arrayIndex,2:4);
   x=zeros(size(list,1),len);
   for ii=1:size(list,1)
      connectionNr=list(ii,1);
      dspNr=list(ii,2);
      actNr=list(ii,3);
      x(ii,:)=readDspByIndex(dspNr,connectionNr,len,startAddress+actNr*len,dataType);
   end
%    CONF_MAT(:,end+1)=0;
%    CONF_MAT1=sortrows(CONF_MAT,MY_INDEX);
%    % get the list of all affected dsp
%    % [CONNECTION_NR DSP] already unique values
%    % [1  1
%    %  1  0
%    %  1  2]
%    list=unique(CONF_MAT1(arrayIndex,2:3),'rows');
%    [b,idx]=unique(list(:,1));
%    startIdx=1;
%    for ii=idx'
%        connectionNr=list((ii),1);
%        dspList=list((startIdx:ii),2);
%        startIdx=ii+1;
%        readData=readDspByIndex(dspList,connectionNr,DSP_NUM_CHANNELS,startAddress,dataType);
%        %readData=testReadDspByIndex(dspList,connectionNr,DSP_NUM_CHANNELS,startAddress,dataType);
%        %place read data in table at its right position
%        b=repmat([1:DSP_NUM_CHANNELS]',1,length(dspList));
%        c=b+repmat(dspList'*DSP_NUM_CHANNELS,DSP_NUM_CHANNELS,1);
%        baseIndex=reshape(c,length(dspList)*DSP_NUM_CHANNELS,1);
%        %%%% da checkare queste 2 righe in certe condizioni funziona solo la
%        %%%% seconda normalmente bisogna usare la prima...
%        CONF_MAT(baseIndex+(connectionNr-1)*CONNECTION_NR_2_CHANNEL_NR,end)=readData;
%        %CONF_MAT(baseIndex(1:size(CONF_MAT,1)),end)=readData(1:size(CONF_MAT,1));
%    end
%    %get requested data
%    CONF_MAT1=sortrows(CONF_MAT,MY_INDEX);
%    x=CONF_MAT1(arrayIndex,end);
elseif device==1
   % Roby wrong!!!
   % arrayIndex=arrayIndex+1;
   % CONF_MAT=sortrows(CONF_MAT,MY_INDEX);
   % list=CONF_MAT(arrayIndex,2:4);
   % add value column
   [tmp,arrayIndex,dataIndex]=intersect(CONF_MAT(:,MY_INDEX),arrayIndex);
   list=CONF_MAT(arrayIndex,2:4);
   x=zeros(size(list,1),len);
   for ii=1:size(list,1)
      connectionNr=list(ii,1);
      finNr=list(ii,2);
      actNr=list(ii,3);
      x(ii,:)=m4_op_read(finNr,finNr,len,startAddress+hex2dec('00000800')*actNr,connectionNr,dataType,2);
%       x(ii,:)=m4_op_read(finNr,finNr,len,startAddress+hex2dec('00001000')*actNr,connectionNr,dataType,2);
   end
else
   error('Invalid device selected');
end

x(dataIndex,:)=x;
