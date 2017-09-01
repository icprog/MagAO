function cross_time=find_cross(data,time,threshold,slope)
% x_cross=find_cross(y_data,x_data,threshold,slope)
% Finds the position on the ascissa where y_data crosses the threshold.
% The ascissa is computed by linear interpolation between two data points across threshold. 
% Parameters: 
% y_data = function data points
% x_data = ascissa of function data
% threshold = threshold level
% slope = 1 for rising, -1 for falling
% Author(s): R. Biasi
%
% Copyright 2004-2008 Microgate s.r.l.
% $Revision 0.1 $ $Date: 14/10/2005

data=data(:)*slope;
threshold=threshold*slope;
time=time(:);


idx_leq_th=find(data <= threshold);
if isempty(idx_leq_th), cross_time=[]; return; end;
if idx_leq_th(end)==length(data), idx_leq_th=idx_leq_th(1:end-1); end;
% trova gli indici dei punti appena sopra lo zero, sui rising edge
idx_jgt_th=idx_leq_th(find(data(idx_leq_th+1) > threshold))+1;
idx_jleq_th=idx_jgt_th-1;
% interpolazione tempi
cross_time=(time(idx_jgt_th)-time(idx_jleq_th))./(data(idx_jgt_th)-data(idx_jleq_th))...
   .*(threshold-data(idx_jleq_th))+time(idx_jleq_th);


% test code
function test_find_cross
test_time=0:.1:10;
test_data=sin(2*pi*3*test_time);
test_threshold=0.1;
cross_time=find_cross(test_data,test_time,test_threshold,1);
plot(test_time,test_data,cross_time,ones(size(cross_time))*test_threshold,'*');

