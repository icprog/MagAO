function resStatus=mgp_op_close_tcp_connection(varargin)
% resStatus = mgp_op_close_tcp_connection([connectionNr])
% [connectionNr]: default is 1
%
% Author(s): Mario
%
% Copyright 2004-2011 Microgate s.r.l.
%  Revision 0.1 Date: 29/05/2014 used to send close connection command to the new tcp/mgp socket
%  

RET_STRING_LEN_BYTE = 48;

%get parameters
connectionNr=1;

%check variable parameters
if nargin>=1
	connectionNr=varargin{1};
   dataType='uint32';
end

[socket,trasport]=getSocket(connectionNr);

switch lower(trasport)
   case 'tcp'
      tag=randi(10000)-1;

      sendStr=sprintf('$%04d %-20s %-12s %s %-46s\n',tag,'MGP_CLOSE_CONNECTION',' ',dec2bin(1,8),' ');
      pnet(socket,'write',sendStr);

      repStr=pnet(socket,'read',RET_STRING_LEN_BYTE,'char');
      if length(repStr)<RET_STRING_LEN_BYTE
         error('Timeout error from TCP/MGP connection');
      end
      if repStr(1)~='&'
         error('Invalid reply special char received');
      end
      repStr(1)=' ';
      rx_cmdID=sscanf(repStr,'%d',1);
      if rx_cmdID~=tag
         error('Invalid command ID received');
      end
      repStr(2:5)=' ';
      rx_cmdRep=sscanf(repStr,'%s',1);
      if ~strcmpi(rx_cmdRep,'MGP_CMD_SUCCESS')
         resStatus='FAULT';
         return;
      end
      resStatus='SUCCESS';
      return
   otherwise
      error('Invalid trasport socket type');
end
