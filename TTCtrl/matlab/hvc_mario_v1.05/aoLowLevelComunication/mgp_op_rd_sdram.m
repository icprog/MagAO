function data=mgp_op_rd_sdram(firstDsp,lastDsp,len,startAddress,varargin)
% mgp_op_rd_sdram(firstDsp,lastDsp,len,startAddress,[connectionNr],[dataType])
% [connectionNr]: default is 1
% [dataType]:     default is 'uint32'
% len, expressed in dwords, must be less or equal to MAX_DWORD_LEN dwords
%
% Author(s): D. Pescoller, Mario, Chris
%
% Copyright 2004-2011 Microgate s.r.l.
%  Revision 0.1 Date: 22/12/2004
%  Revision 0.2 Date:     4/2011 support tcp connection via LCU (Mario)
%  Revision 0.4 Date: 08/08/2011 len must always be expressed in dwords (Chris)
% 13/03/2012           modified by mario to support tcp connection via LCU
% 

MGP_DEFINE();

%get parameters
firstDsp=getNumeric(firstDsp);
lastDsp=getNumeric(lastDsp);
len=getNumeric(len);
startAddress=getNumeric(startAddress);
connectionNr=1;
dataType='uint32';

%check variable parameters
if     nargin==5
   if(isnumeric(varargin{1}))
      connectionNr=varargin{1};
      dataType='uint32';
   else
      connectionNr=1;
      dataType=varargin{1};
   end
elseif nargin==6
   connectionNr=varargin{1};
   dataType=varargin{2};
elseif nargin>6
   error('wrong number of parameters');
end

[socket,trasport]=getSocket(connectionNr);

switch lower(trasport)
   case 'udp'
      data=[];
      for dsp=firstDsp:2:lastDsp
          dataBlock=pnet(socket,'READMGPPACKET',dsp,dsp,startAddress,143,1,len,dataType,'intel')';
          data=[data;dataBlock];
      end
%       data=double(data); % attenzione che Mario ha commentato questo passaggio... concettualmente e' meglio che, se necessario, il casting sia fatto all'esterno della funzione...
                           % l'azione sara' necessaria nel caso si voglia estendere le funzioni aoRead e aoWrite anche sulla sdram e queste funzioni hanno bisgno del data type originale
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
      counterAddr=startAddress;
      data=zeros((lastDsp-firstDsp+1)*len,1);
      for startPtr=1:MAX_DWORD_LEN:len
         endPtr=min(startPtr+MAX_DWORD_LEN-1,len);
         packetLen=min(MAX_DWORD_LEN,endPtr-startPtr+1);
%          if len>MAX_DWORD_LEN
%             error(['len must be <= ',num2str(MAX_DWORD_LEN)]);
%          end
         buff=[bitshift(143,24)+bitshift(lastDsp,16)+bitshift(firstDsp,4) bitshift(1,16)+packetLen counterAddr];
         buff=[0 0 typecast(uint32(buff),'uint8')];
         timeout=0;
         while timeout<5
            [res_sdata,res_stat]=m2dsmHKLCommand('DSM_MGP',connectionNr,buff);
            if strcmp(res_stat(1:3),'BAD')
               fprintf('*');
               error('Invalid LCU reply: %s\n',res_stat);
            end
            if strcmp(res_stat(1:2),'NO')
               fprintf('*');
               timeout=timeout+1;
            else
               break;
            end
         end
         if timeout==5
            error('Invalid LCU reply: %s\n',res_stat);
         end
         counterAddr=counterAddr+MAX_DWORD_LEN;
         res_sdata=typecast(res_sdata(:),dataType);
         data(startPtr:endPtr)=res_sdata;
      end
      data=reshape(data,len*type_len,ceil(((lastDsp-firstDsp)+1)/2))';
%       buff=[bitshift(143,24)+bitshift(lastDsp,16)+bitshift(firstDsp,4) bitshift(1,16)+len startAddress];
%       buff=[0 0 typecast(uint32(buff),'uint8')];
%       [data,res_stat]=m2dsmHKLCommand('DSM_MGP',connectionNr,buff);
%       if strcmp(res_stat(1:2),'NO') || strcmp(res_stat(1:3),'BAD')
%          error('Invalid LCU reply: %s\n',res_stat);
%       end
%       data=typecast(data(:),dataType);
%       data=reshape(data,len*type_len,ceil(((lastDsp-firstDsp)+1)/2))';
      % for back compatibility with the udp transport socket reshape the output data as a unidimensional vertical vector
      data=data';
      data=data(:);
   otherwise
      error('Invalid trasport socket type');
end
