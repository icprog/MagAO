;$Id: test_continuos_chop.pro,v 1.3 2009/08/24 15:24:26 labot Exp $

Function test_continuos_chop, amp, data_reduced, HIST=hist, FORCE_A_NULL=force_a_null
    @adsec_common

    sys_status_ini = sys_status
    file1 = '/home/labot/idl/adsec672a/meas/flat/2009_04_02/flat_data_000_200_70um_chopA+25e-6_10deg.sav'
    file2 = '/home/labot/idl/adsec672a/meas/flat/2009_04_02/flat_data_000_200_70um_chopA-25e-6_10deg.sav'
    file1 = '/usr/local/adopt/calib/left/adsec/CMD/shape/2009_08_12_chop_shape_-25um.sav'
    file2 = '/usr/local/adopt/calib/left/adsec/CMD/shape/2009_08_12_chop_shape_+25um.sav'


;    file1 = '/home/labot/idl/adsec672a/meas/flat/2009_07_30/chop_a.sav'
;    file2 = '/home/labot/idl/adsec672a/meas/flat/2009_07_30/chop_bis.sav'
    ;restore, adsec_path.data+"/calib_divisor.sav"

    restore, /ver, file1
    newpos1 = transpose(flattened_status.position)
    s1 = flattened_status 
;    s1.position *= calib_divisor
;    s1.command *= calib_divisor
    restore, /ver, file2
    newpos2 = transpose(flattened_status.position)
    s2 = flattened_status
;    s2.position *= calib_divisor
;    s2.command *= calib_divisor
    restore, /ver, file2
    tmp_A_dcmd = filt_flat(s1, sys_status_ini) 
    tmp_B_dcmd = filt_flat(s2, sys_status_ini)
    mode_idx = indgen(n_elements(adsec.act_w_cl))

    new_s1 = s1
    new_s1.position = sys_status_ini.position+tmp_A_dcmd
    find_opt_flat, sys_status_ini, new_s1, mode_idx, A_dcmd , THR=thr, MAX_MODE=max_mode, NOT_MATCH_ACT=cl_not_match, FORCE_THR = 0.3
    
    new_s2 = s2
    new_s2.position = sys_status_ini.position+tmp_B_dcmd
    find_opt_flat, sys_status_ini, new_s2, mode_idx, B_dcmd , THR=thr, MAX_MODE=max_mode, NOT_MATCH_ACT=cl_not_match, FORCE_THR = 0.3

    A_dcmd = transpose(A_dcmd)
    if keyword_set(FORCE_A_NULL) then A_dcmd = A_dcmd * 0.0
    B_dcmd = transpose(B_dcmd)
    
    
    timepcmd = 15e-3  ;max 231e-3
    timepcurr = 15e-3 ;max 231e-3
    acc_period = 1e-3 

    ;chopping a 10Hz con settling time 15ms
    ; the samples has to be set to 4950
    ;the preshaper time is set to 15ms, as the oversampling period
    ;we consider only 6 period in the chop "steady state"
    ;the final frequency of chopping is 11.11 Hz
    ;the interferometer has to be set to 10.76 Hz = 5.38 Hz
    ;at least chopping 30 cycles has to be done for one wave sampling
    ;taking 1500 images (about 140 seconds) gives you 50 chopping periods fully sampled
    if rtr.disturb_len ne 4944 then message, "ERROR: rtr.disturb_len must be 4944"
    chop = [rebin(A_dcmd, 6, adsec.n_actuators, /SAMP), rebin(B_dcmd, 6, adsec.n_actuators, /SAMP)]
    dummy = rebin(transpose(chop), 4944/12.*adsec.n_actuators, 12, /SAMP)
    hist = reform(transpose(dummy), 4944, adsec.n_actuators)
    
    ;set up the modes/commands/forces thresholds
    err = set_safe_skip(ACT=adsec.act_w_cl, DELTACMD=70e-6, MAXCMD=135e-6)
    if err ne adsec_error.ok then return, err
   
;    err = set_diagnostic(OVER=0.0)

    ;set up the dummy recostruction/delay/m2c matrices
;    b0 = fltarr(rtr.n_slope,adsec.n_actuators)
;    b_delay = fltarr(rtr.n_slope,adsec.n_actuators, rtr.n_slope_delay) ;(1 delay slopes)
;    a_delay = fltarr(adsec.n_actuators,adsec.n_actuators,rtr.n_modes_delay) ;(2 delay modes)
;    new_m2c_mat = fltarr(rtr.n_modes2correct, adsec.n_actuators)
;    err = set_m2c_matrix(new_m2c_mat)
;    if err ne adsec_error.ok then return, err


    cmd_preshaper = time2step_preshaper(timepcmd, APPLIED=cmd_preshaper_sec, /VERB)
    cur_preshaper = time2step_preshaper(timepcurr, APPLIED=cur_preshaper_sec, /VERB)

    preshaper_sec = max([cmd_preshaper_sec,cur_preshaper_sec])
    max_settling_time = preshaper_sec
    acc_delay = ceil(0.7e-3/adsec.sampling_time)
    acc_len   = long(acc_period/adsec.sampling_time-acc_delay)

    print, "ACC_DELAY: ", strtrim(acc_delay,2)," (",strtrim(acc_delay*adsec.sampling_time,2),"s)"
    print, "ACC_LEN  : ", strtrim(acc_len,2)," (",strtrim(acc_len*adsec.sampling_time,2),"s)"
    print, "MAX FREQ ALLOWED: ", strtrim( 1d/acc_period , 2)

    err = set_acc(acc_len,acc_delay)
    if err ne adsec_error.ok then return, err
    wait, 0.1
    err = start_acc(/WAIT)
    if err ne adsec_error.ok then return, err
    wait, 0.1

    pr_cmd = ((round(adsec.preshaper_len*adsec.sampling_time/timepcmd)  > 1) $
              < dsp_const.fastest_preshaper_step)
    pr_cur = ((round(adsec.preshaper_len*adsec.sampling_time/timepcurr)  > 1) $
              < dsp_const.fastest_preshaper_step)
        
    print, "PRESHAPER CMD: ", timepcmd, pr_cmd
    print, "PRESHAPER CURR" , timepcurr, pr_cur

    err = set_preshaper(sc.all, pr_cmd)
    if err ne adsec_error.ok then return, err
    err = set_preshaper(sc.all, pr_cur, /FF)
    if err ne adsec_error.ok then return, err
        
    g_gain = replicate(0.0, adsec.n_actuators)
        
;    err = set_b0_matrix(b0)
;    if err ne adsec_error.ok then message, "Error encountered: "+strtrim(err,2)
        
;    err = set_b_delay_matrix(b_delay)
;    if err ne adsec_error.ok then message, "Error encountered: "+strtrim(err,2)
        
;    err = set_a_delay_matrix(a_delay)
;    if err ne adsec_error.ok then message, "Error encountered: "+strtrim(err,2)
        
    err = set_g_gain(sc.all, g_gain)
    if err ne adsec_error.ok then message, "Error encountered: "+strtrim(err,2)
        
    err = set_diagnostic(OVER=0.015)
    if err ne adsec_error.ok then return, err
        
    print, "CHOPPING DISTURBANCE APPLICATION *********************************************************************"

    err = set_disturb(/RESET, /COUNTER)
    if err ne adsec_error.ok then return, err

    err = set_disturb(hist)
    if err ne adsec_error.ok then return, err

    ;print, wfs_switch_step(fltarr(rtr.n_slope), /ENA_DIST)

    return, adsec_error.ok

 End


Function continuos_chop_get, filename, dr, drw

;        nsample2restore = 3000
;
;        err = set_diagnostic(OVER=0.0)
;        if err ne adsec_error.ok then return, err
;
;        err = dspstorage(/DISABLE)
;;        if err ne adsec_error.ok then return, err
;
;        err = load_diag_data_switch(n_samples2restore+1, data_reduced_sw, /NOFASTCHECK, /READFC, XADSEC=xadsec)
 ;       if err ne adsec_error.ok then return, err
;;
;        err = set_diagnostic(over=old_ovs)
 ;       if err ne adsec_error.ok then return, err
;
;;        err = load_diag_data_dsp(n_samples2restore+1, data_reduced, /NOFASTCHECK, /READFC, XADSEC=xadsec)
;        if err ne adsec_error.ok then return, err
;        err = set_diagnostic(over=0.0)
 ;       if err ne adsec_error.ok then return, err

 ;       err = dspstorage()
  ;      if err ne adsec_error.ok then return, err

 ;       err = set_diagnostic(over=old_ovs)
 ;       if err ne adsec_error.ok then return, err

 ;   ans = dialog_message("All ok??", /QUESTION)
;    if ans ne "Yes" then begin
;        err = set_offload_cmd(cmd2apply[adsec.act_w_pos], /START, /NOCHECK)
;    endif

        @adsec_common
        print, wfs_switch_step(fltarr(rtr.n_slope), /DIS_DIST)

        err = get_sdram_data(8000, dr, drw, /DSP, /SWI)

        err = get_status(status_save)
        if err ne adsec_error.ok then return, err
   ;     save, file=meas_path('step')+'fast_step_chop_'+filename, status_save, dr, n_samples2restore, drw, old_ovs, mc
        save, file=meas_path('flat')+'fast_step_chop_'+filename, status_save, dr, n_samples2restore, drw, old_ovs, mc

    return, adsec_error.ok
end


