function [outMat]=remapActList(inMat,dspConfMatInCol,dspConfMatOutCol)
% [outMat]=remapActList(inMat,dspConfMatInCol,dspConfMatOutCol)
% Swaps a matrix or a vector from one numbering convention to another
% % Author(s): R. Biasi
%
% Parameters:
% inMap: input matrix  or vector
% dspConfMatInCol: column of DSP_CONF_MAT to which inMat refers
% dspConfMatOutCol: column of DSP_CONF_MAT to which outMat refers
%
% Returns:
% outMat = reordered matrix or vector
%
% Example: outList(ff_matrix,5,7) converts ff_matrix from DSP to Mirror numbering 
% Copyright 2004-2008 Microgate s.r.l.
% $Revision 0.1 $ $Date: 25/08/2009


DSP_CONF_MAT=evalin('base','DSP_CONF_MAT');

if dspConfMatInCol==dspConfMatOutCol,
    outMat=inMat;
    return;
end;

for i=1:length(DSP_CONF_MAT(:,dspConfMatInCol)),
    idx(i)=find(DSP_CONF_MAT(:,dspConfMatOutCol) == DSP_CONF_MAT(i,dspConfMatInCol));
end;
outMat=swapRowCol(inMat,idx);

