function AOConnect(hostname,varargin)
%   AOConnect('hostname',[port])
%   port default is 10000
%   creates an AO struct in workspace.
%   Example
%    AOConnect('192.168.0.51')
%   See also AODISCONNECT

% Author(s): D. Pescoller
%
% Copyright 2004-2008 Microgate s.r.l.
% $Revision 0.1 $ $Date: 22/12/2004
try
    AO=evalin('base','AO');
catch
    AO=[];
end;


if nargin==2
    port=varargin{1};
else
    port=10000+length(AO);
end

new=length(AO)+1;
udpSocket=pnet('udpsocket',port);
pnet(udpSocket,'udpconnect',hostname,port);
if(pnet(udpSocket,'status')==0)
    error('no connection')
end;
AO(new).udpSocket=udpSocket;
AO(new).hostname=hostname;
AO(new).port=port;
assignin('base','AO',AO);    
