function x=prepareTransferFunction(varargin)
% x=prepareTranferFunction([setup])
% setup is a struct with the following fields
%  sampleTime=1;
%  identStartF=.001;
%  identStopF=.4;
%  nrPoints=32768;
%  minX=32768;
%  maxX=65535;
%  nrProve=4;
%  plotOn=0;
% returns a struct array of nrProve size each struct element represents a
%  single run test to be performed. A single test struct has the following 
%  fields:
%    rawData
%    gain
%    offset
%    startF
%    stopF
% the real data is given by data=x.rawData*x.gain+x.offset;

% Author(s): D. Pescoller
%
% Copyright 2004-2008 Microgate s.r.l.
% $Revision 0.1 $ $Date: 14/01/2005
 
%inputs

%sample setup
 sampleTime=1;
 identStartF=.001;
 identStopF=.4;
 nrPoints=32768;
 minX=32768;
 maxX=65535;
 nrProve=4;
 plotOn=0;


if nargin==1
     setup=varargin{1};
     sampleTime=setup.sampleTime;
     identStartF=setup.identStartF;
     identStopF=setup.identStopF;
     nrPoints=setup.nrPoints;
     minX=setup.minX;
     maxX=setup.maxX;
     nrProve=setup.nrProve;
end;

 prove=logspace(log10(identStartF),log10(identStopF),nrProve+1);
for provaNr=1:length(prove)-1
    startF=prove(provaNr);
    stopF=prove(provaNr+1);
    
    
    deltaF=1/(nrPoints*sampleTime);
    % f=linspace(0,maxF*2,nrPoints);
    f=[0:deltaF:(nrPoints-1)*deltaF];
    t=(0:sampleTime:(nrPoints-1)*sampleTime);

    mybin_idx=find(f >= startF & f < stopF);


    fft_vect=zeros(size(f));
    fft_vect(mybin_idx)=exp((rand(size(f(mybin_idx)))*2*pi-pi)*i);
%      fft_vect(mybin_idx)=exp(linspace(-pi,pi,length(f(mybin_idx)))*i);
%      fft_vect=fft_vect(:);
    x_hist=ifft(fft_vect, 'symmetric')/sampleTime;
    % x_hist=t_hist/max(t_hist)*V_NORMALIZED;
    x_histGain=(maxX-minX)/(max(x_hist)-min(x_hist));
    x_histOffset=minX-min(x_hist*x_histGain);

    if plotOn
        figure(provaNr);
        subplot(2,2,1);
        plot(t,x_hist);
        title('stimolo');
        subplot(2,2,2);
        plot(f,abs(fft_vect));
        title('bin');
        subplot(2,2,3);
        plot(f,angle(fft_vect));
        title('phase');
        len=length(t);      
        fft_data=abs(fft(x_hist));
        subplot(2,2,4);
        plot((0:len-1)/sampleTime/len,fft_data);
        title('FFT');xlabel('Freq (Hz)');ylabel('Amplitude');
    end;

t=t;
x(provaNr).rawData=x_hist(:);
x(provaNr).gain=x_histGain;
x(provaNr).offset=x_histOffset;
x(provaNr).startF=startF;
x(provaNr).stopF=stopF;
% x(provaNr).rawData=[ones(8192,1)*49152;ones(16384,1)*49255;ones(8192,1)*49152];
% % x(provaNr).rawData=[[49152:49152+16384] [49152+16383:-1:49153]];
% %x(provaNr).rawData=sin([1:32768]'/32768*3072*2*pi)*8192+49152;
% x(provaNr).gain=1;
% x(provaNr).offset=0;
end;

