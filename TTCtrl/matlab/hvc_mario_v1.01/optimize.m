function [p,val]=optimize(method)
% usage is: optimize('method');
% optimize a set of parameters p using a "method"
% method is a function which has the following definition
% cost=method(p,methodParameter)
% methodParameter are considered to be constant during the optimization
% process
%
% besides the method function there is a methodInit function which is
% supposed to initialize the OPTIONS, the initial p vector, and the
% methodParameter

% Author(s): D. Pescoller
%
% Copyright 2006-2010 Microgate s.r.l.
% $Revision 0.1 $ $Date: 15/12/2006

feval([method,'Init']);
[p,val]=fminsearch(method,p,OPTIONS,methodParameter);
