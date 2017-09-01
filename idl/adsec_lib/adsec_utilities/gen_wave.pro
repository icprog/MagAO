; $Id: gen_wave.pro,v 1.8 2007/04/11 15:49:41 labot Exp $
;+
; HISTORY
;  02 Dec 2004:
;   Written by Armando Riccardi (AR) 
;   and Daniela Zanotti (DZ)
;
;  09 Dec 2004: Marco Xompero(MX)
;   Now support a configurable DAC current shot and different wave frequencies
;   among capsensing and ADC sampling
;  13 December 2004:DZ
;   Introduced adsec.wave_clock_decimation  
;  13 Dec 2004: AR
;   ADC_decimation is just incremented by one. Verbose mode corrected.
;  20 Dec 2004: MX
;   Bugs in verbose mode fixed.
;  04 July 2006: DZ and AR
;   changed dac_sampling_start computation (ADC_sampling_start + ADC_sampling_duration_int + ADCacqtime_int) 
;   03 April 2007, MX & DZ
;   Substituted the old variable lenghthalfwave with delayofhalfwave to manage the wave delay between different crates.
;   Added the ticks2add keyword.   
;-
;
; Script per la generazione della sequenza di uscita per il DAC del signal generator.
; Genera automaticamente un file siggen_seq.vhd con la parte del VHDL di competenza,
; il file SignalGenerator.mif e SignalGenerator.dat con i parametri di default per la RAM di configurazione
; ed un file S:\Progetti\Engineering\Lbt\Ps\Sig_gen\istim.dat per la simulazione PSPICE 
;
; Il dato corrispondente all'acquisition time + conversion time dell'ADC viene ritenuto ASSOLUTO e pari a 3.2 [us](la variabile e' ADCacqtime)
;

; dati di ingresso

Function gen_wave, wave_freq, wave_amp_ratio, wave_settling, ADC_samp_pos, capsens_samp_pos, $
                   ADC_samp_len, capsens_samp_len, ADC_freq_ratio, delay_dac, wc_comp_time, $
                   dsp2dac_transfer_time, fpga_vec, TICKS2ADD=ticks2add, VERBOSE=verb
@adsec_common						 

if n_elements(ticks2add) eq 0 then ticks2add = 255L

ADCacqtime = adsec.adc_acq_time

fclk     = adsec.clock_freq     ;[Hz] frequenza del clock della macchina a stati
dac_bits = adsec.wave_dac_bits  ;numero di bit del DAC  ******
pkpk_amp = adsec.wave_max_ptv   ;[V] ampiezza segnale in uscita corrispondente  full DAC range

settling_time         = wave_settling    ;durata della zona sin2. Usato solo se sin2=1
fwave                 = wave_freq        ;[Hz] frequenza dell'onda in uscita
gain                  = -wave_amp_ratio  ;ampiezza segnale, compresa tra -1 e 0.9921875
ADC_decimation        = ADC_freq_ratio+1 ;fattore di decimazione segnale sampling ADC. floor(fwave*2/f_ADC-1)
ADC_sampling_duration = ADC_samp_len     ;[s] durata dell'impulso di sampling
ADC_sampling_pos      = ADC_samp_pos
sampling_duration     = capsens_samp_len ;[s] durata dell'impulso di sampling
sampling_pos          = capsens_samp_pos
time_delay_dac        = delay_dac
wc_comptime           = wc_comp_time
dsp2dac_ttime         = dsp2dac_transfer_time

; trasformazione guadagno 
gain_int=round(gain*128);
if gain_int lt -128 then gain_int=-128
if gain_int gt  127 then gain_int=127
amp_wave = -gain_int/128.0
if gain_int lt    0 then gain_int=256+gain_int

; calcola la vera frequenza dell'onda di uscita, 
; considerando che una semionda corrisponda ad un numero intero di colpi di clk/2.
; Nota: clk/2 si deve al fatto che la macchina a stati utilizza half_freq_counter, in modo
; da poter riportare a 1 siggen_cs_n tra un ciclo e l'altro
fclk = fclk/adsec.wave_clock_decimation
fwave_int = (fclk/adsec.wave_clock_decimation)/ceil(fclk/fwave/adsec.wave_clock_decimation) 
sampling_pos_int = floor(sampling_pos*fclk)
sampling_duration_int = floor(sampling_duration*fclk)
ADC_sampling_pos_int = floor(ADC_sampling_pos*fclk)
ADC_sampling_duration_int = floor(ADC_sampling_duration*fclk)
ADCacqtime_int = floor(ADCacqtime*fclk)


; similmente, calcola la vera ampiezza dell'onda di uscita, 
; mediando il segnale a meta' del range del dac
amp_wave_int = double(floor(2L^(dac_bits-1)*amp_wave))/2L^(dac_bits-1);

; colpi di clock per meta' semionda
half_wave_ticks = fclk/fwave_int/adsec.wave_clock_decimation

sampling_start = floor(half_wave_ticks-sampling_pos_int);
sampling_stop = floor(sampling_duration_int+sampling_start);
ADC_sampling_start = floor(half_wave_ticks-ADC_sampling_pos_int);
ADC_sampling_stop = floor(ADC_sampling_duration_int+ADC_sampling_start);

tsample=1d0/fclk
time_delay_dac_length = floor(delay_dac/tsample)
dac_start = ADC_sampling_start + ADC_sampling_duration_int + ADCacqtime_int
time_delay_dac = time_delay_dac_length*tsample

wave_settling         = round(settling_time/tsample)*tsample
wave_freq             = fwave_int
wave_amp_ratio        = amp_wave_int
ADC_samp_len          = ADC_sampling_duration_int/fclk
ADC_samp_pos          = ADC_sampling_pos_int/fclk
capsens_samp_len      = sampling_duration_int/fclk
capsens_samp_pos      = sampling_pos_int/fclk
delay_dac             = time_delay_dac

ADC_freq              = fwave_int/ADC_freq_ratio
sampling_time         = 1d0/ADC_freq ; [s] internal loop sampling time

dt = 1/wave_freq/2
if ADC_samp_pos ge dt then begin
    message, "ADC sampling starts too early.", CONT=sc.debug eq 0B
    return, adsec_error.generic_error
endif
if ADC_samp_pos lt ADC_samp_len then begin
    message, "ADC sampling ends too late.", CONT=sc.debug eq 0B
    return, adsec_error.generic_error
endif
if (ADC_samp_len lt adsec.min_ADC_samp_len) or (ADC_samp_len gt adsec.max_ADC_samp_len) then begin
    message, "ADC sampling length outside allowed range (" $
             +strtrim(adsec.min_ADC_samp_len*1e9,2)+"ns<=len<=" $
             +strtrim(adsec.max_ADC_samp_len*1e9,2)+"ns)", CONT=sc.debug eq 0B
    return, adsec_error.generic_error
endif
if capsens_samp_pos ge dt then begin
    message, "Microswitch capsens sampling starts too early.", CONT=sc.debug eq 0B
    return, adsec_error.generic_error
endif
if capsens_samp_pos lt capsens_samp_len then begin
    message, "Microswitch capsens sampling ends too late.", CONT=sc.debug eq 0B
    return, adsec_error.generic_error

endif
if time_delay_dac lt wc_comptime then begin
   message, "Dac action starts too early!.", CONT=sc.debug eq 0B
   return, adsec_error.generic_error
endif

if time_delay_dac gt (sampling_time - dsp2dac_ttime) then begin
; if time_delay_dac gt sampling_time then begin
   message, "Dac action starts too late!.", CONT=sc.debug eq 0B
   return, adsec_error.generic_error
endif



if keyword_set(verb) then begin
; stampa parametri
    print, "Frequenza [kHz]=", fwave_int/1e3
    print, "Periodo [us]=", 1/fwave_int*1e6
    print, "Ampiezza picco-picco [V]=", amp_wave_int*pkpk_amp
    print, "Anticipo sampling capsens [us]=", sampling_pos_int/fclk*1e6
    print, "Durata sampling capsens [us]=", sampling_duration_int/fclk*1e6
    print, "Anticipo sampling ADC [us]=", ADC_sampling_pos_int/fclk*1e6
    print, "Durata sampling ADC [us]=", ADC_sampling_duration_int/fclk*1e6
    print, "Frequenza sampling ADC [kHz]=", fwave_int*2/(ADC_decimation)/1e3
    print, "Periodo sampling ADC [us]=", (ADC_decimation)/fwave_int/2*1e6
    print, "Wait time del DAC [us]=", time_delay_dac
    print, "Worst case computation time from first interrupt [us]=", wc_comptime
endif

;t=dindgen(half_wave_ticks)*tsample
;t=transpose(t)

if keyword_set(verb) then begin
   
   n_waves = 2*ADC_freq_ratio
   n_half_waves = 2*n_waves
   wave_ticks = half_wave_ticks *n_waves
   maxticks = half_wave_ticks * n_half_waves

   sampling_index     = where(dindgen(half_wave_ticks)+1 gt sampling_start and dindgen(half_wave_ticks)+1 le sampling_stop,ccs)
   if ccs eq 1 then sampling_index = sampling_index[0]

   ADC_sampling_index = where(dindgen(half_wave_ticks)+1 gt ADC_sampling_start and dindgen(half_wave_ticks)+1 le ADC_sampling_stop,cca)
   if cca eq 1 then ADC_sampling_index = ADC_sampling_index[0]

   dac_index = where(dindgen(2*half_wave_ticks*adsec.wave_clock_decimation)+1 eq dac_start,ccd)
   if ccd eq 1 then dac_index = dac_index[0]
   sampling_index = (rebin((dindgen(1,n_half_waves)+1) ## half_wave_ticks,ccs,n_half_waves, /SAMP) + rebin(sampling_index,ccs, n_half_waves, /SAMP)) mod maxticks

   even = lindgen(n_waves)*2 
   odd = lindgen(n_waves)*2 +1 

   sampling_index_even = reform(sampling_index[*,even], n_elements(sampling_index[*,even]))
   sampling_index_odd  = reform(sampling_index[*,odd], n_elements(sampling_index[*,odd]))

   ADC_sampling_index = (rebin((dindgen(1,n_waves)+1) ## wave_ticks,cca,n_waves, /SAMP) + rebin(ADC_sampling_index,cca,n_waves, /SAMP)) mod maxticks
   ADC_sampling_index = reform(ADC_sampling_index, n_elements(ADC_sampling_index))

   dac_index = (rebin((dindgen(1, n_waves)+1) ## wave_ticks, ccd, n_waves, /SAMP) + replicate(dac_index,1,2)) mod maxticks
   dac_index = reform(dac_index, n_elements(dac_index))

   ; shaping onda
   t = dindgen(half_wave_ticks)*tsample
   in=replicate(1d0, half_wave_ticks)
   out_wave  = replicate(1d0, n_elements(t)) 
   sin2phase = sin(dindgen(round(settling_time/tsample))/(settling_time/tsample)*!DPI/2)^2
   out_wave[0:n_elements(sin2phase)-1]=sin2phase
   out_wave_half = round(out_wave*(2^dac_bits-1)*amp_wave_int)-2^(dac_bits-1)*amp_wave_int+2^(dac_bits-1)
   out_wave = [out_wave_half, max(out_wave_half)-out_wave_half]
   out_wave = reform(rebin(out_wave, n_elements(out_wave), n_waves, /SAMP), n_elements(out_wave)*n_waves)

   t = dindgen(maxticks)*tsample

   wmax = max(out_wave)
   wmin = min(out_wave)

   plot, t*1e6, out_wave, TITLE='Waveform:Red=Capsens-samp,Blue=ADC-samp, Greem=DAC-start', XTITLE='Time (us)', XS=17

   plots, (sampling_index_odd-1)*tsample*1e6, replicate(1d0, n_elements(sampling_index_odd))*wmax+0.02*wmax, PSYM=7, COLOR='0000FF'XL
   plots, (sampling_index_even-1)*tsample*1e6, replicate(1d0, n_elements(sampling_index_even))*wmin+0.02*wmax, PSYM=7, COLOR='0000FF'XL


   plots, (ADC_sampling_index-1)*tsample*1e6,replicate(1d0,n_elements(ADC_sampling_index))*wmax+0.04*wmax,PSYM=7, COLOR='FF0000'XL
   plots, (dac_index-1)*tsample*1e6,replicate(1d0,n_elements(dac_index))*wmax+0.04*wmax,PSYM=2, COLOR='00FF00'XL


endif


; creazione del file di default per la RAM di configurazione per l'engineering panel (8 bytes)
str1= string(floor(ADC_sampling_start),FORMAT="(Z2.2)")
str1 = str1 +string(floor(sampling_stop),FORMAT="(Z2.2)")
str1 = str1 +string(floor(sampling_start),FORMAT="(Z2.2)")
;str1 = str1 +string(floor(half_wave_ticks-1),FORMAT="(Z2.2)")
str1 = str1 +string(ticks2add,FORMAT="(Z2.2)")
str2= '00' +string(floor(gain_int),FORMAT="(Z2.2)")
str2=  str2 +string(floor(ADC_decimation),FORMAT="(Z2.2)")
str2=  str2 +string(floor(ADC_sampling_stop),FORMAT="(Z2.2)")

if keyword_set(verb) then begin
    print, str1
    print, str2
endif
var1=0UL
reads, str1, var1, FORMAT="(Z8.8)"
var2=0UL
reads, str2, var2, FORMAT="(Z8.8)"
fpga_vec = [var1, var2]


return, adsec_error.ok
end
