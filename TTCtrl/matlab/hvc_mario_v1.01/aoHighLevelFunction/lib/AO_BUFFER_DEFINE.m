function AO_BUFFER_DEFINE()
% declares various buffer properties
% this is like a C-INCLUDE file

% Author(s): D. Pescoller
%
% Copyright 2004-2008 Microgate s.r.l.
% $Revision 0.1 $ $Date: 2004/08/09

%variable types
assignin('caller','BUFFER_NUMBERS',num2str([1:6]'));
assignin('caller','DSP_NUMBERS',num2str([0:1]'));

compareType(1).name='none';
compareType(2).name='trigger value < memPointer';
compareType(3).name='trigger value > memPointer';
compareType(4).name='trigger value = memPointer';
assignin('caller','COMPARE_TYPE',compareType);
