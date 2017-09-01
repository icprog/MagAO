function x=getContinuousIndex(array)
% returns an index wich can be used to reconstruct the array
% with a minimum of needed elements
% the starting idea is that a sequence of [1,2,3,4,5,6] 
% can also be written like [1:6]
% array must be a monoton vector of integers

% Author(s): D. Pescoller
%
% Copyright 2004-2008 Microgate s.r.l.
% $Revision 0.1 $ $Date: 22/12/2004


b=array(2:end)-array(1:end-1);
x=find(b>1);

