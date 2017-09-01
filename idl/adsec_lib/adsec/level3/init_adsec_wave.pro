; $Id: init_adsec_wave.pro,v 1.6 2009/09/18 08:25:01 marco Exp $$
;
;+
;  NAME:
;   INIT_ADSEC_WAVE
;
;  PURPOSE:
;   Load filters parameter in the adsec structure.
;
;  USAGE:
;   err = INIT_ADSEC_WAVE(filename, STRUCT=struct, NOFILL=nofill)
;
;  INPUT:
;   filename(optional)  : filename of data to load. Default adsec_path.conf+'wave.txt'.
;
;  OUTPUT:
;   error               : error code.
;
;  KEYWORDS:
;   STRUCT              : structure with parameters loaded and calculated.
;   NOFILL              : do not fill the adsec structure.
;
;  COMMON BLOCKS:
;   ADSEC               : secondary adaptive base common block. A part of it will be filled.
;
;  PACKAGE:
;   LBT Adsec Libraries.
;
;  HISTORY
;   Created on 01 Dec 2004 by Armando Riccardi (MX).
;   riccardi@arcetri.astro.it
;  
;  09 Dec 2004 Marco Xompero (MX)
;   DAC shot configuration now supported.
;  10 Dec 2004 Daniela Zanotti(DZ)
;   Deleted wave_settling and added in init_adsec_elec. 
;-



Function init_adsec_wave, par0, STRUCT=struct, NOFILL=nofill, VERBOSE=verb

; common blocks definition
@adsec_common


;;=========================================================================
;; data structure loading
;;
if n_elements(par0) eq 0 then begin
    filename=filepath(ROOT=adsec_path.commons, SUB='ref_wave', "wave.txt")
    data = read_ascii_structure(filename, DEBUG=0)
endif else begin
    if test_type(par0, /STRUC, N_EL=n_el) then begin
        if test_type(par0, /STRING) then begin
            message, "Input parameter has to be a string (filename) or a structure", CONT=sc.debug eq 0B
            return, adsec_error.input_type
        endif else begin
            filename = par0
            data = read_ascii_structure(filename, DEBUG=0)
        endelse
    endif else data = par0
endelse
            
if n_elements(data) eq 0 then begin
    message, 'The configuration file "'+filename+'" does not exist or has a wrong format.'
endif
;;=========================================================================


;===========================================================================
; Definition of main parameters
;
wave_freq        = data.desired_wave_freq        ;[Hz] desired frequency of reference signal wave
wave_amp_ratio   = data.desired_wave_amp_ratio   ;desired ratio actual ptv of reference wave/wave_max_ptv (1=full range)
ADC_samp_pos     = data.desired_ADC_samp_pos     ;[s] desired time before the end of wave cycle at which the ADC sample starts
capsens_samp_pos = data.desired_capsens_samp_pos ;[s] desired time before the end of wave cycle at which the capsens microswitch sample starts
ADC_samp_len     = data.desired_ADC_samp_len     ;[s] desired temporal length of ADC sample
capsens_samp_len = data.desired_capsens_samp_len ;[s] desired temporal length of capsens microswitch sample
ADC_freq_ratio   = data.wave_ADC_freq_ratio      ;wave_freq/ADC_sampling_rate ratio
delay_dac        = data.desired_delay_dac        ;[s] desired delay DAC bumping

wc_comptime      = adsec.wc_comp_time             ;[s] worst case computation time
dsp2dac_ttime    = adsec.dsp2dac_transfer_time    ;[s] worst case transfer time from DSP to DAC

err = gen_wave(wave_freq, wave_amp_ratio, adsec.wave_settling, $
               ADC_samp_pos, capsens_samp_pos, ADC_samp_len, $
               capsens_samp_len, ADC_freq_ratio, delay_dac, $
               wc_comptime, dsp2dac_ttime, VERB=verb)
if err ne adsec_error.ok then return, err

ADC_freq = wave_freq/ADC_freq_ratio
capsens_vref  = wave_amp_ratio*adsec.wave_max_ptv ; [V] PtV voltage of capsens reference signal
sampling_time = 1d0/ADC_freq ; [s] internal loop sampling time

struct =                                           $
  {                                                $
    wavename            : file_basename(filename), $
    sampling_time       : sampling_time,           $ ; [s] internal loop sampling time
    capsens_vref        : capsens_vref,            $ ; [V] PtV cap sensor reference voltage
    wave_freq           : wave_freq,               $ ; [Hz] frequency of reference signal wave
    ADC_samp_pos        : ADC_samp_pos,            $ ; [s] time before the end of wave semi-cycle at which the ADC sample starts
    ADC_samp_len        : ADC_samp_len,            $ ; [s] desired temporal length of ADC sample
    capsens_samp_pos    : capsens_samp_pos,        $ ; [s] time before the end of wave semi-cycle at which the capsens microswitch sample starts
    capsens_samp_len    : capsens_samp_len,        $ ; [s] temporal length of capsens microswitch sample    
    delay_dac           : delay_dac,               $ ; [s] delay before dac action from DSP computation start
    delayofhalfwave     : data.delayofhalfwave     $ ;  delay of half wave between the different Siggen
}

;===========================================================================
if not keyword_set(nofill) then begin
    adsec.sampling_time    = sampling_time    ; [s] internal loop sampling time
    adsec.capsens_vref     = capsens_vref     ; [V] PtV cap sensor reference voltage
    adsec.wave_freq        = wave_freq        ; [Hz] frequency of reference signal wave
    adsec.ADC_samp_pos     = ADC_samp_pos     ; [s] time before the end of wave semi-cycle at which the ADC sample starts
    adsec.ADC_samp_len     = ADC_samp_len     ; [s] desired temporal length of ADC sample
    adsec.capsens_samp_pos = capsens_samp_pos ; [s] time before the end of wave semi-cycle at which the capsens microswitch sample starts
    adsec.capsens_samp_len = capsens_samp_len ; [s] temporal length of capsens microswitch sample    
    adsec.delay_dac        = delay_dac        ; [s] waiting time to wait for DAC bumping from DSP computation starting
    adsec.delayofhalfwave  = data.delayofhalfwave  ; delay of half wave between the different Siggen 

endif

return, adsec_error.ok

End
