function AODisConnect()
% AODisConnect disconnects all AO's connection and clear the AO struct 
% from workspace
% See also AOCONNECT

% Author(s): D. Pescoller
% Copyright 2004-2008 Microgate s.r.l.
% $Revision 0.1 $ $Date: 22/12/2004

AO=evalin('base','AO');
for(i=1:length(AO))
   pnet(AO(i).udpSocket,'close');
end;
evalin('base','clear AO');