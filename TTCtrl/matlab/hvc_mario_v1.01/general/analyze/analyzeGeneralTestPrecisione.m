function x=analyzeGeneralTestPrecisione(precisione,varargin)
% analyzeGeneralTestPrecisione(precisione,[parameterName,parameterValue]...);
% several parameter can be passed by giving the couple parameterName and
% parameterValue
% possible parameterName are:
% fig figure number
% figureName 

% Author(s): D. Pescoller
%
% Copyright 2004-2008 Microgate s.r.l.
% $Revision 0.1 $ $Date: 14/01/2005

%setting defaults
fig=1;
figName='';
for i=1:2:nargin-1
    eval([varargin{i},'=varargin{i+1};']);
end;

in=precisione.in(:);
out=precisione.out(:);

[p,s]=polyfit(in,out,1);
x.gain=p(1);
x.offset=p(2);
fit=polyval(p,in);
x.errRMS=rms(out-fit);
rmsPlotGain=(max(out)-min(out))/(max(out-fit)-min(out-fit));
rmsPlotGainExp=floor(log10(rmsPlotGain));
rmsPlotGain=10^rmsPlotGainExp;
rmsPlotOffset=round((max(out)-min(out))/2);

%plot data
f=figure(fig);
set(f,'name',figName);
hold off;
plot(in,out,'xr');
hold on 
plot(in,fit);
% plot(sort(in),detrend(sort(out)),'g');
plot(sort(in),sort(out-fit)*rmsPlotGain+rmsPlotOffset,'g');

title(['gain=',num2str(x.gain),'; offset=',num2str(x.offset),'; error RMS=',num2str(x.errRMS)]);
xlabel('in');
ylabel('out');
legend('real','ideal',['rms 1e',num2str(rmsPlotGainExp),' offset ',num2str(rmsPlotOffset)],'Location','best');


