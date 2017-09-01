;$Id: test_ff.pro,v 1.1 2007/04/12 13:42:23 labot Exp $$
;+
;   NAME:
;       TEST_FF
;
;   PURPOSE:
;    ROUTINE TO TEST FEEDFORWARD COMMANDS.
;
;   NOTE:
;
;   USAGE:
;    test_ff,INTEGRATOR_MODES = intmodes, NO_TH=no_th, N_FRAMES=n_frames,
;
;
;   INPUT:
;
;   OUTPUT:
;       None.
;
;   KEYWORDS:
;   INTEGRATOR_MODES    = enable integrator modes.
;   NO_TH               = disable thresholds
;
;
;    HISTORY
;
;    03 April 2007, Daniela Zanotti(DZ)
;-

Pro test_ff, NO_TH=no_th, N_FRAMES=n_frames
 
    @adsec_common
    common inc_fc_block, c1_old, old_modes, delaya, delayb
    ;block = 0
    err = set_cmd2bias()
    if err ne adsec_error.ok then message, "Error encountered: "+strtrim(err,2)

    err = clear_dl()
    if err ne adsec_error.ok then message, "Error encountered: "+strtrim(err,2)

    dummy = replicate(1e-6, adsec.n_actuators)
    
    if keyword_set(NO_TH) then dummy[indgen(adsec.n_actuators)] = 1e6
    dummy = replicate(1e6, adsec.n_actuators)
    err = set_skip_par(dummy, /MODE, /DELTA)
    if err ne adsec_error.ok then message, "Error encountered: "+strtrim(err,2)

    dummy = replicate(10e-6, adsec.n_actuators)
    dummy[adsec.dummy_act] = 1e6
    if keyword_set(NO_TH) then dummy[indgen(672)] = 1e6
    err = set_skip_par(dummy, /MODE, /MAX)
    if err ne adsec_error.ok then message, "Error encountered: "+strtrim(err,2)

    dummy = replicate(25e-6, adsec.n_actuators)
    dummy[adsec.dummy_act] = -1e6 
    if keyword_set(NO_TH) then dummy[indgen(adsec.n_actuators)] = -1e6
    err = set_skip_par(dummy, /CMD, /MIN)
    if err ne adsec_error.ok then message, "Error encountered: "+strtrim(err,2)

    dummy = replicate(110e-6, adsec.n_actuators)
    dummy[adsec.dummy_act] = 1e6
    if keyword_set(NO_TH) then dummy[indgen(adsec.n_actuators)] = 1e6
    err = set_skip_par(dummy, /CMD, /MAX)
    if err ne adsec_error.ok then message, "Error encountered: "+strtrim(err,2)

    dummy = replicate(1e-6, adsec.n_actuators)
    dummy[adsec.dummy_act] = 1e6
    if keyword_set(NO_TH) then dummy[indgen(adsec.n_actuators)] = 1e6
    err = set_skip_par(dummy, /CMD, /DELTA)
    if err ne adsec_error.ok then message, "Error encountered: "+strtrim(err,2)

    dummy = replicate(-0.5, adsec.n_actuators)
    dummy[adsec.dummy_act] = -1e6
    if keyword_set(NO_TH) then dummy[indgen(adsec.n_actuators)] = -1e6
    err = set_skip_par(dummy, /FORCE, /MIN)
    if err ne adsec_error.ok then message, "Error encountered: "+strtrim(err,2)

    dummy = replicate(0.5, adsec.n_actuators)
    dummy[adsec.dummy_act] = 1e6
    if keyword_set(NO_TH) then dummy[indgen(adsec.n_actuators)] = 1e6
    err = set_skip_par(dummy, /FORCE, /MAX)
    if err ne adsec_error.ok then message, "Error encountered: "+strtrim(err,2)

    c1_old = 0.0
    old_modes = fltarr(adsec.n_actuators)
    delaya = replicate(0.0, adsec.n_actuators)
    delayb = replicate(0.0, adsec.n_actuators)


    disturb_mode = 55; 55 for 672
    mode=readfits("../adsec_lib/test/mode_"+string(disturb_mode,FORMAT="(I2.2)")+".fits")
    dlen = long(rtr.disturb_len)
    t = findgen(dlen)/20*2*!PI
    amp = sin(t)
    hist = rebin(amp,dlen,adsec.n_actuators) $
             *rebin(reform(mode,1,adsec.n_actuators),dlen,adsec.n_actuators)
    hist = float(hist)
    dhist = [fltarr(1,adsec.n_actuators),hist[1:dlen-1,*]-hist[0:dlen-2,*]]
    
    mod_hist = float(transpose(adsec.ff_p_svec)##hist)
    dmod_hist = [fltarr(1,adsec.n_actuators),hist[1:dlen-1,*]-hist[0:dlen-2,*]]
    
    if rtr.n_modes2correct ne adsec.n_actuators then $
      message, "num of modes has to be equal to num of actuators for the test"
    
    b0 = fltarr(rtr.n_slope,adsec.n_actuators)
    b_delay = fltarr(rtr.n_slope,rtr.n_modes2correct)       ;(1 delay slopes)
    a_delay = fltarr(adsec.n_actuators,adsec.n_actuators,2) ;(2 delay modes)
    m2c = fltarr(rtr.n_modes2correct,adsec.n_actuators)


    b0[0,0] = identity(adsec.n_actuators)
    m2c[0,0] = identity(adsec.n_actuators)
    
    
    g_gain = replicate(1.0, adsec.n_actuators)
 
    err = set_b0_matrix(b0)
    if err ne adsec_error.ok then message, "Error encountered: "+strtrim(err,2)

    err = set_b_delay_matrix(b_delay)
    if err ne adsec_error.ok then message, "Error encountered: "+strtrim(err,2)

    err = set_a_delay_matrix(a_delay)
    if err ne adsec_error.ok then message, "Error encountered: "+strtrim(err,2)
    
    err = set_m2c_matrix(m2c)
    if err ne adsec_error.ok then message, "Error encountered: "+strtrim(err,2)

    err = set_g_gain(sc.all, g_gain)
    if err ne adsec_error.ok then message, "Error encountered: "+strtrim(err,2)

    ;cleaning of all gain block
    err = set_fc_gain(/all)
    if err ne adsec_error.ok then message, 'I Guadagni non sono stati puliti'

    ; clean all the int control current
    err = write_same_ch(sc.all, dsp_map.int_control_current, 0l, /ch)
    err = write_same_ch(sc.all, dsp_map.mean_control_current, 0l, /ch)


    cmd_preshaper = time2step_preshaper(0.9e-3, APPLIED=cmd_preshaper_sec, /VERB)
    cur_preshaper = time2step_preshaper(0.9e-3, APPLIED=cur_preshaper_sec, /VERB)

    ;set preshapers
    err = set_preshaper(sc.all, cmd_preshaper)
    if err ne adsec_error.ok then message, "ERROR"
    err = set_preshaper(sc.all, cur_preshaper, /FF)
    if err ne adsec_error.ok then message, "ERROR"

    acc_delay = ceil(0.9e-3/adsec.sampling_time)
    acc_len   = long(1e-3/adsec.sampling_time-acc_delay)

    ;set the accumulators
    err = set_acc(acc_len,acc_delay)
    if err ne adsec_error.ok then message, "ERROR"
    wait, 0.1
    err = start_acc(/WAIT)
    if err ne adsec_error.ok then message, "ERROR"

    err = set_disturb(/DISABLE)
    if err ne adsec_error.ok then message, "ERROR"

 ;   print, set_diagnostic(/master)
;    if n_elements(n_frames) gt 0 then numframes = n_frames < (dlen-1) else numframes = dlen -1
    ; pseudo deriv

; the first reading

    
    for i=0, 5 do begin

        slopes = [reform(hist[i,*]),fltarr(rtr.n_slope-adsec.n_actuators)]
        inc_fc, slopes, hist[i,*], /SWITCH, /DOCH
        wait, 0.1
     endfor

    err = set_g_gain(sc.all, /reset)
    if err ne adsec_error.ok then message, "Error encountered: "+strtrim(err,2)
    err = clear_ff()
    if err ne adsec_error.ok then message, "Error cleaning the ff current"

end
            
