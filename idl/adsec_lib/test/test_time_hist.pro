Function test_time_hist, work_freq, nsamples, SWITCHB=switchb

    @adsec_common

    ;History generation
    amp = 100e-9
    freq = 1. ;[Hz]   ; ; la frequenza qui e' sbagliata..... bisogna sistemarla con quella sotto...
    period=1./work_freq
    modulation = [findgen(rtr.disturb_len/2)*2, rtr.disturb_len-findgen(rtr.disturb_len/2)*2]
    modulation /= max(modulation)
    hist = sin(2*!pi*freq*findgen(rtr.disturb_len)*period)
    mode = (adsec.ff_p_svec[0,*])/max(abs(adsec.ff_p_svec[0, adsec.act_w_pos]))
    time_hist = float(rebin(hist*modulation*amp, rtr.disturb_len, adsec.n_actuators, /SAMPLE) * $
                rebin(mode, rtr.disturb_len, adsec.n_actuators, /SAMPLE))

    
   ; set preshaper lenti e accumulatori

   ; TO DEBUG!!!!!!!!
   ; clear all feedforward force

    ;disable disturbance
    err = set_disturb(/DISABLE)
    if err ne adsec_error.ok then return, err

    err=clear_ff(/RESET_FILTER)
    if err ne adsec_error.ok then begin
        message, "Error clearing feed-forward currents!!",CONT= (sc.debug eq 0B)
        return, err
    endif

    err = clear_rtr()
    if err ne adsec_error.ok then return, err


    ;end clear
   
    timepcmd = 35e-3  ;???troppo lenti???
    timepcurr = 35e-3
    acc_period = 30e-3

    cmd_preshaper = time2step_preshaper(timepcmd, APPLIED=cmd_preshaper_sec, /VERB)
    cur_preshaper = time2step_preshaper(timepcurr, APPLIED=cur_preshaper_sec, /VERB)

    preshaper_sec = max([cmd_preshaper_sec,cur_preshaper_sec])
    max_settling_time = preshaper_sec

    acc_delay = ceil(max_settling_time/adsec.sampling_time)
    acc_len   = long(acc_period/adsec.sampling_time-acc_delay)

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
    err = get_preshaper(sc.all, pr_cmd_old)
    if err ne adsec_error.ok then return, err
    err = get_preshaper(sc.all, pr_cur_old, /FF)
    if err ne adsec_error.ok then return, err

    err = set_preshaper(sc.all, pr_cmd)
    if err ne adsec_error.ok then return, err
    err = set_preshaper(sc.all, pr_cur, /FF)
    if err ne adsec_error.ok then return, err
    
; ctrl force 
    dcomm = reform(time_hist[0,*]) 
    update_panels, /OPT
    dforce = mamano(adsec.ff_matrix,dcomm)
    tot_force = sys_status.current+dforce
    dummy =  max(abs(tot_force[adsec.adsec.act_w_curr]), idx_peak)
    if dummy gt abs(4.5*mean(sys_status.current[adsec.act_w_curr])) then $
      message,'Current required to flatten is too high!'

    ; apply command  
    answ = dialog_message(["Peak force: "+strtrim(tot_force[idx_peak],2) $
                           ,"Do you want to apply it?"], /QUEST)
    if strlowcase(answ) eq "no" then begin

        return, adsec_error.generic_error

    endif else begin

        err = set_delta_position_vector(dcomm)
        if err ne adsec_error.ok then return, err

        err = start_ff()
        if err ne adsec_error.ok then return, err

        ;update_delta_position, dcomm_filt, /ff
        wait,adsec.time_secure_factor*time_preshaper() ; 1
    
        wait, 0.1
        update_panels, /OPT
        sys_status_fin = sys_status
        if max(abs(sys_status_fin.current)) gt 4.5*mean(abs(sys_status_fin.current)) then begin
		err = rip()
		message, 'Current required is too high! Mirror was reset.', /info
                return, err
        endif
    endelse

; END TO DEBUG!!!!!!!!!!!!!!!!!!!!!!!!!!
;System setup
    ;I guadagni e i preshaper deve GIA' ESSERE SETTATI
    timepcmd = 0.1e-3
    timepcurr = 0.7e-3
    acc_period = 0.8e-3 ;period * 0.8

    cmd_preshaper = time2step_preshaper(timepcmd, APPLIED=cmd_preshaper_sec, /VERB)
    cur_preshaper = time2step_preshaper(timepcurr, APPLIED=cur_preshaper_sec, /VERB)

    preshaper_sec = max([cmd_preshaper_sec,cur_preshaper_sec])
    max_settling_time = preshaper_sec

    acc_delay = ceil(max_settling_time/adsec.sampling_time)
    acc_len   = long(acc_period/adsec.sampling_time-acc_delay)

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
;    err = get_preshaper(sc.all, pr_cmd_old)
;    if err ne adsec_error.ok then return, err
;    err = get_preshaper(sc.all, pr_cur_old, /FF)
;    if err ne adsec_error.ok then return, err

    err = set_preshaper(sc.all, pr_cmd)
    if err ne adsec_error.ok then return, err
    err = set_preshaper(sc.all, pr_cur, /FF)
    if err ne adsec_error.ok then return, err

    ;accumulator setting




    ;clearing the feed forward currents and disable the ave ctrl curr integration

    ;disable disturbance ; moved before the update commands
   ; err = set_disturb(/DISABLE)
;    if err ne adsec_error.ok then return, err

;    err=clear_ff(/RESET_FILTER)
;    if err ne adsec_error.ok then begin
;        message, "Error clearing feed-forward currents!!",CONT= (sc.debug eq 0B)
;        return, err
;    endif

;    err = clear_rtr()
;    if err ne adsec_error.ok then return, err

    ;set up the modes/commands/forces thresholds
    err = set_safe_skip(ACT=adsec.act_w_cl)
    if err ne adsec_error.ok then return, err

    ;set up the dummy recostruction/delay/m2c matrices
    b0 = fltarr(rtr.n_slope,adsec.n_actuators)
    b_delay = fltarr(rtr.n_slope,adsec.n_actuators, rtr.n_slope_delay)       ;(1 delay slopes)
    a_delay = fltarr(adsec.n_actuators,adsec.n_actuators,rtr.n_modes_delay) ;(2 delay modes)
    b0[0,0] = identity(adsec.n_actuators)
    old_m2c_mat = *rtr.m2c_matrix
    new_m2c_mat = fltarr(rtr.n_modes2correct, adsec.n_actuators)
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
    master_freq = 200.
    diagn_dec = round(work_freq/master_freq)
    err = set_diagnostic(/MASTER, DECIMATION_MASTER=diagn_dec)
    if err ne adsec_error.ok then return, err

    err = test_skip_frame(OLDPEND=oldpend, OLDSKIP=oldskip)
    if err ne adsec_error.ok then return, err

    ;enable disturbance
    err = set_disturb(time_hist, /ENABLE)
    if err ne adsec_error.ok then return, err

    if work_freq gt 500 then begin 
        err = enable_pending(IS_PENDING=old_pending)
        if err ne adsec_error.ok then return, err
        err = disable_pending()
        if err ne adsec_error.ok then return, err
    endif

    undefine, pbs, wfsc

    slopes = fltarr(rtr.n_slope)
    err = wfs_switch_step(slopes, WAIT=fulltime, WFSC=wfsc, PBS=pbs)
    if err ne adsec_error.ok then return, err

    err = get_fast_diagnostic(diagn_struct)
    if err ne adsec_error.ok then return, err
    base_frame = diagn_struct

    if keyword_set(SWITCHB) then begin
        
        for i=0, nsamples-1 do begin
            slopes = fltarr(rtr.n_slope)
            err = wfs_switch_step(slopes, WAIT=fulltime, WFSC=wfsc, PBS=pbs)
            if err ne adsec_error.ok then return, err
        endfor

    endif else begin

        err = set_diagnostic(OVER=period)
        if err ne adsec_error.ok then return, err

        wait, nsamples*period*1.01

        err = set_diagnostic(OVER=0.0)
        if err ne adsec_error.ok then return, err

    endelse

    err = set_diagnostic(MASTER=0)
    if err ne adsec_error.ok then return, err
    err = set_disturb(/DISABLE)    
    if err ne adsec_error.ok then return, err

    err = test_skip_frame(OLDPEND=oldpend, OLDSKIP=oldskip)
    if err ne adsec_error.ok then return, err

    ;RESTORE OLD PRESHAPER COMMANDS 
    err = set_preshaper(sc.all, pr_cmd_old)
    if err ne adsec_error.ok then return, err
    err = set_preshaper(sc.all, pr_cur_old, /FF)
    if err ne adsec_error.ok then return, err
    if n_elements(old_pending) gt 0  then begin
        if old_pending then begin 
            err = enable_pending()
            if err ne adsec_error.ok then return, err
        endif

    endif

    err = get_status(status_save)
    if err ne adsec_error.ok then return, err
    save, FILE=meas_path('disturbance')+'data_to_reduce.sav', nsamples, work_freq, base_frame, time_hist, status_save
End

Function test_time_hist_get, XADSEC=xadsec

    @adsec_common
    restore, meas_path('disturbance')+'data_to_reduce.sav', /ver
    err = load_diag_data_dsp(nsamples+1, data_reduced, /NOFASTCHECK, /READFC, XADSEC=xadsec)
    if err ne adsec_error.ok then return, err

    err = load_diag_data_switch(nsamples+1, data_reduced_sw, /NOFASTCHECK, /READFC, XADSEC=xadsec)
    if err ne adsec_error.ok then return, err

    save, FILE=meas_path('disturbance')+'disturb_1Khz_6sec.sav', data_reduced, nsamples, work_freq, base_frame, time_hist, data_reduced_sw
    

    return, adsec_error.ok

End
