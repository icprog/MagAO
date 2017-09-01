function varargout = myPSD(data,samplingFreq)
% [freqScale,psd,(rmsIntg)]=myPSD(data,samplingFreq)
%
% Computes the data to plot the PSD and the RMS as a function of frequency 
% for a given data vector
% 
%
% Parameters
% data = data vector
% samplingFreq = sampling frequency
%
% Returns
% freqScale = vector of frequency scale for the PSD (positive only)
% psd = PSD data
% rmsIntg = square root of integral of PSD, 
%           giving the rms as a function of freq. OPTIONAL PARAMETER
%
% Note
% The output PSD is truncated to the first half of the FFT output (up to
% Nyquist freq). So it is related to the signal RMS as:
% sqrt(2*sum(psd)*frequencyBin) = rms(data) where frequencyBin can be
% computed as freqScale(2)-freqScale(1)
% Care should be taken if the input data have a DC component (use
% detrend!);
%
% Example:
% [freqScale,psd,rmsIntg]=myPSD(detrend(data),samplingFreq);
% figure(1); plot(freqScale,psd);
% figure(2); plot(freqScale,rmsIntg);

% Author(s): R. Biasi
%
% Copyright 2004-2008 Microgate s.r.l.
% $Revision 0.1 $ $Date: 2007
% $Revision 0.2 $ $Date: 09/10/2008 - rmsIntg optional output added - by R.Biasi



len=length(data);
fft_data=(abs(fft(data)));
psd=fft_data.^2;
intgPSD=sum(psd)*samplingFreq/len;
ratio=rms(data)^2/intgPSD;
psd=psd*ratio; % note: ratio=1/(len*acq_freq) !!!
% psd=psd/(len*samplingFreq);
psd=psd(1:floor(len/2));
freqScale=(0:floor(len/2)-1)*samplingFreq/len;
psd(1)=psd(1)/2;  %the module in freq 0 is devided by two (0 has the + and the - contribute)

if nargout < 2 | nargout > 3,
    error('Wrong number of output arguments');
else
    varargout{1}=freqScale;
    varargout{2}=psd;
    if nargout == 3,
        rmsIntg=(2*cumsum(psd)*samplingFreq/len).^.5;
        varargout{3}=rmsIntg;
    end;
end;
