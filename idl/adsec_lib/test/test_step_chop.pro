Function test_step_chop, NOCMD2BIAS=nocmd2bias, response

    @adsec_common
    ;I guadagni e i preshaper deve GIA' ESSERE SETTATI
    timepcmd = 35e-3
    timepcurr = 35e-3
    acc_period = 36e-3 ;period * 0.8 - > 

    x = adsec.act_coordinates[0,*]/adsec_shell.out_radius
    y = adsec.act_coordinates[1,*]/adsec_shell.out_radius
    tilt = zern(3, x, y)        ; / sqrt (n_elements(adsec.act_w_cl)
    coeffs = transpose(adsec.ff_p_svec) ## tilt
    coeffs[0, 5:*] = 0 
    tilt_mir = adsec.ff_p_svec ## coeffs
    
    amp = 1e-6 ;;;;; RMS TILT : PEAK 2*  ;PTV 4*
    cmd2apply = float(amp * tilt_mir)
    cmd2apply_neg = float(-1 * amp * tilt_mir)

    ;disable disturbance
    err = set_disturb(/DISABLE)
    if err ne adsec_error.ok then return, err
    
    err = set_diagnostic(RESET_COUNTER=-1)
    if err ne adsec_error.ok then return, err

    err=clear_ff(/RESET_FILTER)
    if err ne adsec_error.ok then begin
        message, "Error clearing feed-forward currents!!",CONT= (sc.debug eq 0B)
        return, err
    endif

    err = clear_rtr()
    if err ne adsec_error.ok then return, err
    
    ;set up the modes/commands/forces thresholds
    err = set_safe_skip(ACT=adsec.act_w_cl, DELTACMD=50e-6, MAXCMD=125e-6)   
    err = set_safe_skip(/reset)
    if err ne adsec_error.ok then return, err

    ;set up the dummy recostruction/delay/m2c matrices
    b0 = fltarr(rtr.n_slope,adsec.n_actuators)
    b_delay = fltarr(rtr.n_slope,adsec.n_actuators, rtr.n_slope_delay) ;(1 delay slopes)
    a_delay = fltarr(adsec.n_actuators,adsec.n_actuators,rtr.n_modes_delay) ;(2 delay modes)
    b0[0,0] = identity(adsec.n_actuators)
    old_m2c_mat = *rtr.m2c_matrix
    new_m2c_mat = fltarr(rtr.n_modes2correct, adsec.n_actuators)
    err = set_m2c_matrix(new_m2c_mat)
    if err ne adsec_error.ok then return, err


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
    wait, 0.1

    ;pr_cmd = ((round(adsec.preshaper_len*adsec.sampling_time/timepcmd)  > 1) $
    ;       < dsp_const.fastest_preshaper_step)
    ;pr_cur = ((round(adsec.preshaper_len*adsec.sampling_time/timepcurr)  > 1) $
    ;       < dsp_const.fastest_preshaper_step)

    err = set_preshaper(sc.all, cmd_preshaper)
    if err ne adsec_error.ok then return, err
    err = set_preshaper(sc.all, cur_preshaper, /FF)
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
    
    err = set_diagnostic(MASTER=0)
    if err ne adsec_error.ok then return, err

    wait, 0.1

    ;buffers settings
    offset_trig=1000
    chop_freq = 10. ;[Hz]
    if time_preshaper() gt 1./chop_freq then begin
        print, "ERROR on PRESHAPER SETTINGS. Returning"
        return, adsec_error.generic_error
    endif

    settling = round(time_preshaper()/adsec.sampling_time)
    plateau = round(1./chop_freq/adsec.sampling_time)-settling
    nsamples = uint(offset_trig + 4*(settling+plateau)  < 65535L)
    print, nsamples*adsec.sampling_time
    err=update_status(1024)
    sys_status0=sys_status
    err = get_commands(sc.all_actuators, command)
    if err ne adsec_error.ok then return, err
    
    addr2save=[dsp_map.distance,dsp_map.float_dac_value,dsp_map.preshaped_cmd]
    n_addr2save = n_elements(addr2save)
    nbuff= indgen(n_addr2save)+1
;    err = set_IO_buffer(sc.all_dsp, nbuff, addr2save, nsamples, $
;                    rtr_map.update_ff, 1L, /TRIG,  RECORD=adsec.n_act_per_dsp, /START)
    err = set_IO_buffer(sc.all_dsp, nbuff, addr2save, nsamples, $
                    RECORD=adsec.n_act_per_dsp, /START)

    wait, 0.1

    undefine, pbs, wfsc
    ;pbs=2UL^7 ;+ 2UL^6    ;ONLY FASTLINK ENABLE
    pbs=0UL
    slopes = fltarr(rtr.n_slope)
    err = wfs_switch_step(slopes, WAIT=acc_period, WFSC=wfsc, PBS=pbs)
    if err ne adsec_error.ok then return, err

    err = test_skip_frame(/dsp)
    if err ne adsec_error.ok then return, err

    tt=time_preshaper()

    for j=0, 3 do begin

        print, "TILT APPLICATION #"+strtrim(j,2)
        bank = (j+1) mod 2
        print, "BANK #", bank
        err = set_offload_cmd((-1)^j * cmd2apply[adsec.act_w_pos], /START, /NOCHECK, BANK=bank)
        if err ne adsec_error.ok then return, err

        err = wfs_switch_step(slopes, WFSC=wfsc, PBS=pbs)
        if err ne adsec_error.ok then return, err

        err = test_skip_frame()
        if err ne adsec_error.ok then return,err
        ;wait, (1./chop_freq)
        wait, 0.1

    endfor

    print, "RETURNING TO INITIAL POSITION"
    bank = (j+1) mod 2
    print, "BANK #", bank
    err = set_offload_cmd(0. * cmd2apply[adsec.act_w_pos], /START, /NOCHECK, BANK=bank)
    if err ne adsec_error.ok then return, err
    err = wfs_switch_step(slopes,  WFSC=wfsc, PBS=pbs)
    if err ne adsec_error.ok then return, err
    wait, 0.5;(1./chop_freq)

;    err = set_diagnostic(MASTER=1)

;    for i=0, 100 do begin
;        err = wfs_switch_step(slopes, WFSC=wfsc)
;        if err ne adsec_error.ok then return, err
;        wait, 0.1
;    endfor

;    err = set_diagnostic(MASTER=0)
    if err ne adsec_error.ok then return, err

    err=read_IO_buffer_ch(sc.all_dsp, nbuff, nsamples, response)
    if err ne adsec_error.ok then return, err

    err = test_skip_frame()
    if err ne adsec_error.ok then return, err
    print, "PBS", pbs
    return, adsec_error.ok
    


end
