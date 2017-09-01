function h=meshD45(data,varargin)
% meshD45(data,[zeroVal],[dspConfMatCol],[colorRange],[format])
% Plots graphically VLT D45 data
% Parameters: 
% data = input data vector, can be either a 45 elements
%   vector or a two column matrix with the first column being the
%   data vector and the second being the actuator list to which the data
%   refer (in any numbering scheme, see dspConfMatCol parameter).
%   In this case the plot range is limited to the useful area
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
%
% DSP_CONF_MAT and coordD45Act must be available in the workspace
%
% Author(s): R. Biasi
%
% Copyright 2004-2009 Microgate s.r.l.
% $Revision 0.1 $ $Date: 19/09/2009

N_ACT = 45;

coordD45Act=evalin('base','coordD45Act');
coordD45BorderAct=evalin('base','coordD45BorderAct');
borderActData_matrix=evalin('base','borderActData_matrix');
border_radius=evalin('base','border_radius');
xD45Mesh=evalin('base','xD45Mesh');
yD45Mesh=evalin('base','yD45Mesh');

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

data_full=[data;borderActData_matrix*data];

colormap('jet');
C = griddata(coordD45Act(:,1),coordD45Act(:,2),data,xD45Mesh,yD45Mesh,'linear');
% C = griddata([coordD45Act(:,1);coordD45BorderAct(:,1)],[coordD45Act(:,2);coordD45BorderAct(:,2)],data_full,xD45Mesh,yD45Mesh,'linear');
h=surf(xD45Mesh,yD45Mesh,C);
set(gcf, 'Renderer', 'ZBuffer');        % mario ha aggiunto questa chiamata per far funzionare la visuzlizzazione sul suo PC
view(2);
axis([-border_radius(2) border_radius(2) -border_radius(2) border_radius(2)],'square');
caxis(colorRange);
colorbar;
shading interp;

if exist('textFormat','var'),
    radiusAct=(coordD45Act(:,1).^2+coordD45Act(:,2).^2).^.5;
    text(coordD45Act(:,1).*(1+7e-4./radiusAct),coordD45Act(:,2).*(1+7e-4./radiusAct),{num2str(textData,textFormat)},...
        'FontSize',7,'Color','k','EraseMode','none','HorizontalAlignment','center');
end;

hold on
% scatter3(coordD45Act(:,1),coordD45Act(:,2),data,120,data,'filled');
% mycircle=rsmak('circle',border_radius(2));
% fnplt(mycircle,'k',0.5);
% mycircle=rsmak('circle',border_radius(1));
% fnplt(mycircle,'k',0.5);

hold off
