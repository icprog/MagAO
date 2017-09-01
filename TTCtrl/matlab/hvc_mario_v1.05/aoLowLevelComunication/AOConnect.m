function AOConnect(hostname,varargin)
%   AOConnect('hostname1',['hostname2'],['hostname3']...['hostnameN']['P',port],['T',transport],['C',close all connection],['A',append new connection])
%   hostname are the hostname/IP of the BCU of each crate
%   port default is 10000 and is unique for all connections.
%   Note: if multiple connections are used the port number is increased by one for each connection (this allows also proper windows operation... NEEDS TO BE CONFERMED!!!)
%   creates an AO struct in workspace containing all connection informations.
%   Example:
%    AOConnect('192.168.0.51')
%    AOConnect('192.168.0.51','192.168.0.52','192.168.0.53')
%    AOConnect('192.168.0.51','192.168.0.52','192.168.0.53','P',10000)

%   See also AODISCONNECT

% Author(s): D. Pescoller
%
% Copyright 2008-2012 Microgate s.r.l.
% $Revision 0.2 $ $Date: 09/03/2007
%            modified by mario to support tcp connection via LCU
%            modified by mario to support hsdl connection via LCU (modified also previous 'tcp' id to 'hkl')
%            modified by mario to support mgp connection via TCP, added the field 'trasport' into the AO struct, directly used via getSocket function
%            modified by mario to add the 'Append' feature that adds a new connection without closing the existing ones

%default port
port=0;
transport='udp';
close_sockets='yes';
append=1==0;
MGP_DEFINE();

if nargin==0
    error('At least on hostname/IP must be supplied')
end

connection{1}=hostname;
ii=1;
while ii < nargin
	if     varargin{ii}=='P'
      port=varargin{ii+1};
      ii=ii+2;
	elseif varargin{ii}=='T'
      transport=varargin{ii+1};
      ii=ii+2;
	elseif varargin{ii}=='C'
      close_sockets=varargin{ii+1};
      ii=ii+2;
	elseif varargin{ii}=='A'
      append=1==1;
      ii=ii+1;
	else
      connection{ii+1}=varargin{ii};
      ii=ii+1;
	end
end

% close all existing connections
if strcmpi(close_sockets,'yes') && ~append,
   try
      AODisconnect();
   %    disp('reconnecting...')
   catch
      disp('create new connection...')
   end
end

if     ~port && strcmpi(transport,'udp')
   port=10000;
elseif ~port && strcmpi(transport,'hkl')
   port=6324;
elseif ~port && strcmpi(transport,'hsdl')
   port=6326;
elseif ~port && strcmpi(transport,'tcp')
   port=10000;
end

try
   AO=evalin('base','AO');
catch
   AO=[];
end

idx=length(AO)+1;
for ii=1:length(connection)
   switch lower(transport)
      case 'udp'
         socket=pnet('udpsocket',port);
         pnet(socket,'udpconnect',connection{ii},port);
         if pnet(socket,'status')<=0
           error('no connection')
         end
         AO(idx).socket=socket;
         AO(idx).hostname=connection{ii};
         AO(idx).port=port;
         AO(idx).transport=transport;
         if strcmpi(transport,'udp')
            port=port+1;
         end
         pnet(socket,'setwritetimeout',MGP_WRITETIMEOUT_SHORT);
         pnet(socket,'setreadtimeout',MGP_READTIMEOUT_SHORT);
         % flush read buffer if any data
         while ~isempty(pnet(socket,'read',1,'uint8','noblock')),end;
         % pnet(socket,'setwritetimeout',MGP_WRITETIMEOUT_LONG);
         % pnet(socket,'setreadtimeout',MGP_READTIMEOUT_LONG);
      case 'tcp'
         socket=pnet('tcpconnect',connection{ii},port);
         pnet(socket,'setwritetimeout',MGP_WRITETIMEOUT_SHORT);
         pnet(socket,'setreadtimeout',MGP_READTIMEOUT_SHORT);
%          pnet(socket,'readline');
%          pnet(socket,'readline');
%          pnet(socket,'readline');
%          pnet(socket,'readline');
%          pnet(socket,'readline');
         fprintf([pnet(socket,'readline') pnet(socket,'read',1)]);
         fprintf([pnet(socket,'readline') pnet(socket,'read',1)]);
         fprintf([pnet(socket,'readline') pnet(socket,'read',1)]);
         fprintf([pnet(socket,'readline') pnet(socket,'read',1)]);
         fprintf([pnet(socket,'readline') pnet(socket,'read',1)]);
         if pnet(socket,'status')<=0
            error('no connection')
         end
         AO(idx).socket=socket;
         AO(idx).hostname=connection{ii};
         AO(idx).port=port;
         AO(idx).transport=transport;
      case 'hkl'
         socket=pnet('tcpconnect',connection{ii},port);
         pnet(socket,'readline');
         pnet(socket,'readline');
         pnet(socket,'readline');
         pnet(socket,'readline');
         pnet(socket,'readline');
%          disp(pnet(socket,'readline'));
%          disp(pnet(socket,'readline'));
%          disp(pnet(socket,'readline'));
%          disp(pnet(socket,'readline'));
%          disp(pnet(socket,'readline'));
         if pnet(socket,'status')<=0
            error('no connection')
         end
         AO(idx).socket=socket;
         AO(idx).hostname=connection{ii};
         AO(idx).port=port;
         AO(idx).transport=transport;
%          pnet(socket,'setwritetimeout',MGP_WRITETIMEOUT_SHORT);
%          pnet(socket,'setreadtimeout',MGP_READTIMEOUT_SHORT);
         pnet(socket,'setwritetimeout',MGP_WRITETIMEOUT_LONG);
         pnet(socket,'setreadtimeout',MGP_READTIMEOUT_LONG);
%          pnet(socket,'setwritetimeout',MGP_WRITETIMEOUT_LONGLONG);
%          pnet(socket,'setreadtimeout',MGP_READTIMEOUT_LONGLONG);
      case 'hsdl'
         socket=pnet('tcpconnect',connection{ii},port);
         if pnet(socket,'status')<=0
            error('no connection')
         end
         AO(idx).socket=socket;
         AO(idx).hostname=connection{ii};
         AO(idx).port=port;
         AO(idx).transport=transport;
%          pnet(socket,'setwritetimeout',MGP_WRITETIMEOUT_SHORT);
%          pnet(socket,'setreadtimeout',MGP_READTIMEOUT_SHORT);
         pnet(socket,'setwritetimeout',MGP_WRITETIMEOUT_LONG);
         pnet(socket,'setreadtimeout',MGP_READTIMEOUT_LONG);
%          pnet(socket,'setwritetimeout',MGP_WRITETIMEOUT_LONGLONG);
%          pnet(socket,'setreadtimeout',MGP_READTIMEOUT_LONGLONG);
      otherwise
         error('Invalid transport argument');
   end
   idx = idx + 1;
end

assignin('base','AO',AO);
