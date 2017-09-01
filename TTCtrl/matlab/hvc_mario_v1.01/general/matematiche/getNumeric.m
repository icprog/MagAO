function x=getNumeric(var)
% this gets a numeric value
% var can be in binary(b) hexadecimal(0x) or decimal format or a numeric
% is useful for comfortable user interface
%
% example
% getNumeric(45)
% getNumeric('b01101101')
% getNumeric('0xFFB4500')


% Author(s): D. Pescoller
%
% Copyright 2004-2008 Microgate s.r.l.
% $Revision 0.1 $ $Date: 22/12/2004


if(isnumeric(var))
    x=var;
    return;
end;

x=str2num(var);
if(isempty(x))
    if(var(1)=='b')
        x=bin2dec(var(2:end));
    elseif(strcmp(var(1:2),'0x'))
        x=hex2dec(var(3:end));
    end;
end;
