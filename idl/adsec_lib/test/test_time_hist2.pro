;$Id: test_time_hist2.pro,v 1.9 2009/11/25 08:53:54 marco Exp $
;+
;
;print, test_time_hist2(HIST=adsec_path.data+'layer_phi_000000_time_hist_armando_672a.sav')
;HISTORY
;       written by Marco Xompero(MX)
;       13 Feb 2008 MX, Armando Riccardi (AR)
;       Added the delta error position on the bias_command in order to clean the error position.         
;-
Function test_time_hist2, work_freq, nsamples, SWITCHB=switchb, HISTFILE=histfile , OLD_OVS=old_ovs, SOUNDFILE=soundfile, FACT=fact

    @adsec_common
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
    if max(abs(dpos[adsec.act_w_cl])) gt 1e-6 then message, "TOO LARGE STROKE!"

    err = write_bias_cmd(ss.bias_command+dpos)
    if err ne adsec_error.ok then return, err

    err = set_icc(5.0, 10.0, slope_freq=1000)
    if err ne adsec_error.ok then return, err

    ;period=rtr.oversampling_time
    if n_elements(histfile) eq 0 then begin
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
    endif else begin

		if keyword_set(soundfile) then begin
			time_hist = 2.*readfits(histfile)
		endif else begin
				restore,histfile, /ver
        
				time_hist = pos_read *0.0
				for i=0, rtr.disturb_len-1 do time_hist[i,adsec.act_w_cl] = pos_read[i, adsec.act_w_cl] - mean(double(pos_read[i,adsec.act_w_cl]))
				time_hist = float(time_hist)
		endelse
				nsamples = rtr.disturb_len

    endelse

    if rtr.disturb_len ne max(size(time_hist, /DIM)) then begin
        message, "ERROR: TIMEHIST SIZE DOESN'T MATCH WITH RTR.DISTURB_LEN 4952"
        return, adsec_error.generic_error
    endif
    
    err = set_disturb(/DISABLE)

    ;TIMEHIST POSITION AND FORCE CHECKS
    err = update_status()
    if err ne adsec_error.ok then return, err
;    update_panels, /NO_READ
    if ~keyword_set(soundfile) then begin    
        ;;;; DA CAPIRE PERCHE' SIA COSI' ALTA ;;;;
        r0_hist = lbt.lambda / (0.65*!pi/180./3600.)
        r0_scale = (lbt.r0/r0_hist)^(5./6.)
        time_hist *= r0_scale   
    endif

    if n_elements(fact) eq 0 then fact=1.
    time_hist=float(time_hist)*fact
    hpos = float(time_hist)
    hfor = adsec.ff_matrix ## hpos
    fullpos = rebin(transpose(sys_status.position), rtr.disturb_len, adsec.n_actuators)+ hpos
    fullfor = rebin(transpose(sys_status.current), rtr.disturb_len, adsec.n_actuators) + hfor
    maxp = 1e6* max(hpos[*, adsec.act_w_cl])
    minp = 1e6* min(hpos[*, adsec.act_w_cl])
    maxc = max(hfor[*, adsec.act_w_cl])
    minc = min(hfor[*, adsec.act_w_cl])
    
    maxfp = 1e6* max(fullpos[*, adsec.act_w_cl])
    minfp = 1e6* min(fullpos[*, adsec.act_w_cl])
    maxfc = max(fullfor[*, adsec.act_w_cl])
    minfc = min(fullfor[*, adsec.act_w_cl])
    c_rms = fltarr(rtr.disturb_len)
    
    for ii=0, rtr.disturb_len-1 do c_rms[ii] = stddev(hfor[ii,adsec.act_w_cl])
    
    print, "Min,Max, Force Hist  [N]:"+string(minc, format='(F6.3)') + " " + string(maxc, format='(F5.3)') 
    print, "Min,Max, RMS Force Hist  [N]:"+string(min(c_rms), format='(F6.3)') + " " + string(max(c_rms), format='(F5.3)') 
    print, "Min,Max, Pos  Hist  [um]:"+string(minp, format='(F5.1)') + " " + string(maxp, format='(F5.1)') 
    

    print, "Min,Max Force Total [N]:"+string(minfc, format='(F6.3)') + " " + string(maxfc, format='(F5.3)') 
    print, "Min,Max Pos  Total [um]:"+string(minfp, format='(F5.1)') + " " + string(maxfp, format='(F5.1)') 

    ans = dialog_message("Continue?",/QUEST)
    if ans ne "Yes" then begin
        err = set_icc(/reset)
        return, adsec_error.generic_error
    endif
    
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

    if work_freq gt 900 then begin 
        err = enable_pending(IS_PENDING=old_pending)
        if err ne adsec_error.ok then return, err
        err = disable_pending()
        if err ne adsec_error.ok then return, err
    endif else begin
        err = enable_pending()
        if err ne adsec_error.ok then return, err
    endelse

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
    save, FILE=meas_path('disturbance')+'data_to_reduce.sav', nsamples, work_freq, base_frame, time_hist, status_save, hpos, hfor, status_save2

    err = test_skip_frame(OLDPEND=oldpend, OLDSKIP=oldskip)
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

Function test_time_hist_get, XADSEC=xadsec, OLD_OVS=old_ovs

    @adsec_common
;    nsamples = rtr.disturb_len
;    WORK_FREQ = 1000.
    if n_elements(old_ovs) eq 0 then old_ovs = 0.002;old_ovs=1e-3
    file = dialog_pickfile()
    ;restore, meas_path('disturbance')+'data_to_reduce.sav', /ver
    restore, file, /ver


    err = set_diagnostic(over=0.0)
    if err ne adsec_error.ok then return, err
    err = set_disturb(/DISABLE)    
    if err ne adsec_error.ok then return, err
    err = dspstorage(/DISABLE)
    if err ne adsec_error.ok then return, err
    err = set_icc(/reset)
    if err ne adsec_error.ok then return, err


    n_samples2restore=19000
    ;err = load_diag_data_switch(nsamples+1, data_reduced_sw, /NOFASTCHECK, /READFC, XADSEC=xadsec)
    err = load_diag_data_switch(n_samples2restore+1, data_reduced_sw, /NOFASTCHECK, /READFC, XADSEC=xadsec)
    if err ne adsec_error.ok then return, err

    err = set_diagnostic(over=old_ovs)
    if err ne adsec_error.ok then return, err
    n_samples2restore=19000
    ;err = load_diag_data_dsp(4*nsamples+1, data_reduced, /NOFASTCHECK, /READFC, XADSEC=xadsec)
    err = load_diag_data_dsp(n_samples2restore+1, data_reduced, /NOFASTCHECK, /READFC, XADSEC=xadsec)
    if err ne adsec_error.ok then return, err

    ;err = load_diag_data_switch(nsamples+1, data_reduced_sw, /NOFASTCHECK, /READFC, XADSEC=xadsec)
    ;err = load_diag_data_switch(n_samples2restore+1, data_reduced_sw, /NOFASTCHECK, /READFC, XADSEC=xadsec)
    ;if err ne adsec_error.ok then return, err

;    err = set_diagnostic(over=0.0)
;    if err ne adsec_error.ok then return, err
;    err = set_disturb(/DISABLE)    
;    if err ne adsec_error.ok then return, err
    
    err = set_diagnostic(over=0.0)
    if err ne adsec_error.ok then return, err

    err = dspstorage()
    if err ne adsec_error.ok then return, err

    err = set_diagnostic(over=old_ovs)
    if err ne adsec_error.ok then return, err
 
    file = meas_path('disturbance')+'telescope_CR_90deg_disturb_1000hz_TS5.sav'
    log_print, file

    save, FILE=file, data_reduced, nsamples, work_freq, base_frame, time_hist, data_reduced_sw, hpos, hfor, status_save, status_save2
    

    return, adsec_error.ok

End

