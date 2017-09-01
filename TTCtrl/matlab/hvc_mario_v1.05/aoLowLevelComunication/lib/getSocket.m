function [socket,transport]=getSocket(connectionNr,varargin)
% [socket,trasport]=getSocket(connectionNr,[device])

% Author(s): D. Pescoller
%
% Copyright 2004-2008 Microgate s.r.l.
% $Revision 0.1 $ $Date: 22/12/2004
%            modified by mario to support tcp connection via LCU
%            20/04/2011 modified by mario to support MOXA device connection
%            23/06/2014 modified to use the 'transport' field in the AO struct instead of getting from pnet call

device = 'AO';

if nargin>=2
   device = varargin{1};
end

con=evalin('base',device);
if connectionNr>length(con)
    error('connection not exists');
end
socket=con(connectionNr).socket;
transport=con(connectionNr).transport;
% stat=pnet(socket,'status');
% switch stat
%    case 0
%       error('no connection')
%    case 18
%       trasport='udp';
%    case 11
%       trasport='tcp';
%    otherwise
%       error('invalid connection trasport')
% end
