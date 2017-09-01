Function prepare4twok

    @adsec_common
;----------------------------------------------------------------------------    
    ;PRESHAPERS SETTING

    old_ovs=rtr.oversampling_time
    err = set_diagnostic(over=0, TIM=3000, MASTER=0)
    if err ne adsec_error.ok then return, err

    timepcmd = 0.1e-3
    timepcurr = 0.40e-3 ;len 2, delay 40, max 1400

;----------------------------------------------------------------------------    

    t_acc_delay = max([timepcmd,timepcurr])
    t_set = timepcurr
    acc_delay = ceil(t_acc_delay/adsec.sampling_time)-4
    acc_len   = 4L
    ;acc_period = 0.4e-3
    ;acc_delay = ceil(acc_period/2./adsec.sampling_time)
    ;acc_len   = long(acc_period/adsec.sampling_time-acc_delay)


    log_print, "ACC_DELAY after setting: "+ strtrim(acc_delay,2)+" ("+strtrim(acc_delay*adsec.sampling_time,2)+"s)"
    log_print, "ACC_LEN   after setting: "+ strtrim(acc_len,2)+" ("+strtrim(acc_len*adsec.sampling_time,2)+"s)"
    log_print, "MAX FREQ ALLOWED: "+ strtrim( 1d/t_set , 2)

    err = set_acc(acc_len,acc_delay)
    if err ne adsec_error.ok then return, err
    wait, 0.1
    err = start_acc(/WAIT)
    if err ne adsec_error.ok then return, err

    err = wfs_switch_step()
    if err ne adsec_error.ok then return, err

    err = set_diagnostic(over=old_ovs, MASTER=1)
    if err ne adsec_error.ok then return, err

    wait, 2*rtr.oversampling_time > 0.1

;---------------------------------------------------------------------------
    return, adsec_error.ok

end
