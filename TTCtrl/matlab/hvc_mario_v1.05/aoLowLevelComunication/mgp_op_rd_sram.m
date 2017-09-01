function data=mgp_op_rd_sram(firstDsp,lastDsp,len,startAddress,varargin)
% mgp_op_rd_sram(firstDsp,lastDsp,len,startAddress,[connectionNr],[dataType])
% [connectionNr]: default is 1
% [dataType]:     default is 'uint32'
%
% Author(s): D. Pescoller
%
% Copyright 2004-2008 Microgate s.r.l.
% $Revision 0.1 $ $Date: 22/12/2004
% 13/03/2012           modified by mario to support tcp connection via LCU

%get parameters
firstDsp=getNumeric(firstDsp);
lastDsp=getNumeric(lastDsp);
len=getNumeric(len);
startAddress=getNumeric(startAddress);
connectionNr=1;
dataType='uint32';
data=[];

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
      for dsp=firstDsp:2:lastDsp
          dataBlock=pnet(socket,'READMGPPACKET',dsp,dsp,startAddress,148,1,len,dataType,'intel')';
          data=[data;dataBlock];
      end
%       data=double(data); % attenzione che Mario il 21/10/2016 ha commentato questo passaggio! concettualmente e' meglio che, se necessario, il casting sia fatto all'esterno della funzione...
                           % l'azione e' stata necessaria perche' sono state estese le funzioni aoRead e aoWrite anche sulla sram e queste funzioni hanno bisgno del data type originale
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
      buff=[bitshift(148,24)+bitshift(lastDsp,16)+bitshift(firstDsp,4) bitshift(1,16)+len startAddress];
      buff=[0 0 typecast(uint32(buff),'uint8')];
      [data,res_stat]=m2dsmHKLCommand('DSM_MGP',connectionNr,buff);
      if strcmp(res_stat(1:2),'NO') || strcmp(res_stat(1:3),'BAD')
         error('Invalid LCU reply: %s\n',res_stat);
      end
      data=typecast(data(:),dataType);
      data=reshape(data,len*type_len,ceil(((lastDsp-firstDsp)+1)/2))';
      % for back compatibility with the udp transport socket reshape the output data as a unidimensional vertical vector
      data=data';
      data=data(:);
   otherwise
      error('Invalid trasport socket type');
end
