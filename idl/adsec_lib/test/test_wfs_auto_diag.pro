;$Id: test_wfs_auto_diag.pro,v 1.1 2007/04/12 14:19:01 labot Exp $$
;+
;   NAME:
;       TEST_WFS_AUTO_DIAG
;
;   PURPOSE:
;    ROUTINE TO TEST THE WFS FRAMES: FROM TIMEOUT FRAMES TO WFS FRAMES AND RETURN.
;
;   NOTE:
;
;   USAGE:
;     test_wfs_auto_diag,  buf
;
;   INPUT:
;
;   OUTPUT:
;       BUF = THE STRUCTURE WITH THE WFS COUNTER AND THE MIRROR COUNTER
;
;   KEYWORDS:
;
;
;    HISTORY
;
;    April 2007, Marco Xompero(MX)
;    April 2007, Daniela Zanotti(DZ) 
;-



Pro test_wfs_auto_diag,  buf

    no_th=1

    @adsec_common
;    common inc_fc_block, c1_old, old_modes, delaya, delayb

;    err = set_cmd2bias()
;    if err ne adsec_error.ok then message, "Error encountered: "+strtrim(err,2)

;    err = clear_dl()
;    if err ne adsec_error.ok then message, "Error encountered: "+strtrim(err,2)

;    if keyword_set(intmodes) then dummy = replicate(10e-6, adsec.n_actuators) else dummy = replicate(1e-6, adsec.n_actuators)
;    if keyword_set(NO_TH) then dummy[indgen(672)] = 1e6
;    dummy = replicate(1e6, adsec.n_actuators)
;    err = set_skip_par(dummy, /MODE, /DELTA)
;    if err ne adsec_error.ok then message, "Error encountered: "+strtrim(err,2)

;    dummy = replicate(10e-6, adsec.n_actuators)
;    dummy[adsec.dummy_act] = 1e6
;    if keyword_set(NO_TH) then dummy[indgen(672)] = 1e6
;    err = set_skip_par(dummy, /MODE, /MAX)
;    if err ne adsec_error.ok then message, "Error encountered: "+strtrim(err,2)

;    dummy = replicate(25e-6, adsec.n_actuators)
;    dummy[adsec.dummy_act] = -1e6 
;    if keyword_set(NO_TH) then dummy[indgen(672)] = -1e6
;    err = set_skip_par(dummy, /CMD, /MIN)
;    if err ne adsec_error.ok then message, "Error encountered: "+strtrim(err,2)

;    dummy = replicate(110e-6, adsec.n_actuators)
;    dummy[adsec.dummy_act] = 1e6
;    if keyword_set(NO_TH) then dummy[indgen(672)] = 1e6
;    err = set_skip_par(dummy, /CMD, /MAX)
;    if err ne adsec_error.ok then message, "Error encountered: "+strtrim(err,2)

;    if keyword_set(intmodes) then dummy = replicate(10e-6, adsec.n_actuators) else dummy = replicate(1e-6, adsec.n_actuators)
;    dummy[adsec.dummy_act] = 1e6
;    if keyword_set(NO_TH) then dummy[indgen(672)] = 1e6
;    err = set_skip_par(dummy, /CMD, /DELTA)
;    if err ne adsec_error.ok then message, "Error encountered: "+strtrim(err,2)

;    dummy = replicate(-0.5, adsec.n_actuators)
;    dummy[adsec.dummy_act] = -1e6
;    if keyword_set(NO_TH) then dummy[indgen(672)] = -1e6
;    err = set_skip_par(dummy, /FORCE, /MIN)
;    if err ne adsec_error.ok then message, "Error encountered: "+strtrim(err,2)

;    dummy = replicate(0.5, adsec.n_actuators)
;    dummy[adsec.dummy_act] = 1e6
;    if keyword_set(NO_TH) then dummy[indgen(672)] = 1e6
;    err = set_skip_par(dummy, /FORCE, /MAX)
;    if err ne adsec_error.ok then message, "Error encountered: "+strtrim(err,2)

;    c1_old = 0.0
;    old_modes = fltarr(adsec.n_actuators)
;    delaya = replicate(0.0, adsec.n_actuators)
;    delayb = replicate(0.0, adsec.n_actuators)


;    disturb_mode = 55; 55 for 672
;    mode=readfits("../adsec_lib/test/mode_"+string(disturb_mode,FORMAT="(I2.2)")+".fits")
;    dlen = long(rtr.disturb_len)
;    t = findgen(dlen)/20*2*!PI
;    amp = sin(t)
;    hist = rebin(amp,dlen,adsec.n_actuators) $
;             *rebin(reform(mode,1,adsec.n_actuators),dlen,adsec.n_actuators)
;    hist = float(hist)
;    dhist = [fltarr(1,adsec.n_actuators),hist[1:dlen-1,*]-hist[0:dlen-2,*]]
    
;    mod_hist = float(transpose(adsec.ff_p_svec)##hist)
;    dmod_hist = [fltarr(1,adsec.n_actuators),hist[1:dlen-1,*]-hist[0:dlen-2,*]]
    
;    if rtr.n_modes2correct ne adsec.n_actuators then $
;      message, "num of modes has to be equal to num of actuators for the test"
    
;    b0 = fltarr(rtr.n_slope,adsec.n_actuators)
;    b_delay = fltarr(rtr.n_slope,rtr.n_modes2correct)       ;(1 delay slopes)
;    a_delay = fltarr(adsec.n_actuators,adsec.n_actuators,2) ;(2 delay modes)
;    m2c = fltarr(rtr.n_modes2correct,adsec.n_actuators)


;    if ~keyword_set(DISTURBANCE) then begin
;        b0[0,0] = identity(adsec.n_actuators)
;        if keyword_set(intmodes) then a_delay[*,*,0] = identity(rtr.n_modes2correct)
;        m2c[0,0] = identity(adsec.n_actuators)
;    endif
    
;    g_gain = replicate(1.0, adsec.n_actuators)
 
;    err = set_b0_matrix(b0)
;    if err ne adsec_error.ok then message, "Error encountered: "+strtrim(err,2)

;    err = set_b_delay_matrix(b_delay)
;    if err ne adsec_error.ok then message, "Error encountered: "+strtrim(err,2)

;    err = set_a_delay_matrix(a_delay)
;    if err ne adsec_error.ok then message, "Error encountered: "+strtrim(err,2)
    
;    err = set_m2c_matrix(m2c)
;    if err ne adsec_error.ok then message, "Error encountered: "+strtrim(err,2)

;    err = set_g_gain(sc.all, g_gain)
;    if err ne adsec_error.ok then message, "Error encountered: "+strtrim(err,2)

;    cmd_preshaper = time2step_preshaper(0.9e-3, APPLIED=cmd_preshaper_sec, /VERB)
;    cur_preshaper = time2step_preshaper(0.9e-3, APPLIED=cur_preshaper_sec, /VERB)

;    ;set preshapers
;    err = set_preshaper(sc.all, cmd_preshaper)
;    if err ne adsec_error.ok then message, "ERROR"
;    err = set_preshaper(sc.all, cur_preshaper, /FF)
;    if err ne adsec_error.ok then message, "ERROR"

;    acc_delay = ceil(0.9e-3/adsec.sampling_time)
;    acc_len   = long(1e-3/adsec.sampling_time-acc_delay)

;    ;set the accumulators
;    err = set_acc(acc_len,acc_delay)
;    if err ne adsec_error.ok then message, "ERROR"
;    wait, 0.1
;    err = start_acc(/WAIT)
;    if err ne adsec_error.ok then message, "ERROR"

;    ;DISTURBANCE FIRMWARE SETTINGS
;    if keyword_set(DISTURBANCE) then begin
       
;        err = set_disturb(hist, /RESET, /ENABLE) 
;        if err ne adsec_error.ok then message, "ERROR"
        
;    endif else begin
        
;       ;err = set_disturb(hist, /RESET, /DISABLE) 
;        err = set_disturb(/DISABLE)
;        if err ne adsec_error.ok then message, "ERROR"

;    endelse

;------------------------------------------------

    @adsec_common
    common inc_fc_block, c1_old, old_modes, delaya, delayb

    err = set_cmd2bias()
    if err ne adsec_error.ok then message, "Error encountered: "+strtrim(err,2)

    err = clear_dl()
    if err ne adsec_error.ok then message, "Error encountered: "+strtrim(err,2)

    if keyword_set(intmodes) then dummy = replicate(10e-6, adsec.n_actuators) else dummy = replicate(1e-6, adsec.n_actuators)
    if keyword_set(NO_TH) then dummy[indgen(672)] = 1e6
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
    if keyword_set(NO_TH) then dummy[indgen(672)] = -1e6
    err = set_skip_par(dummy, /CMD, /MIN)
    if err ne adsec_error.ok then message, "Error encountered: "+strtrim(err,2)

    dummy = replicate(110e-6, adsec.n_actuators)
    dummy[adsec.dummy_act] = 1e6
    if keyword_set(NO_TH) then dummy[indgen(672)] = 1e6
    err = set_skip_par(dummy, /CMD, /MAX)
    if err ne adsec_error.ok then message, "Error encountered: "+strtrim(err,2)

    if keyword_set(intmodes) then dummy = replicate(10e-6, adsec.n_actuators) else dummy = replicate(1e-6, adsec.n_actuators)
    dummy[adsec.dummy_act] = 1e6
    if keyword_set(NO_TH) then dummy[indgen(672)] = 1e6
    err = set_skip_par(dummy, /CMD, /DELTA)
    if err ne adsec_error.ok then message, "Error encountered: "+strtrim(err,2)

    dummy = replicate(-0.5, adsec.n_actuators)
    dummy[adsec.dummy_act] = -1e6
    if keyword_set(NO_TH) then dummy[indgen(672)] = -1e6
    err = set_skip_par(dummy, /FORCE, /MIN)
    if err ne adsec_error.ok then message, "Error encountered: "+strtrim(err,2)

    dummy = replicate(0.5, adsec.n_actuators)
    dummy[adsec.dummy_act] = 1e6
    if keyword_set(NO_TH) then dummy[indgen(672)] = 1e6
    err = set_skip_par(dummy, /FORCE, /MAX)
    if err ne adsec_error.ok then message, "Error encountered: "+strtrim(err,2)

    c1_old = 0.0
    old_modes = fltarr(adsec.n_actuators)
    delaya = replicate(0.0,1, adsec.n_actuators)
    delayb = replicate(0.0,1, adsec.n_actuators)


    disturb_mode = 4; 55 for 672
    mode=readfits("../adsec_lib/test/mode672_"+string(disturb_mode,FORMAT="(I3.3)")+".fits")
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
    b_delay = fltarr(rtr.n_slope,adsec.n_actuators)       ;(1 delay slopes)
    a_delay = fltarr(adsec.n_actuators,adsec.n_actuators,2) ;(2 delay modes)
    m2c = fltarr(adsec.n_actuators,adsec.n_actuators)


    if ~keyword_set(DISTURBANCE) then begin
        b0[0,0] = identity(adsec.n_actuators)
        if keyword_set(intmodes) then a_delay[*,*,0] = identity(rtr.n_modes2correct)
        m2c[0,0] = identity(rtr.n_modes2correct)
    endif
    
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
    err = set_acc(6,64)
    if err ne adsec_error.ok then message, "ERROR"
    wait, 0.1
    err = start_acc(/WAIT)
    wait, 1
    if err ne adsec_error.ok then message, "ERROR"

    ;DISTURBANCE FIRMWARE SETTINGS
    if keyword_set(DISTURBANCE) then begin
       
        err = set_disturb(hist, /RESET, /ENABLE) 
        if err ne adsec_error.ok then message, "ERROR"
        
    endif else begin
        
       ;err = set_disturb(hist, /RESET, /DISABLE) 
        err = set_disturb(/DISABLE)
        if err ne adsec_error.ok then message, "ERROR"

    endelse

; Aggancio WFS

; Disable Pending
  ;  err = disable_pending()
; Disable Frames Timeout

;Enable WFS
;Wait 
;Enable WFS

;-------------------------------------------------
    varchanged = setdiagnparam(process_list.fastdiagn.msgd_name, 'MirrorFrameCounter WFSFrameCounter', -1, -1, MEAN=10, TIM=2)
    if varchanged eq 0 then message, "ERROR"

    err= set_diagnostic(/master)
    if err ne adsec_error.ok then message, "ERROR"
    err= set_diagnostic(over=5e-2)
    if err ne adsec_error.ok then message, "ERROR"
    wait, 5

; Aggancio WFS

; Disable Pending
   err = disable_pending()
    if err ne adsec_error.ok then message, "ERROR"

; Disable Frames Timeout
    err= set_diagnostic(over=0.0)
    if err ne adsec_error.ok then message, "ERROR"

;Enable WFS
;Wait
;Enable Timeout



    for i=0, 300 do begin
        slopes = [reform(hist[i,*]),fltarr(rtr.n_slope-adsec.n_actuators)]
        ;err=set_diagnostic(Decimation_master=2)
        inc_fc, slopes, hist[i,*], /SWITCH
        if i eq 10 then begin
             ;abilito frames timeout   
              err= set_diagnostic(/master)
              if err ne adsec_error.ok then message, "ERROR"
             err= set_diagnostic(over=0.5e-1)
             if err ne adsec_error.ok then message, "ERROR"
        endif
    endfor

    ;wait, 3
    
    err = set_diagnostic(over=0.0)
    if err ne adsec_error.ok then message, "ERROR"

    err = set_g_gain(sc.all, /reset)
    if err ne adsec_error.ok then message, "Error encountered: "+strtrim(err,2)

    err = getdiagnbuffer(process_list.fastdiagn.msgd_name, 'MirrorFrameCounter WFSFrameCounter', -1, -1, buf, TIM=2)    
    if err ne adsec_error.ok then message, "Error encountered: "+strtrim(err,2)

    
    ndc = exdiagbuf ("MirrorFrameCounter", buf)
    
    ffc = exdiagbuf ("WFSFrameCounter", buf)


    return


end
            
