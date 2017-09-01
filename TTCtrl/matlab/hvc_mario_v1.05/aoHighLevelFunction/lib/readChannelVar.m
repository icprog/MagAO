function x=readChannelVar(arrayIndex,startAddress,dataType,varargin)
% x=readChannelVar(arrayIndex,startAddress,dataType,[dspConfCol],[len],[device],[step_ptr],[vartype])
% this routine read DSP variables using a channel type indexing
% dspConfCol: default is column DSP_MY_INDEX (typically dsp actuator numbering) 
%             7 for mirror actuator ordering (see DSP_CONF_MAT for different column options)
% len: default is 1, number of items of selected var
% device: default is 0, DSP device
% step_ptr: default is set "len" value, otherwise set a distance between channels data

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
% 22/08/14 modified to set the distance between the channels data using the step_ptr instead of len


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
vartype='double';
if nargin>=4
   MY_INDEX=varargin{1};
end;
if nargin>=5
   len=varargin{2};
end;
if nargin>=6
   device=varargin{3};
end;
if nargin>=7
   step_ptr=varargin{4};
else
   step_ptr=len/DSP_NUM_CHANNELS;
end;
if nargin>=8
   vartype=varargin{5};
end;

if (isnumeric(device) && device==0) || strcmpi(device,'DSP')
   if rem(len,DSP_NUM_CHANNELS)
      error('Invalid data len, should be a multiple of number of channels');
   else
      len=len/DSP_NUM_CHANNELS;
   end
   [tmp,arrayIndex,dataIndex]=intersect(CONF_MAT(:,MY_INDEX),arrayIndex);
   list=CONF_MAT(arrayIndex,2:4);
   x=zeros(size(list,1),len);
   for ii=1:size(list,1)
      connectionNr=list(ii,1);
      dspNr=list(ii,2);
      actNr=list(ii,3);
      x(ii,:)=readDspByIndex(dspNr,connectionNr,len,startAddress+actNr*step_ptr,dataType);
   end
elseif isnumeric(device) && device==1
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
elseif strcmpi(device,'FPGA_CNTLOOP') || strcmpi(device,'FPGA_COMPINT')
   [tmp,arrayIndex,dataIndex]=intersect(CONF_MAT(:,MY_INDEX),arrayIndex);
   list=CONF_MAT(arrayIndex,:);
   x=uint32(zeros(size(list,1),len));
   for ii=1:size(list,1)
      connectionNr=list(ii,2);
      offset=list(ii,4);
      [x(ii,:),bb]=mgp_op_tcp_command('MGP_MEM_READ',device,len,startAddress+offset*step_ptr,[],connectionNr);
      if ~strcmpi(bb,'MGP_CMD_SUCCESS')
         error('Bad reply command: %s',bb);
      end
   end
   sx=size(x);
   x=typecast(x(:),vartype);
   x=reshape(x,sx);
else
   error('Invalid device selected');
end

x(dataIndex,:)=x;
