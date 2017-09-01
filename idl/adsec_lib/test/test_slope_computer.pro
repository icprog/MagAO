;$Id: test_slope_computer.pro,v 1.2 2008/02/21 10:26:02 labot Exp $
;+
;HISTORY
; written by D.Zanotti(DZ)
; 
;TO DO BEFORE
; 1KHZ
;Set the mirror, raising gain 
;test slope computer con le soglie (set_safe_skip)
; Disable Pendig
;Slope computer start to send
;Verify the frequency of slopes
;print, test_skip_frame() 
;print, set_diagnostic(over=0.0025,/master) ;400Hz
;print, test_skip_frame()
;print, test_skip_frame(); deve andare avanti solo il WFSFrames
;stop Slope computer
;Verify the engage 
;print, test_skip_frame()
;print, test_skip_frame(); deve andare avanti solo il MirrorFrames
; <700KHz
; 
;-
; 
;
Function test_slope_computer, work_freq, nsamples,INT_MODE=int_mode, OLD_OVS=old_ovs

    @adsec_common
    old_ovs = 1./rtr.oversampling_time 
    work_freq = 1000.  ; optical loop frequency, slope computer
    period = 1./work_freq
    
 
    ;TIMEHIST POSITION AND FORCE CHECKS
    err = update_status()
    if err ne adsec_error.ok then return, err
    update_panels, /NO_READ
    
    ;System setup
    ;I guadagni devono essere GIA' ESSERE SETTATI
    
    timepcmd = 0.1e-3
    timepcurr = 0.8e-3 ;0.8e-3 at 60um 0.7e-3
    acc_period = 0.9e-3 ;period * 0.8

    cmd_preshaper = time2step_preshaper(timepcmd, APPLIED=cmd_preshaper_sec, /VERB)
    cur_preshaper = time2step_preshaper(timepcurr, APPLIED=cur_preshaper_sec, /VERB)

    preshaper_sec = max([cmd_preshaper_sec,cur_preshaper_sec])
    max_settling_time = preshaper_sec

    acc_delay = ceil(max_settling_time/adsec.sampling_time)
    acc_len   = long(acc_period/adsec.sampling_time-acc_delay)


    err = set_diagnostic(OVER=0.0)
    if err ne adsec_error.ok then return, err

    print, "ACC_DELAY: ", strtrim(acc_delay,2)," (",strtrim(acc_delay*adsec.sampling_time,2),"s)"
    print, "ACC_LEN  : ", strtrim(acc_len,2)," (",strtrim(acc_len*adsec.sampling_time,2),"s)"
    print, "MAX FREQ ALLOWED: ", strtrim( 1d/acc_period , 2)

    err = set_acc(acc_len,acc_delay)
    if err ne adsec_error.ok then return, err
    wait, 0.1
    err = start_acc(/WAIT)
    if err ne adsec_error.ok then return, err


    pr_cmd = ((round(adsec.preshaper_len*adsec.sampling_time/timepcmd)  > 1) $
           < dsp_const.fastest_preshaper_step)
    pr_cur = ((round(adsec.preshaper_len*adsec.sampling_time/timepcurr)  > 1) $
           < dsp_const.fastest_preshaper_step)

    err = set_preshaper(sc.all, pr_cmd)
    if err ne adsec_error.ok then return, err
    err = set_preshaper(sc.all, pr_cur, /FF)
    if err ne adsec_error.ok then return, err

    ;set up the modes/commands/forces thresholds
    err = set_safe_skip(ACT=adsec.act_w_cl) ; DA TOGLIERE IL RESET!!!!
    if err ne adsec_error.ok then return, err

    ;set up the dummy recostruction/delay/m2c matrices
    b0 = fltarr(rtr.n_slope,adsec.n_actuators)
    b_delay = fltarr(rtr.n_slope,adsec.n_actuators, rtr.n_slope_delay)       ;(1 delay slopes)
    a_delay = fltarr(adsec.n_actuators,adsec.n_actuators,rtr.n_modes_delay) ;(2 delay modes)
    if keyword_set(int_mode) then a_delay[*,*,0] = identity(adsec.n_actuators) ;(2 delay modes)
    
    b0[0,0] = identity(adsec.n_actuators)
    old_m2c_mat = *rtr.m2c_matrix
    ;new_m2c_mat = fltarr(rtr.n_modes2correct, adsec.n_actuators)
    new_m2c_mat = float(adsec.ff_p_svec)
    err = set_m2c_matrix(new_m2c_mat)
    if err ne adsec_error.ok then return, err

    g_gain = replicate(1.0, adsec.n_actuators)

    err = set_b0_matrix(b0)
    if err ne adsec_error.ok then message, "Error encountered: "+strtrim(err,2)

    err = set_b_delay_matrix(b_delay)
    if err ne adsec_error.ok then message, "Error encountered: "+strtrim(err,2)

    err = set_a_delay_matrix(a_delay)
    if err ne adsec_error.ok then message, "Error encountered: "+strtrim(err,2)

    err = set_g_gain(sc.all, g_gain)
    if err ne adsec_error.ok then message, "Error encountered: "+strtrim(err,2)

    err = set_cmd2bias()
    if err ne adsec_error.ok then message, "Error encountered: "+strtrim(err,2)

    err = update_status(samples)
    if err ne adsec_error.ok then begin
        message, "Error updating status!!",CONT= (sc.debug eq 0B)
        return, err
    endif

    sys0 = sys_status    

    err = clear_skip_counter() 
    if err ne adsec_error.ok then return, err

    ;enable master diagnostic
    ;master_freq = 30.
    ;diagn_dec = round(work_freq/master_freq)
    ;err = set_diagnostic(/MASTER, DECIMATION_MASTER=diagn_dec)
    ;if err ne adsec_error.ok then return, err

    ;err = test_skip_frame(OLDPEND=oldpend, OLDSKIP=oldskip)
    ;if err ne adsec_error.ok then return, err

    ;if work_freq gt 700 then begin 
    ;    err = enable_pending(IS_PENDING=old_pending)
    ;    if err ne adsec_error.ok then return, err
    ;    err = disable_pending()
    ;    if err ne adsec_error.ok then return, err
    ;endif
    err = enable_pending(IS_PENDING=old_pending)
    if err  ne adsec_error.ok then return, err
    ;ENABLE TIMEOUT FRAMES
    ;err = set_diagnostic(OVER=period)
    ;if err ne adsec_error.ok then return, err
    ;go to first atm position

    return, adsec_error.ok

End
