Function restore2onek

    @adsec_common
;----------------------------------------------------------------------------    
    ;PRESHAPERS SETTING

    old_ovs=rtr.oversampling_time
    err = set_diagnostic(over=0, TIM=4000, MASTER=0)
    if err ne adsec_error.ok then return, err

    timepcmd = 0.1e-3
    timepcurr = 0.70e-3 ;len 2, delay 40, max 1400
    time_ps = max([timepcmd, timepcurr])*1.1 > 0.01

;    pr_cmd = ((round(adsec.preshaper_len*adsec.sampling_time/timepcmd)  > 1) < dsp_const.fastest_preshaper_step)
;    pr_cur = ((round(adsec.preshaper_len*adsec.sampling_time/timepcurr)  > 1) < dsp_const.fastest_preshaper_step)

;    err = set_preshaper(sc.all, pr_cmd)
;    if err ne adsec_error.ok then return, err
;    err = set_preshaper(sc.all, pr_cur, /FF)
;    if err ne adsec_error.ok then return, err

;----------------------------------------------------------------------------    



;----------------------------------------------------------------------------    
    ;ACCUMULATORS SETTINGS
;    acc_period = fast_period*0.95
    max_settling_time = max([timepcmd,timepcurr])
    acc_period = max_settling_time+0.1e-3 ; 

    acc_delay = ceil(max_settling_time/adsec.sampling_time)
    acc_len   = long(acc_period/adsec.sampling_time-acc_delay)

;    acc_period = 0.8e-3
;    acc_delay = ceil(acc_period/2./adsec.sampling_time)
;    acc_len   = long(acc_period/adsec.sampling_time-acc_delay)


    log_print, "ACC_DELAY after setting: "+ strtrim(acc_delay,2)+" ("+strtrim(acc_delay*adsec.sampling_time,2)+"s)"
    log_print, "ACC_LEN   after setting: "+ strtrim(acc_len,2)+" ("+strtrim(acc_len*adsec.sampling_time,2)+"s)"

    err = set_acc(acc_len,acc_delay)
    if err ne adsec_error.ok then return, err
    wait, 0.1
    err = start_acc(/WAIT)
    if err ne adsec_error.ok then return, err


    err = wfs_switch_step()
    if err ne adsec_error.ok then return, err


    if old_ovs lt 1.5e-3 then old_ovs=2e-3
    err = update_ovs(old_ovs)
    if err ne adsec_error.ok then return, err

    err = set_diagnostic(over=old_ovs, TIM=3000, MASTER=1)
    if err ne adsec_error.ok then return, err


    wait, 2*rtr.oversampling_time > 0.1

    return, adsec_error.ok

end
