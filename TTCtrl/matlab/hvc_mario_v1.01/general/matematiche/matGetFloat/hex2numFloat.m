function x=hex2numFloat(a)
% hex2numFloat(stream)
% converts an 8 length hex string in the corresponding IEEE 32-bit float 
% it builds on the matGetFloat MEX function!!!

% Author(s): D. Pescoller
%
% Copyright 2004-2008 Microgate s.r.l.
% $Revision 0.1 $ $Date: 14/01/2005

if (length(a)~=8)
    error('wrong stream length');
end;
b=reshape(a,2,4)';
c=hex2dec(b);
x=matGetFloat(uint8(c));