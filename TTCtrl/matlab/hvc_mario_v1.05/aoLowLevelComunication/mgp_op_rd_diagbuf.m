function data=mgp_op_rd_diagbuf(firstDsp,lastDsp,len,startAddress,varargin)
% mgp_op_rd_diagbuf(firstDsp,lastDsp,len,startAddress,[connectionNr],[dataType])
% connectionNr: default is 1
% dataType: default is 'uint32'

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

%check variable parameters
if nargin==0+4
    connectionNr=1;
    dataType='uint32';
elseif nargin==1+4
    if(isnumeric(varargin{1}))
        connectionNr=varargin{1};
        dataType='uint32';
    else
        connectionNr=1;
        dataType=varargin{1};
    end
elseif nargin==2+4
    connectionNr=varargin{1};
    dataType=varargin{2};
else
    error('wrong number of parameters');
end;

%split up reads length
data=[];
i=0;
if len>=MAX_DWORD_LEN
    for i=1:floor(len/MAX_DWORD_LEN)
        data=[data,readSdram(firstDsp,lastDsp,MAX_DWORD_LEN,startAddress+(i-1)*MAX_DWORD_LEN,connectionNr,dataType)];
    end;
end;
lenLeft=mod(len,MAX_DWORD_LEN);
data=[data,readSdram(firstDsp,lastDsp,lenLeft,startAddress+i*MAX_DWORD_LEN,connectionNr,dataType)]';

function data=readSdram(firstDsp,lastDsp,len,startAddress,connectionNr,dataType)
global COMMAND_ID
if(isempty(COMMAND_ID))
    COMMAND_ID=0;
end;
MGP_DEFINE();

COMMAND=152;
FLAGS=1;

%get communication socket

[socket,trasport]=getSocket(connectionNr);

switch lower(trasport)
   case 'udp'
      %create header
      %dword 1
      dword=uint32([0,0,0]);
      dword(1)=bitshift(uint32(COMMAND),24);
      dummy=bitshift(uint32(lastDsp),16);
      dword(1)=bitor(dword(1),dummy);
      dummy=bitshift(uint32(firstDsp),4);
      dword(1)=bitor(dword(1),dummy);

      COMMAND_ID=uint8(COMMAND_ID+1);
      if(COMMAND_ID==255)
          COMMAND_ID=0;
      end;
      %dword 2

      len=uint32(len);
      dword(2)=bitand(len,hex2dec('FF'));
      dword(2)=bitor(bitshift(dword(2),8),bitshift(len,-8));
      dword(2)=bitshift(dword(2),16);
      dummy=bitshift(uint32(FLAGS),8);
      dword(2)=bitor(dword(2),dummy);
      dword(2)=bitor(dword(2),uint32(COMMAND_ID));
      %dword 3
      dword(3)=uint32(startAddress);

      %write data
      pnet(socket,'write',uint8([0,0])); % write dummy
      pnet(socket,'write',uint32(dword(1)),'intel'); % write header
      pnet(socket,'write',uint32(dword(2))); % write header
      pnet(socket,'write',uint32(dword(3)),'intel'); % write header
      pnet(socket,'writepacket');   % Send buffer as UDP packet
      %read back data
      bufferLen=pnet(socket,'readpacket');
      header=pnet(socket,'read',14,'uint8','noblock');
      data=pnet(socket,'read',1000,dataType,'intel');
      header(1:2)=[];%remove dummy
      %test header if it is ok
      if(header(4)~=MGP_OP_CMD_SUCCESS)
          header(4)
          error('communication failed MGP_OP_CMD_FAULT')
      end;
      readLen=bitor(uint16(header(5)),bitshift(uint16(header(6)),8));
      if(readLen~=len*(lastDsp-firstDsp+1))
          readLen
          error('communication failed: wrong data length');
      end;
      if(header(8)~=COMMAND_ID)
          header(8)
          error('communication failed: wrong command ID')
      end
   case 'hkl'
      type_len=0.5*strcmp(dataType,'double')+...
               1*strcmp(dataType,'single')+...
               4*strcmp(dataType,'char')+...
               4*strcmp(dataType,'int8')+...
               4*strcmp(dataType,'uint8')+...
               2*strcmp(dataType,'int16')+...
               2*strcmp(dataType,'uint16')+...
               1*strcmp(dataType,'int32')+...
               1*strcmp(dataType,'uint32')+...
               0.5*strcmp(dataType,'int64')+...
               0.5*strcmp(dataType,'uint64');
      var_name=[{'_dbfub_startRegister'},{'_dbfub_stopRegister'},{'_dbfub_paramBuffer0'},{'_dbfub_paramBuffer1'},{'_dbfub_paramBuffer2'},{'_dbfub_paramBuffer3'},{'_dbfub_paramBuffer4'},{'_dbfub_paramBuffer5'},{'_dbfub_paramBuffer6'},{'_dbfub_paramBuffer7'}];
      if startAddress>=8
         [data,resStatus]=m2dsmHKLCommand(['DSM_AOREAD ', cell2mat(var_name(startAddress/8+2)), ' ', num2str(firstDsp), '-', num2str(lastDsp)],connectionNr);
      else
         [data,resStatus]=m2dsmHKLCommand(['DSM_AOREAD ', cell2mat(var_name(startAddress+1)), ' ', num2str(firstDsp), '-', num2str(lastDsp)],connectionNr);
      end
      if ~strcmpi(resStatus, 'OK DSM_AOREAD 200')
         error(['aoRead() / HKL mode: LCU returned error: ', resStatus]);
      end
      data=typecast(data(:),dataType);
      data=reshape(data,len*type_len,ceil(((lastDsp-firstDsp)+1)/2))';
      % for back compatibility with the udp transport socket reshape the output data as a unidimensional vertical vector
      data=data';
      data=data(:);
   otherwise
      error('Invalid trasport socket type');
end
