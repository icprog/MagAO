;$Id: test_time_hist2.pro,v 1.9 2009/11/25 08:53:54 marco Exp $
;+
;
;print, test_time_hist2(HIST=adsec_path.data+'layer_phi_000000_time_hist_armando_672a.sav')
;HISTORY
;       written by Marco Xompero(MX)
;       13 Feb 2008 MX, Armando Riccardi (AR)
;       Added the delta error position on the bias_command in order to clean the error position.         
;-
Function test_curr_leak, work_freq, nsamples, SWITCHB=switchb, HISTFILE=histfile , OLD_OVS=old_ovs

    @adsec_common
    histfile = '/usr/local/adopt/conf/adsec/672a/data/layer_phi_000000_time_hist_armando_672a.sav'
    old_ovs = 1./rtr.oversampling_time 
    work_freq = 1000.
;    work_freq = 800.
    period = 1./work_freq
    
    err = set_icc(/RESET)
    if err ne adsec_error.ok then return, err
    wait, 1.0

    err = update_status()
    if err ne adsec_error.ok then return, err
    ss = sys_status

    dpos = (ss.position-ss.full_bias_command)
    if max(abs(dpos[adsec.act_w_cl])) gt 2e-6 then message, "TOO LARGE STROKE!"

    err = write_bias_cmd(ss.bias_command+dpos)
    if err ne adsec_error.ok then return, err

    err = set_icc(5.0, 10.0, slope_freq=1000)
    if err ne adsec_error.ok then return, err

    ;period=rtr.oversampling_time
         nel = ((rtr.disturb_len-3)/2*2)
         template = fltarr(nel)
         template[indgen(nel/2)*2] = -1
         template[indgen(nel/2)*2+1] = 1
         err =  make_modal_disturb(500,500, template=template, trigg_amp=0, ini_0=1, end_0=1, hist=time_hist, /nocheck )

    ;    time_hist = time_hist/3. *2
        nsamples = rtr.disturb_len


    if rtr.disturb_len ne max(size(time_hist, /DIM)) then begin
        message, "ERROR: TIMEHIST SIZE DOESN'T MATCH WITH RTR.DISTURB_LEN"
        return, adsec_error.generic_error
    endif
    
    err = set_disturb(/DISABLE)

    ;TIMEHIST POSITION AND FORCE CHECKS
    err = update_status()
    if err ne adsec_error.ok then return, err
;    update_panels, /NO_READ
    
    ;ans = dialog_message("Continue?",/QUEST)
    ;if ans ne "Yes" then begin
    ;    err = set_icc(/reset)
    ;    return, adsec_error.generic_error
    ;endif
    
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
    err = set_safe_skip(ACT=adsec.act_w_cl, MODLIST=indgen(n_elements(adsec.act_w_cl)))
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
    master_freq = 30.
    diagn_dec = round(work_freq/master_freq)
    err = set_diagnostic(/MASTER, DECIMATION_MASTER=diagn_dec)
    if err ne adsec_error.ok then return, err

    err = test_skip_frame(OLDPEND=oldpend, OLDSKIP=oldskip)
    if err ne adsec_error.ok then return, err

    ;if work_freq gt 900 then begin 
    ;    err = enable_pending(IS_PENDING=old_pending)
    ;    if err ne adsec_error.ok then return, err
    ;    err = disable_pending()
    ;    if err ne adsec_error.ok then return, err
    ;endif else begin
        err = enable_pending()
        if err ne adsec_error.ok then return, err
    ;endelse

    ;ENABLE TIMEOUT FRAMES
;    err = set_diagnostic(OVER=period)
;    if err ne adsec_error.ok then return, err

    ;write disturbance time_hist
;   err = set_disturb(time_hist)
;   if err ne adsec_error.ok then return, err

    ;go to first atm position
;    nstep=20
;    dcomm = time_hist[0,*]/nstep
;    for kk = 0, nstep do begin
;        for2w = (adsec.ff_matrix ## dcomm*kk)[adsec.act_w_curr]
;        err = set_offload_force(for2w, /SW)
;        if err ne adsec_error.ok then return, err
;        cmd2w = (dcomm*kk)(adsec.act_w_pos)
;        err = set_offload_cmd(cmd2w, /SW, /START)
;        if err ne adsec_error.ok then return, err
;        wait, 2*period > 0.01
;        err = update_status()
;        if err ne adsec_error.ok then return, err
;        update_panels, /no_read
;    endfor
    
    err = get_status(status_save)
    if err ne adsec_error.ok then return, err
    wait, 1

    ;enable disturbance
;    err = get_status(status_save)

;    err = set_disturb(time_hist, /ENABLE)
;    if err ne adsec_error.ok then return, err

    err = set_disturb(/RESET, /COUNTER)
    if err ne adsec_error.ok then return, err

    err = set_disturb(time_hist)
    if err ne adsec_error.ok then return, err

    undefine, pbs, wfsc

    ;slopes = fltarr(rtr.n_slope)
    ;err = wfs_switch_step(slopes, WAIT=fulltime, WFSC=wfsc, PBS=pbs)
    ;if err ne adsec_error.ok then return, err

    ;err = get_fast_diagnostic(diagn_struct)
    ;if err ne adsec_error.ok then return, err
    ;base_frame = diagn_struct

    if keyword_set(SWITCHB) then begin
        
        for i=0, nsamples-1 do begin
            slopes = fltarr(rtr.n_slope)
            err = wfs_switch_step(slopes, WAIT=fulltime, WFSC=wfsc, PBS=pbs)
            if err ne adsec_error.ok then return, err
            err = update_status()
            if err ne adsec_error.ok then return, err
            ;update_panels, /no_read
        endfor

    endif else begin


        err = set_diagnostic(over=0.0)
        if err ne adsec_error.ok then return, err
        print, wfs_switch_step(fltarr(rtr.n_slope),  PBS='90c0'xl)
        err = set_diagnostic(OVER=period)
        if err ne adsec_error.ok then return, err

        ts = systime(/sec)
        while (systime(/sec) - ts) lt  nsamples*period*1.01 do begin
            err = update_status()
            if err ne adsec_error.ok then return, err
            ;update_panels, /no_read

            ;err = set_diagnostic(OVER=0.0)
            ;if err ne adsec_error.ok then return, err
        endwhile
    endelse

    ;err = set_diagnostic(MASTER=0, DECIMATION_MASTER=0)
    ;if err ne adsec_error.ok then return, err
    ;err = set_disturb(/DISABLE)    
    ;if err ne adsec_error.ok then return, err


    err = get_status(status_save2)
    if err ne adsec_error.ok then return, err
;    save, FILE=meas_path('disturbance')+'data_to_reduce.sav', nsamples, work_freq, base_frame, time_hist, status_save, hpos, hfor, status_save2

    err = test_skip_frame(OLDPEND=oldpend, OLDSKIP=oldskip)
    if err ne adsec_error.ok then return, err

     err = set_if_trigger(1,1)
    if err ne adsec_error.ok then return, err
    ;RESTORE OLD PRESHAPER COMMANDS 
;    err = set_preshaper(sc.all, pr_cmd_old)
;    if err ne adsec_error.ok then return, err
;    err = set_preshaper(sc.all, pr_cur_old, /FF)
;    if err ne adsec_error.ok then return, err
;    if n_elements(old_pending) gt 0  then begin
;        if old_pending then begin 
;            err = enable_pending()
;            if err ne adsec_error.ok then return, err
;        endif
;    endif


End

Function test_curr_leak_stop, XADSEC=xadsec, OLD_OVS=old_ovs, FULL=full

    @adsec_common
;    nsamples = rtr.disturb_len
;    WORK_FREQ = 1000.
    ;if n_elements(old_ovs) eq 0 then old_ovs = 0.03;old_ovs=1e-3
;    file = dialog_pickfile()
    old_ovs = 0.002
    ;restore, meas_path('disturbance')+'data_to_reduce.sav', /ver
    ;restore, file, /ver


    err = set_diagnostic(over=0.0)
    if err ne adsec_error.ok then return, err
    err = set_disturb(/DISABLE)    
    if err ne adsec_error.ok then return, err
    err = dspstorage(/DISABLE)
    if err ne adsec_error.ok then return, err
    err = wfs_switch_step(fltarr(1600), /DIS)
    if err ne adsec_error.ok then return, err
    err = set_icc(/reset)
    if err ne adsec_error.ok then return, err


    n_samples2restore=19000
    ;err = load_diag_data_switch(nsamples+1, data_reduced_sw, /NOFASTCHECK, /READFC, XADSEC=xadsec)
;    err = load_diag_data_switch(n_samples2restore+1, data_reduced_sw, /NOFASTCHECK, /READFC, XADSEC=xadsec)
;    if err ne adsec_error.ok then return, err

    err = set_diagnostic(over=old_ovs)
    if err ne adsec_error.ok then return, err
    n_samples2restore=19000
    ;err = load_diag_data_dsp(4*nsamples+1, data_reduced, /NOFASTCHECK, /READFC, XADSEC=xadsec)
;    err = load_diag_data_dsp(n_samples2restore+1, data_reduced, /NOFASTCHECK, /READFC, XADSEC=xadsec)
;    if err ne adsec_error.ok then return, err

    ;err = load_diag_data_switch(nsamples+1, data_reduced_sw, /NOFASTCHECK, /READFC, XADSEC=xadsec)
    ;err = load_diag_data_switch(n_samples2restore+1, data_reduced_sw, /NOFASTCHECK, /READFC, XADSEC=xadsec)
    ;if err ne adsec_error.ok then return, err

;    err = set_diagnostic(over=0.0)
;    if err ne adsec_error.ok then return, err
    
    err = set_diagnostic(over=0.0)
    if err ne adsec_error.ok then return, err

    err = dspstorage()
    if err ne adsec_error.ok then return, err

     err = set_disturb(fltarr(rtr.disturb_len, adsec.n_actuators)    )
    if err ne adsec_error.ok then return, err
     err = set_disturb(/ENA)
    if err ne adsec_error.ok then return, err
     err = wfs_switch_step(fltarr(1600), /ENA)
    if err ne adsec_error.ok then return, err

    err = set_diagnostic(over=old_ovs)
    if err ne adsec_error.ok then return, err

    if keyword_set(FULL) then begin
    print, set_diagnostic(over=0) & err= clear_ff() & print, wfs_switch_step(/dis) & print, set_disturb(/dis) & print, set_diagnostic(over=0.002)
    endif

 ;   save, FILE=meas_path('disturbance')+'disturb_1000hz_TS3.sav', data_reduced, nsamples, work_freq, base_frame, time_hist, data_reduced_sw, hpos, hfor, status_save, status_save2
    

    return, adsec_error.ok

End

Pro test_curr_leak_now

    @adsec_common
    for i=1, 1000 do begin
        err = getdiagnvalue('fastdiagn.L', 'ChFFPureCurrent', -1, -1, ffp)
        if max(abs(ffp.last)) gt 0.5 then begin
            err = set_diagnostic(over=0)
            wait, 0.2
            err = read_seq_ch(sc.all, dsp_map.ff_pure_current, 1l, bb)
            mbb = transpose(adsec.ff_f_svec) ## bb & mbb2 = fltarr(adsec.n_actuators) & mbb2[500]=mbb[500] & cc = adsec.ff_f_svec ## mbb2
            dd = bb-cc
            window, /free
            display, dd, /sh, title='FF PureCurrent DRIFT'
            err = set_diagnostic(over=0.0)
;            err = test_curr_leak_stop()
            break
        endif else begin
            wait, 1
        endelse
    endfor
    return

End


