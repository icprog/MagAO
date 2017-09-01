function x=sigGenWave(varargin)
% Script per la generazione della sequenza di uscita per il DAC del signal generator.
% Genera automaticamente un file siggen_seq.vhd con la parte del VHDL di competenza,
% il file SignalGenerator.mif e SignalGenerator.dat con i parametri di default per la RAM di configurazione
% ed un file S:\Progetti\Engineering\Lbt\Ps\Sig_gen\istim.dat per la simulazione PSPICE 
% si può passare una struct contenente tutti i campi che si vogliono
% sovrascrivere
% ritorna 2 int32 di configurazione

% Author(s): R. Biasi
% Change: D. Pescoller  
% Copyright 2004-2008 Microgate s.r.l.
% $Revision 0.1 $ $Date: 22/12/2004


% dati di ingresso

sin2=1;			% se vale 0, utilizza l'onda quadra con filtro di Thomson. Altrimenti implementa un sin2.
settling_time=1e-6; 	% standard 1.6e-6 % durata della zona sin2. Usato solo se sin2=1
nf=1;			% ordine del filtro di smoothing (Thomson) (1..4). Usato solo se sin2==0.
ft=1.6e6; 		% frequenza di taglio del filtro di smoothing (Hz).  Usato solo se sin2==0.
fclk=60.71428571e6;	% frequenza del clock della macchina a stati
fwave=160e3;		% frequenza dell'onda in uscita
amp_wave=1;		% ampiezza ell'onda di uscita, in dac_range (1=full range)
dac_bits=9;		% numero di bit del DAC
pkpk_amp=10;		% ampiezza segnale in uscita corrispondente  full DAC range
pspice_ampl=.5e-3;	% ampiezza per il simulatore Pspice
pspice_zero=.5e-3;	% offset per il simulatore Pspice
sampling_pos=1.05e-6;  % standard 1.05e-6; % 1.1e-6;	% posizione del segnale di sampling, espressa come anticipo del segnale di sampling rispetto alla fine della semionda
sampling_duration=1e-6;	% durata dell'impulso di sampling
ADC_sampling_pos=1.1e-6; % standard 1.1e-6; % 2e-6	% posizione del segnale di sampling, espressa come anticipo del segnale di sampling rispetto alla fine della semionda
ADC_sampling_duration=.5e-6;	% durata dell'impulso di sampling
gain=-1;		 % standard -1 % -.696	% ampiezza segnale, compresa tra -1 e 0.9921875
ADC_decimation=3; 		% fattore di decimazione segnale sampling ADC. floor(fwave*2/f_ADC-1)

sin2=1;			% se vale 0, utilizza l'onda quadra con filtro di Thomson. Altrimenti implementa un sin2.
settling_time=1e-6; 	% standard 1.6e-6 % durata della zona sin2. Usato solo se sin2=1
nf=1;			% ordine del filtro di smoothing (Thomson) (1..4). Usato solo se sin2==0.
ft=1.6e6; 		% frequenza di taglio del filtro di smoothing (Hz).  Usato solo se sin2==0.
fclk=60.71428571e6;	% frequenza del clock della macchina a stati
fwave=80e3;		% frequenza dell'onda in uscita
amp_wave=1;		% ampiezza ell'onda di uscita, in dac_range (1=full range)
dac_bits=9;		% numero di bit del DAC
pkpk_amp=10;		% ampiezza segnale in uscita corrispondente  full DAC range
pspice_ampl=.5e-3;	% ampiezza per il simulatore Pspice
pspice_zero=.5e-3;	% offset per il simulatore Pspice
sampling_pos=2e-6;  % standard 1.05e-6; % 1.1e-6;	% posizione del segnale di sampling, espressa come anticipo del segnale di sampling rispetto alla fine della semionda
sampling_duration=1.5e-6;	% durata dell'impulso di sampling
ADC_sampling_pos=3.8e-6; % standard 1.1e-6; % 2e-6	% posizione del segnale di sampling, espressa come anticipo del segnale di sampling rispetto alla fine della semionda
ADC_sampling_duration=.5e-6;	% durata dell'impulso di sampling
gain=-1;		 % standard -1 % -.696	% ampiezza segnale, compresa tra -1 e 0.9921875
ADC_decimation=1; 		% fattore di decimazione segnale sampling ADC. floor(fwave*2/f_ADC-1)


sin2=1;			% se vale 0, utilizza l'onda quadra con filtro di Thomson. Altrimenti implementa un sin2.
settling_time=1e-6; 	% standard 1.6e-6 % durata della zona sin2. Usato solo se sin2=1
nf=1;			% ordine del filtro di smoothing (Thomson) (1..4). Usato solo se sin2==0.
ft=1.6e6; 		% frequenza di taglio del filtro di smoothing (Hz).  Usato solo se sin2==0.
fclk=60.71428571e6;	% frequenza del clock della macchina a stati
fwave=60e3;		% frequenza dell'onda in uscita
amp_wave=1;		% ampiezza ell'onda di uscita, in dac_range (1=full range)
dac_bits=9;		% numero di bit del DAC
pkpk_amp=10;		% ampiezza segnale in uscita corrispondente  full DAC range
pspice_ampl=.5e-3;	% ampiezza per il simulatore Pspice
pspice_zero=.5e-3;	% offset per il simulatore Pspice
sampling_pos=2.5e-6;  % standard 1.05e-6; % 1.1e-6;	% posizione del segnale di sampling, espressa come anticipo del segnale di sampling rispetto alla fine della semionda
sampling_duration=2e-6;	% durata dell'impulso di sampling
ADC_sampling_pos=5.2e-6; % standard 1.1e-6; % 2e-6	% posizione del segnale di sampling, espressa come anticipo del segnale di sampling rispetto alla fine della semionda
ADC_sampling_duration=4e-6;	% durata dell'impulso di samplingsampling_pos
gain=-1;		 % standard -1 % -.696	% ampiezza segnale, compresa tra -1 e 0.9921875
ADC_decimation=1; 		% fattore di decimazione segnale sampling ADC. floor(fwave*2/f_ADC-1)


sin2=1;			% se vale 0, utilizza l'onda quadra con filtro di Thomson. Altrimenti implementa un sin2.
settling_time=1e-6; 	% standard 1.6e-6 % durata della zona sin2. Usato solo se sin2=1
nf=1;			% ordine del filtro di smoothing (Thomson) (1..4). Usato solo se sin2==0.
ft=1.6e6; 		% frequenza di taglio del filtro di smoothing (Hz).  Usato solo se sin2==0.
fclk=60.71428571e6;	% frequenza del clock della macchina a stati
fwave=82.960e3;		% frequenza dell'onda in uscita
amp_wave=1;		% ampiezza ell'onda di uscita, in dac_range (1=full range)
dac_bits=9;		% numero di bit del DAC
pkpk_amp=10;		% ampiezza segnale in uscita corrispondente  full DAC range
pspice_ampl=.5e-3;	% ampiezza per il simulatore Pspice
pspice_zero=.5e-3;	% offset per il simulatore Pspice
sampling_pos=2.5e-6;  % standard 1.05e-6; % 1.1e-6;	% posizione del segnale di sampling, espressa come anticipo del segnale di sampling rispetto alla fine della semionda
sampling_duration=2.2e-6;	% durata dell'impulso di sampling
ADC_sampling_pos=4.5e-6; % standard 1.1e-6; % 2e-6	% posizione del segnale di sampling, espressa come anticipo del segnale di sampling rispetto alla fine della semionda
ADC_sampling_duration=3.5e-6;	% durata dell'impulso di sampling
gain=-1;		 % standard -1 % -.696	% ampiezza segnale, compresa tra -1 e 0.9921875
ADC_decimation=1; 		% fattore di decimazione segnale sampling ADC. floor(fwave*2/f_ADC-1)

sin2=0;			% se vale 0, utilizza l'onda quadra con filtro di Thomson. Altrimenti implementa un sin2.
settling_time=1e-6; 	% standard 1.6e-6 % durata della zona sin2. Usato solo se sin2=1
nf=1;			% ordine del filtro di smoothing (Thomson) (1..4). Usato solo se sin2==0.
ft=1.2e6; 		% frequenza di taglio del filtro di smoothing (Hz).  Usato solo se sin2==0.
fclk=60.71428571e6;	% frequenza del clock della macchina a stati
amp_wave=1;		% ampiezza ell'onda di uscita, in dac_range (1=full range)
dac_bits=9;		% numero di bit del DAC
pkpk_amp=10;		% ampiezza segnale in uscita corrispondente  full DAC range
pspice_ampl=.5e-3;	% ampiezza per il simulatore Pspice
pspice_zero=.5e-3;	% offset per il simulatore Pspice

fwave=84e3;		% frequenza dell'onda in uscita
sampling_pos=3.5e-6;  % standard 1.05e-6; % 1.1e-6;	% posizione del segnale di sampling, espressa come anticipo del segnale di sampling rispetto alla fine della semionda
sampling_duration=2.6e-7;	% durata dell'impulso di sampling
ADC_sampling_pos=0.7e-6; % standard 1.1e-6; % 2e-6	% posizione del segnale di sampling, espressa come anticipo del segnale di sampling rispetto alla fine della semionda
ADC_sampling_duration=70e-9;	% durata dell'impulso di sampling
gain=-1; %-1;		 % standard -1 % -.696	% ampiezza segnale, compresa tra -1 e 0.9921875
ADC_decimation=1; 		% fattore di decimazione segnale sampling ADC. floor(fwave*2/f_ADC-1)




% fwave=80e3; %[Hz] desired frequency of reference signal wave
% amp_wave=1.0d0; %desired ratio actual ptv of reference wave/wave_max_ptv (1=full range)
% ADC_sampling_pos=3.0643697e-6 %[s] desired time before the end of wave cycle at which the ADC sample starts
% %desired_capsens_samp_pos   double  2.1257563d-6   ;[s] desired time before the end of wave cycle at which the capsens microswitch sample starts
% ADC_sampling_duration=0.56e-6 %[s] desired temporal length of ADC sample
% %desired_capsens_samp_len   double  1.3678571d-6  ;[s] desired temporal length of capsens microswitch sample
% ADC_decimation=2; %wave_freq/ADC_sampling_rate ratio

%desired_delay_dac          double  3.2800000d-6   ;[s] delay of start of DSP-mem-to-DAC transfert from main 




%sovvrascrivo i parametri con i valori passati
if nargin==1
    inputStruct=varargin{1};
    %assegnamento valori
    for field=fieldnames(inputStruct)'
        field=cell2mat(field);
        if(exist(field))
        eval([field,'=inputStruct.',field,';']);
        else
            error([field,' does not exist']);
        end;
    end;
end;


% trasformazione guadagno 
gain_int=round(gain*128);
if gain_int < -128, gain_int=-128; end;
if gain_int > 127, gain_int=127; end;
if gain_int<0, gain_int=256+gain_int; end;

% dati  fissi fer filtro
if nf==1,
	w1=1;	% frequenza di taglio del filtro normalizzato
	cf1=[1 1]; % coefficienti filtro normalizzati
elseif nf==2,
	w1=1.3617;	% frequenza di taglio del filtro normalizzato
	cf1=[1 3 3]; % coefficienti filtro normalizzati
elseif nf==3,
	w1=1.7557;	% frequenza di taglio del filtro normalizzato
	cf1=[1 6 15 15]; % coefficienti filtro normalizzati
elseif nf==4,
	w1=2.1139;	% frequenza di taglio del filtro normalizzato
	cf1=[1 10 45 105 105]; % coefficienti filtro normalizzati
else
	disp('Ordine filtro errato !');
	return;
end;

% calcola la vera frequenza dell'onda di uscita, 
% considerando che una semionda corrisponda ad un numero intero di colpi di clk/2.
% Nota: clk/2 si deve al fatto che la macchina a stati utilizza half_freq_counter, in modo
% da poter riportare a 1 siggen_cs_n tra un ciclo e l'altro

fclk=fclk/2;
fwave_int=.5*fclk/ceil(.5*fclk/fwave);
sampling_pos_int=floor(sampling_pos*fclk);
sampling_duration_int=floor(sampling_duration*fclk);
ADC_sampling_pos_int=floor(ADC_sampling_pos*fclk);
ADC_sampling_duration_int=floor(ADC_sampling_duration*fclk);

% similmente, calcola la vera ampiezza dell'onda di uscita, 
% mediando il segnale a meta' del range del dac
amp_wave_int=floor(2^(dac_bits-1)*amp_wave)/2^(dac_bits-1);

% colpi di clock per meta' semionda
half_wave_ticks=fclk/fwave_int/2;

sampling_start=floor(half_wave_ticks-sampling_pos_int);
sampling_stop=floor(sampling_duration_int+sampling_start);
ADC_sampling_start=floor(half_wave_ticks-ADC_sampling_pos_int);
ADC_sampling_stop=floor(ADC_sampling_duration_int+ADC_sampling_start);


tsample=1/fclk; 


% calcolo coefficienti filtro
wt=2*pi*ft;
rwtw1=wt/w1;

for i=1:nf+1,	% calcolo coeff. corretti
	cf(i)=cf1(i)*rwtw1^(i-1);
end


% shaping onda
t=[0:half_wave_ticks-1]*tsample; t=t(:);
if sin2 == 0,	% smoothed step
	out_wave=step(cf(nf+1),cf,t);
else	% sin2
	out_wave=ones(size(t));
	sin2phase=sin(0:tsample/settling_time*pi/2:pi/2).^2;
	out_wave(1:length(sin2phase))=sin2phase;
end;


% % close all
% % figure(1);
% % out_wave=round(out_wave*(2^dac_bits-1)*amp_wave_int)-2^(dac_bits-1)*amp_wave_int+2^(dac_bits-1);
% % 
% % % stampa parametri
% % fprintf(1,'Frequenza/Periodo = %f KHz - %f us\nAmpiezza picco-picco = %f V\nAnticipo sampling capsens = %f us\nDurata sampling capsens = %f us\nAnticipo sampling ADC = %f us\nDurata sampling ADC = %f us\nFrequenza/periodo sampling ADC = %f KHz - %f us\n\n',...
% % 	fwave_int/1e3,1/fwave_int*1e6,...
% % 	amp_wave_int*pkpk_amp,...
% % 	sampling_pos_int/fclk*1e6,...
% % 	sampling_duration_int/fclk*1e6,...
% % 	ADC_sampling_pos_int/fclk*1e6,...
% % 	ADC_sampling_duration_int/fclk*1e6,...
% % 	fwave_int*2/(ADC_decimation+1)/1e3,(ADC_decimation+1)/fwave_int/2*1e6);
% % 
% % 
% % t=[0:half_wave_ticks-1]*tsample; t=t(:);
% % 
% % sampling_index=find(([1:half_wave_ticks]>sampling_start & [1:half_wave_ticks]<=sampling_stop));
% % ADC_sampling_index=find(([1:half_wave_ticks]>ADC_sampling_start & [1:half_wave_ticks]<=ADC_sampling_stop));
% % plot(t*1e6,out_wave,...
% % (sampling_index-1)*tsample*1e6,ones(size(sampling_index))*max(out_wave)*1.02,'rx',...
% % (ADC_sampling_index-1)*tsample*1e6,ones(size(ADC_sampling_index))*max(out_wave)*1.04,'bx'),...
% % title('Waveform'), xlabel('Time (us)'),
% % legend('Waveform','Capsens sampling','ADC sampling');


% creazione del file di default per la RAM di configurazione (8 bytes)
% fid = fopen('SignalGenerator.mif','wt');
% fprintf(fid,'WIDTH=8;\nDEPTH=8;\nADDRESS_RADIX=HEX;\nDATA_RADIX=HEX;\n\nCONTENT BEGIN\n\n');
% fprintf(fid,...
% '\t%08.8d : %2.2x;\n\t%08.8d : %2.2x;\n\t%08.8d : %2.2x;\n\t%08.8d : %2.2x;\n\t%08.8d : %2.2x;\n\t%08.8d : %2.2x;\n\t%08.8d : %2.2x;\n\t%08.8d : %2.2x;\n',...
% 0,floor(half_wave_ticks-1),...
% 1,floor(sampling_start),...
% 2,floor(sampling_stop),...
% 3,floor(ADC_sampling_start),...
% 4,floor(ADC_sampling_stop),...
% 5,floor(ADC_decimation),...
% 6,floor(gain_int),...
% 7,0);
% fprintf(fid,'\nEND;\n');
% fclose(fid);

% creazione del file di default per la RAM di configurazione per l'engineering panel (8 bytes)
% fid = fopen('C:\Documents and Settings\Roby.BZMICROGATE\Desktop\LBT_EngPanel\SignalGenerator.dat','wt');
% fid = fopen('SignalGenerator.dat','wt');
% fprintf(fid,'0x%02.2x%02.2x%02.2x%02.2x\n0x%02.2x%02.2x%02.2x%02.2x\n',...
% floor(ADC_sampling_start),...
% floor(sampling_stop),...
% floor(sampling_start),...
% floor(half_wave_ticks-1),...
% 0,...
% floor(gain_int),...
% floor(ADC_decimation),...
% floor(ADC_sampling_stop));
% fclose(fid);

a=sprintf('0x%02.2x%02.2x%02.2x%02.2x\n0x%02.2x%02.2x%02.2x%02.2x\n',...
floor(ADC_sampling_start),...
floor(sampling_stop),...
floor(sampling_start),...
floor(half_wave_ticks-1),...
0,...
floor(gain_int),...
floor(ADC_decimation),...
floor(ADC_sampling_stop));

x(1)=hex2dec(a(3:10));
x(2)=hex2dec(a(14:21));

% floor(half_wave_ticks-1),...
% floor(sampling_start),...
% floor(sampling_stop),...
% floor(ADC_sampling_start),...
% floor(ADC_sampling_stop),...
% floor(ADC_decimation),...
% floor(gain_int),...
% 0);


% creazione del file VHDL con la LUT per la generazione dell'onda (da copiare in 
% % first=1;
% % fid = fopen('siggen_seq.vhd','wt');
% % fprintf(fid,'-- Begin waveform LUT\n');
% % oldoutwave=0;
% % for i=1:half_wave_ticks,
% % 	if out_wave(i) ~= oldoutwave,
% % 
% % % trasformazione da unsigned a signed
% % 		if out_wave(i) < 2^(dac_bits-1),
% % 			wave=2^(dac_bits-1)+out_wave(i);
% % 		else
% % 			wave=out_wave(i)-2^(dac_bits-1);
% % 		end;
% % 			
% % 		out_string=dec2bin(wave,dac_bits);
% % 		if first == 1, % non genera n_cs sul primo valore
% % 			fprintf(fid,'\t\t\t\twhen x"%2.2x" => siggen_master_wave <= "%s"; -- %3.3x %3.3d\n',...
% % 				floor(i-1),out_string,floor(wave),floor(wave));
% % 			first=0;
% % 		else
% % 			fprintf(fid,'\t\t\t\twhen x"%2.2x" => siggen_master_wave <= "%s"; siggen_cs_n <= ''0''; -- %3.3x %3.3d\n',...
% % 				floor(i-1),out_string,floor(wave),floor(wave));
% % 		end;
% % 		oldoutwave=out_wave(i);
% % 		old_i=i;
% % 	end;
% % end;
% % % aggiunge scrittura fittizia ripetendo l'ultimo valore per generare l'ultimo cs_n
% % fprintf(fid,'\t\t\t\twhen x"%2.2x" => siggen_master_wave <= "%s"; siggen_cs_n <= ''0''; -- last cs_n\n',...
% % 	floor(old_i),out_string);
% % fprintf(fid,'-- End waveform LUT\n');
% % fclose(fid);

% creazione dati per simulazione Pspice
out_wave_ps=(out_wave-mean(out_wave))*pspice_ampl/(max(out_wave)-min(out_wave))-pspice_zero;
out_wave=out_wave(:);
out_wave_ps=repmat(out_wave_ps,10,1); % dieci onde

time=((1:length(out_wave_ps))*tsample-tsample);

% out_wave=out_wave(:)';
% time=time(:)';

% to_save(1:2:length(time)*2)=time;
% to_save(2:2:length(time)*2)=out_wave;

time=time(:);
