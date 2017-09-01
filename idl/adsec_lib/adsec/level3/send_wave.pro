;$Id: send_wave.pro,v 1.9 2008/02/05 10:39:12 labot Exp $$
;+
;  NAME:
;   SEND_WAVE
;  
;  USAGE:
;   err = send_wave(VER=ver)
;
;  PURPOSE:
;   Send reference wave to signal generator board.
;
;  INPUT:
;   None.
;
;  OUTPUT:
;   Error code.
;  
;  KEYWORD:
;   VER=ver : verbose, print some information.
;
;  HISTORY:
;   Created by Armando Riccardi(AR) on Nov 2004.
;   
;  09 Dec 2004:
;   DAC shot configuration now supported. 
;  Added counter_delay_dac, unsigned counter of 32bit.
;  Introduced adsec.dsp_clock_decimation. 
;  13 Dec 2004:
;  Deleted bcus.Fixed bug in downloading delay_dac. 
;  20 Mar 2007, MX & DZ
;   Updated at the new siggen configuration
;  03 April 2007, MX & DZ
;   New siggen inizialization for the multicrate case.
;   Insert the exact delay for the P45(only one crate)
;   added adsec.delayofhalfwave
;    
;-

Function send_wave, VERBOSE=ver 

   @adsec_common

    ;err = disable_watchdog(/DSP)
    ;if err ne adsec_error.ok then return, err

    ADC_freq_ratio = round(adsec.wave_freq/(1/adsec.sampling_time))

    err = gen_wave(adsec.wave_freq, adsec.capsens_vref/adsec.wave_max_ptv, $
                  adsec.wave_settling, adsec.ADC_samp_pos, adsec.capsens_samp_pos, $
                  adsec.ADC_samp_len, adsec.capsens_samp_len, $
                  ADC_freq_ratio, adsec.delay_dac,adsec.wc_comp_time,$
                  adsec.dsp2dac_transfer_time, vect, VER=ver)

    print, "I'm writing...."

    err = manage_interrupt(/DISABLE)   
    if err ne adsec_error.ok then return, err

    err= write_same_board(sc.all, 0L, vect, /SIGGEN, SET_PM=siggen_datasheet.fpgasig_mem_id)
    if err ne adsec_error.ok then return, err
    ;il delay_dac e' un counter unsigned a 32 bit (NON float) che viene
    ;decrementato alla frequenza interna del DSP = 4 x master_clk 
    ; (quindi circa 242.86MHz)
    counter_delay_dac = ulong(round(adsec.delay_dac*adsec.clock_freq*adsec.DSP_clock_decimation, /L64))
    err= write_same_dsp(sc.all, dsp_map.delay_dac, counter_delay_dac, /CHECK)
    if err ne adsec_error.ok then return, err

    err = manage_interrupt(/ENABLE)   
    if err ne adsec_error.ok then return, err

    wait, 0.01   ;  questa pausa deve esserci per dare il tempo alle siggen di parlarsi

    err = manage_interrupt(/DISABLE)   
    if err ne adsec_error.ok then return, err
    
;    delay fot P45 toadd = [40l], for 672a toadd = [79l, 40l, 39l, 39l, 40l, 41l]
    toadd = adsec.delayofhalfwave

    err = gen_wave(adsec.wave_freq, adsec.capsens_vref/adsec.wave_max_ptv, $
                  adsec.wave_settling, adsec.ADC_samp_pos, adsec.capsens_samp_pos, $
                  adsec.ADC_samp_len, adsec.capsens_samp_len, $
                  ADC_freq_ratio, adsec.delay_dac,adsec.wc_comp_time,$
                  adsec.dsp2dac_transfer_time, vect, VER=ver, TICKS=0L)
    one = vect[1]
    base = vect[0]

    for i=0, adsec.n_crates -1 do begin
    
        vect2 = [base + toadd[i], one]
        printhex, vect2[0]
        printhex, vect2[1]
        err= write_same_board(i, 0L, vect2, /SiGGEN, SET_PM=siggen_datasheet.fpgasig_mem_id)
        if err ne adsec_error.ok then return, err

    endfor


    err = manage_interrupt(/ENABLE)
    if err ne adsec_error.ok then return, err


    ;err = enable_watchdog(/DSP)
    ;if err ne adsec_error.ok then return, err
    !AO_STATUS.REF_WAVE = adsec.wavename

    print, "Done."
    return, update_rtdb()

End
