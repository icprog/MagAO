function plot_mirror(mirr_map,data,varargin)
%
% NAME:
%  PLOT_MIRROR(mirr_map,data,[dmSystem],[plotType])
%
% PURPOSE:
%  Plots the position and currents of the mirror
%
% USAGE:
%  plot_mirror(mirr_map,data)
%
% INPUT:
%  mirr_map:   actuator map of the mirror
%  data:       data to display can be a matrix of length(mirr_map) by n to obtain n sub plot of different images
%  [dmSystem]: select the deformable system (0=M4-DP, 1=D45) - default is 0 (M4-DP)
%  [plotType]: select the plot type (0=mesh, 1=boubles)
%
% OUTPUT:
%  none
%
% Author(s): M. Andrighettoni
%
% Copyright 2004-2009 Microgate s.r.l.
% $Revision 0.1 $ $Date: 03/09/2009


dmSystem=0;
plotType=0;
if nargin>=3
   dmSystem=varargin{1};
end
if nargin>=4
   plotType=varargin{2};
end

if dmSystem==0
   for i=1:size(data,2)
      subplot(1,size(data,2),i);
      if plotType==0
         meshDP([data(:,i) mirr_map(:)],NaN);
      else
         plotDP([data(:,i) mirr_map(:)],NaN);
      end
   end
elseif dmSystem==1
   for i=1:size(data,2)
      subplot(1,size(data,2),i);
      if plotType==0
         meshD45([data(:,i) mirr_map(:)],NaN);
      else
         plotD45([data(:,i) mirr_map(:)],NaN);
      end
   end
else
   error('Invalid dmSystem option');
end
