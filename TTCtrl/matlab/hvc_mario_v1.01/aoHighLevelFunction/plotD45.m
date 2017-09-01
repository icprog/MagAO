function plotD45(data,varargin)
% plotD45(data,[zeroVal],[dspConfMatCol],[colorRange],[format],[altData])
% Plots graphically VLT D45 data
% Parameters: 
% data = input data vecotr, must be 330 elements
% zeroVal = optional parameter. 
%   All zeros in data will be replaced with zeroVal.
%   Default is 0 (no change)
% dspConfMatCol = optional parameter. 
%   DSP_CONF_MAT column number to which the numbering refers.
%   Default is 7 (mirror numbering)
% colorRange = optional parameter. 
%   sets the color mapping range. Two values vector (ex.: [0 10])
%   Default is autoranging (min to max of vector)
% format = optional parameter. 
%   Specifies the format string to be used to print the numeric data.
%   If not specified, no numeric data will be printed
% altData = optional parameter. 
%   Vector with the values to be printed.
%   'altData' shall use the same ordering convention as 'data'
%   If not specified, 'data' vector is printed.
%
% DSP_CONF_MAT and coordD45Act must be available in the workspace
%
% Author(s): R. Biasi
%
% Copyright 2004-2009 Microgate s.r.l.
% $Revision 0.1 $ $Date: 19/09/2009

N_ACT = 45;

dspConfMatCol=7;
colorRange='auto';
zeroVal=0;
if nargin>=2,
   zeroVal=varargin{1};
end;
if nargin>=3,
   dspConfMatCol=varargin{2};
end;
if nargin>=4,
   colorRange=varargin{3};
end;
if nargin>=5,
   textFormat=varargin{4};
end;

if size(data,2) == 2,
   tmp=zeros(N_ACT,1);
   tmp(data(:,2)+1)=data(:,1);
   data=tmp;
   clear tmp;
end;

data=data(:);
data(data == 0)=zeroVal;
if nargin>=6,
    textData=varargin{5};
    textData=remapActList(textData,dspConfMatCol,7);
    textData=textData(:);
else
    textData=data;
end;

mirrRadius=125e-3;
hole_radius=30e-3;

coordD45Act=evalin('base','coordD45Act');
colormap('jet');

scatter(coordD45Act(:,1),coordD45Act(:,2),300,data,'filled');
axis([-mirrRadius mirrRadius -mirrRadius mirrRadius],'square');
caxis(colorRange);
colorbar;

if exist('textFormat','var'),
    radiusAct=(coordD45Act(:,1).^2+coordD45Act(:,2).^2).^.5;
    text(coordD45Act(:,1).*(1+0.01./radiusAct),coordD45Act(:,2).*(1+0.01./radiusAct),{num2str(textData,textFormat)},...
        'FontSize',7,'Color','k','EraseMode','none','HorizontalAlignment','center');
%     textXOffset=-6e-3;
%     textYOffset=7.5e-3;
%     text(coordD45Act(:,1)+textXOffset,coordD45Act(:,2)+textYOffset,{num2str(textData,textFormat)},'FontSize',7,'Color','k','EraseMode','none');
end;

hold on
mycircle=rsmak('circle',mirrRadius);
fnplt(mycircle,'k',0.5);

mycircle=rsmak('circle',hole_radius);
fnplt(mycircle,'k',0.5);
hold off
