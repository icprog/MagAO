function x=rms(a)
%
% computes rms of a vector
% x=rms(a)

% Author(s): D. Pescoller
%
% Copyright 2004-2008 Microgate s.r.l.
% $Revision 0.1 $ $Date: 2004/08/19

[nr,nc]=size(a);
if(nc==1)
    x=sqrt(a'*a/nr);
    return;
end;
if(nr==1)
    x=sqrt(a*a'/nc);
    return;
end;
if(nr>1&&nc>1)
 x=sqrt(sum(a.*a)/nr);
end;
