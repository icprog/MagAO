function data=mgp_op_wrsame_sdram(firstDsp,lastDsp,len,startAddress,data,varargin)
% mgp_op_wrsame_sdram(firstDsp,lastDsp,len,startAddress,data,[connectionNr],[dataType])
% [connectionNr]: default is 1
% [dataType]:     default is 'uint32'
% len, expressed in dwords, must be less or equal to MAX_DWORD_LEN dwords
%
% Author(s): D. Pescoller, Mario, Chris
%
% Copyright 2004-2011 Microgate s.r.l.
%  Revision 0.1 Date: 22/12/2004
%  Revision 0.2 Date: 23/12/2009 aggiunto come reply il numero di words realmente spedite da usare come notifica di un eventuale errore di comunicazione
%  Revision 0.3 Date:     4/2011 support tcp connection via LCU (Mario)
%  Revision 0.4 Date: 08/08/2011 len must always be expressed in dwords (Chris)
%  

% profiling shows MGP_DEFINE is pretty slow, we need just one constant,
% so I'm repeating it here (Chris)
% MGP_DEFINE();
MAX_DWORD_LEN = 364;

%get parameters
firstDsp=getNumeric(firstDsp);
lastDsp=getNumeric(lastDsp);
len=getNumeric(len);
startAddress=getNumeric(startAddress);
connectionNr=1;
dataType='uint32';
data=getNumeric(data);

%check variable parameters
if nargin==6
   if(isnumeric(varargin{1}))
      connectionNr=varargin{1};
      dataType='uint32';
   else
      connectionNr=1;
      dataType=varargin{1};
   end
elseif nargin==7
   connectionNr=varargin{1};
   dataType=varargin{2};
elseif nargin>7
   error('wrong number of parameters');
end

%convert data to datatype
data=eval([dataType,'(data)']);

[socket,trasport]=getSocket(connectionNr);

switch lower(trasport)
   case 'udp'
      data=pnet(socket,'WRITEMGPPACKET',firstDsp,lastDsp,startAddress,142,1,len,data,'intel');
      data=double(data);
   case 'tcp'
      if len > MAX_DWORD_LEN
         error(['len must be <= ', num2str(MAX_DWORD_LEN)]);
      end
      sizeof = (8 * strcmp(dataType,'double') +...
                4 * strcmp(dataType,'single') +...
                1 * strcmp(dataType,'char')   +...
                1 * strcmp(dataType,'int8')   +...
                1 * strcmp(dataType,'uint8')  +...
                2 * strcmp(dataType,'int16')  +...
                2 * strcmp(dataType,'uint16') +...
                4 * strcmp(dataType,'int32')  +...
                4 * strcmp(dataType,'uint32') +...
                8 * strcmp(dataType,'int64')  +...
                8 * strcmp(dataType,'uint64'));
      if length(data) * sizeof ~= len * 4
         error('Invalid data buffer size\n');
      end
      buff=[bitshift(142,24)+bitshift(lastDsp,16)+bitshift(firstDsp,4) bitshift(1,16)+len startAddress];
      buff=[0;0;typecast(uint32(buff(:)),'uint8');typecast(data(:),'uint8')];
      [data,res_stat]=m2dsmHKLCommand('DSM_MGP',connectionNr,buff);
      if strcmp(res_stat(1:2),'NO') || strcmp(res_stat(1:3),'BAD')
         error('Invalid LCU reply: %s\n',res_stat);
      end
   otherwise
      error('Invalid trasport socket type');
end
