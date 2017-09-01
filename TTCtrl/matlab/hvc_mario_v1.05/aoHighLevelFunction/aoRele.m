function reply=aoRele(varargin)
% controls the rele board
% reply=aoRele([array],[setArray,1],[clearArray,0])
% array is the list of which relais are on (absolute command)
% setArray is the list of wich relais are to switch on (relative command)
% clearArray is the list of wich relais are to switch off (relative command)
% if the array, setArray, clearArray are  a hex-string the value represends
% directly the mask
% if no input is equivalent to read the status
% example
% reply=aoRele()
% aoRele([1,3,6]) 
% aoRele(8,1)
% aoRele(4,0) 
% aoRele(dec2hex(2))
% scheda test board da 25:33
% reply is a 2 element vector 
% reply(1) is the output
% reply(2) is the input
%
% Map of relais
% for the DSP testboard
% Rele    Pio
% U0       25
% U1       24
% U2       32
% U3       31
% U4       30
% U5       29
% U6       28
% U7       27
% U8       26
% for Rele of LBT testMachine. We have two relais boards connected to the
% BCU with the following mapping:
% Rele      PIO
%   1A       34
%   2A       35 
%   3A       36
%   4A       37 
%   5A       38
%   6A       39
%   7A       40
%   8A       41
%   9A       42
%  10A       43
%  11A       44
%  12A       45
%  13A       46
%  14A       47
%   1B       48
%   2B       49
%   3B       50
%   4B       51
%   5B       52
%   6B       53
%   7B       54
%   8B       55
%   9B       56
%  10B       57
%  11B       58
%  12B       59
%  13B       60
%  14B       61


% Author(s): D. Pescoller
%
% Copyright 2004-2008 Microgate s.r.l.
% $Revision 0.1 $ $Date: 14/01/2005


RELE_PAUSE=0.003;
mask=0;
mode=1;
if nargin==2
    switch varargin{2}
        case 1
            mode=2;
        case 0
            mode=3;
    end;
end;

if nargin==0
    mask=0;
    mode=2;
else
    if isstr(varargin{1})
        mask=hex2dec(varargin{1});
        mask=mask+1; %per numerazione da 0 a al posto di 1
    else
        tmp=varargin{1};
        tmp=tmp+1; %per numerazione da 0 a al posto di 1
        releWord=unique(tmp(tmp<=32));
        mask(1)=sum(bitset(0,releWord));
        releWord=unique(tmp((tmp>32)&(tmp<=62))-32);
        mask(2)=sum(bitset(0,releWord));
    end;
end;

%invert mask
if mode==3
    mask=bitcmp(mask,20)
end;

reply=mgp_op_wr_screle('0xff','0xff',3,0,[mode,mask]);
pause(RELE_PAUSE);