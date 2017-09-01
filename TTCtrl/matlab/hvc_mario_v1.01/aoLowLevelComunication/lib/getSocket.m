function udpSocket=getSocket(connectionNr)
% socket=getSocket(connectionNr)
%

% Author(s): D. Pescoller
% Copyright 2004-2008 Microgate s.r.l.
% $Revision 0.1 $ $Date: 22/12/2004

AO=evalin('base','AO');
if(connectionNr>length(AO))
    error('connection not exists');
end;
udpSocket=AO(connectionNr).udpSocket;
if(pnet(udpSocket,'status')==0)
    error('no connection')
end;
    