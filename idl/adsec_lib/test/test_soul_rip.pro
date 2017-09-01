;table:
;



;settare gli accumulatori per funzionare a 1500/2000 Hz
;settare oversamping a 1500 Hz
;verificare di non perdere frames...


Function test_soul_rip

    @adsec_common
;----------------------------------------------------------------------------    
    ;PRESHAPERS SETTING

    old_ovs=rtr.oversampling_time
    err = set_diagnostic(over=0, TIM=3000)
    if err ne adsec_error.ok then return, err

    timepcmd = 0.1e-3
    timepcurr = 0.40e-3 ;len 2, delay 40, max 1400
;    timepcurr = 0.45e-3 ;len 3, delay 32, max 1600
    time_ps = max([timepcmd, timepcurr])*1.1 > 0.01

    pr_cmd = ((round(adsec.preshaper_len*adsec.sampling_time/timepcmd)  > 1) < dsp_const.fastest_preshaper_step)
    pr_cur = ((round(adsec.preshaper_len*adsec.sampling_time/timepcurr)  > 1) < dsp_const.fastest_preshaper_step)

    err = set_preshaper(sc.all, pr_cmd)
    if err ne adsec_error.ok then return, err
    err = set_preshaper(sc.all, pr_cur, /FF)
    if err ne adsec_error.ok then return, err

;----------------------------------------------------------------------------    



;----------------------------------------------------------------------------    
    ;ACCUMULATORS SETTINGS
;    acc_period = fast_period*0.95

    t_acc_delay = max([timepcmd,timepcurr])
    ;t_set = 0.6e-3
    ;acc_delay = ceil(t_acc_delay/adsec.sampling_time)
    ;acc_len   = long(t_set/adsec.sampling_time-acc_delay)
    t_set = timepcurr
    acc_delay = ceil(t_acc_delay/adsec.sampling_time)-4
    acc_len   = 4L

    log_print, "ACC_DELAY after setting: "+ strtrim(acc_delay,2)+" ("+strtrim(acc_delay*adsec.sampling_time,2),"s)"
    log_print, "ACC_LEN   after setting: "+ strtrim(acc_len,2)+" ("+strtrim(acc_len*adsec.sampling_time,2),"s)"
    log_print, "MAX FREQ ALLOWED: "+ strtrim( 1d/t_set , 2)

    err = set_acc(acc_len,acc_delay)
    if err ne adsec_error.ok then return, err
    wait, 0.1
    err = start_acc(/WAIT)
    if err ne adsec_error.ok then return, err
    err = set_diagnostic(over=old_ovs)
    if err ne adsec_error.ok then return, err

    wait, 2*rtr.oversampling_time > 0.1

    err= read_seq_dsp(sc.all, dsp_map.delay_acc, 1l, bb, /ul)
    err= read_seq_dsp(sc.all, dsp_map.num_samples, 1l, bb2, /ul)
    print, mean(bb), mean(bb2) 
;---------------------------------------------------------------------------
    return, 0

end
