function x=computeTransferFunction(result,setup,varargin)
% compues the transferFunction from a set of tests
% x=identEvaluate(result,setup,['fig',figureNr])
% 
% result is an array struct with the following fields
% in,out,startF,stopF
% 
% setup needs to report the sampleTime and nrPoints fields
% the same setup used for the prepareTranferFunction can be used
%
% returns a struct with the following fields: 
% f
% fft_mod
% fft_angle


% Author(s): D. Pescoller
%
% Copyright 2004-2008 Microgate s.r.l.
% $Revision 0.1 $ $Date: 14/01/2005
 
%defaults
fig=1;
figName='';
title_extension='';
for ii=1:2:nargin-2
    eval([varargin{ii},'=varargin{ii+1};']);
end;



sampleTime=setup.sampleTime;
nrPoints=setup.nrPoints;
nrProve=length(result);

 
fft_mod=zeros(nrPoints,1); 
fft_angle=zeros(nrPoints,1); 

fft_in1=zeros(nrPoints,1);
fft_out1=zeros(nrPoints,1);


deltaF=1/(nrPoints*sampleTime);
f=[0:deltaF:(nrPoints-1)*deltaF];
for provaNr=1:nrProve
 fft_in=fft(result(provaNr).in);
 fft_out=fft(result(provaNr).out);
 startF=result(provaNr).startF;
 stopF=result(provaNr).stopF;
 mybin_idx=find(f >= startF & f < stopF);
 fft_ratio=[];
 fft_ratio(mybin_idx)=fft_out(mybin_idx)./fft_in(mybin_idx);
 fft_ratio=fft_ratio(:);
 fft_mod(mybin_idx)=fft_mod(mybin_idx)+abs(fft_ratio(mybin_idx));
 fft_angle(mybin_idx)=fft_angle(mybin_idx)+angle(fft_ratio(mybin_idx));
 
%  plot(f,20*log10(abs(fft_in)));
%  title('fft_in')
%  figure(4);
%  plot(f,20*log10(abs(fft_out)));
%  title('fft_out')
%  pause(1);
end;

fHandle=figure(fig);
set(fHandle,'name',figName);
subplot(2,1,1);
semilogx(f,20*log10(fft_mod));
title(['module log ',title_extension])
xlabel('f hz')
ylabel('db')
grid on;
subplot(2,1,2);
semilogx(f,(fft_angle)*180/pi);
title('angle');
xlabel('f hz')
ylabel('deg')
grid on
drawnow();
x.f=f;
x.fft_mod=fft_mod;
x.fft_angle=fft_angle;


