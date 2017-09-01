;$Id: ff_data_acquisition_v2.pro,v 1.3 2009/05/22 16:32:55 labot Exp $
;+
;   NAME:
;    FF_DATA_ACQUISITION_V2
;
;   PURPOSE:
;    The routine performs the data acquisition for the feed forward matrix
;    estimation.
;
;   USAGE:
;    err = ff_data_acquisition, nmeas, amp, samples, data, MODAL=modal, FF=ff, CL_ACT=cl_act $
;    , INTEGRATOR=integ, SMOOTH=do_smooth, MAX_PEAK_FORCE=max_peak_force
;
;   INPUT:
;    nmeas:     number of measurements to do. (positive and negative excitation)
;    amp:       amplitude of excitation(FLOAT).
;    samples:   number of samples to get (UNSIGNED LONG).
;
;   OUTPUT:
;    Error code.
;
;   KEYWORD:
;    MODAL:         use the modal estmation of the feed forward matrix (the default is zonal).
;    FF:            use the feed forward contribution of a precalculated matrix.
;    CL_ACT:        close loop actuators. (defaul adsec.act_w_cl)
;    INTEGRATOR:    perfoms the use of the integrator. (don't use with FF enabled.)
;    SMOOTH:        smoothed visializzation of the data retrieved.
;    MAX_PEAK_FORCE: set a new max peak force value.
;
;   NOTE:
;    Data ordering:
;      reference, +amp(ch/mode_1), -amp(ch/mode_1), +amp(ch/mode_2), ... , -amp(ch/mode_n_meas), reference
;
;   HISTORY
;    Written by M.Xompero on 5 Jun 2007, fixing ff_data_acquisition.pro routine.
;    Osservatorio Asrofisico di Arcetri, ITALY
;    <marco@arcetri.astro.it>
;   20 Sep 2007 MX
;    Fixed display, control current data added.
;-


function ff_data_acquisition_v2, nmeas, amp, samples, data, MODAL=modal, FF=ff, CL_ACT=cl_act $
    , INTEGRATOR=integ, SMOOTH=do_smooth, MAX_PEAK_FORCE=max_peak_force $ 
    , BIAS_CURRENT=bias_current, BIAS_COMMAND=bias_command, TN=tn

    @adsec_common
    if (sc.model eq '672a') or (sc.model eq '672b') then begin
        do_smooth = 0B
    endif

    err = enable_pending(IS_PENDING=old_pending)
    if err ne adsec_error.ok then return, err

    ;add check rtr.n_modes2correct=adsec.n_actuators

    settime = 35e-3
    delaytime = settime+1e-3
    fulltime = (delaytime+adsec.sampling_time*samples) 
    string_displ = " mode# "

    ;Calculation of number of steps in order to fill the dsp sdram
    ;/2. for the up and down step
    ;-2 for the first and last steps
    ;mem = dsp_datasheet.sdram_mem_size
    ;max_cycles = float(dsp_datasheet.sdram_mem_size) / rtr.diag_len / adsec.n_dsp_per_board / n_elements(adsec.act_w_cl) /2. -2
    ;if nmeas gt max_cycles then begin
    ;    message, 'Max number of cycles exceeded. Returning.', CONT=(sc.debug eq 0)
    ;    return, adsec_error.generic_error
    ;endif

    ;checks and setting on default parameters
    if n_elements(max_peak_force) eq 0 then max_peak_force=adsec.weight_curr * adsec.weight_times4peak else max_peak_force = max_peak_force < (adsec.weight_curr * adsec.weight_times4peak)

    if n_elements(cl_act) eq 0 then cl_act=adsec.act_w_cl
    if keyword_set(tn) then path = meas_path('ff_matrix', DAT=tn)

    n_cl_act = n_elements(cl_act)
    if nmeas lt n_cl_act then begin
        message, "WARNING: you set less measures then number of actuators in closed loop", /INFO
    endif

    data_proto={dcommand: fltarr(adsec.n_actuators),         $
                position: fltarr(adsec.n_actuators),        $
                ctrl_current: fltarr(adsec.n_actuators),    $
                rms_pos: fltarr(adsec.n_actuators),         $
                rms_curr: fltarr(adsec.n_actuators),        $
                current: fltarr(adsec.n_actuators),        $
                framec: 0l, $
                ff_current: fltarr(adsec.n_actuators)}

    data=replicate(data_proto, 2*nmeas+3)

    if sc.name_comm eq "Dummy" then t_wait = 0.01 else $
        t_wait = ((time_preshaper()*adsec.time_secure_factor) > 0.01) < 1.

    ;clearing the feed forward currents and disable the ave ctrl curr integration
    err=clear_ff(/RESET_FILTER)
    if err ne adsec_error.ok then begin
        message, "Error clearing feed-forward currents!!",CONT= (sc.debug eq 0B)
        return, err
    endif

    err = clear_rtr()
    if err ne adsec_error.ok then return, err

    ;set up the modes/commands/forces thresholds
    err = set_safe_skip(ACTLIST=adsec.act_w_cl, MODLIST=adsec.act_w_cl)
    if err ne adsec_error.ok then return, err

    ;set up the dummy recostruction/delay/m2c matrices
    b0 = fltarr(rtr.n_slope,adsec.n_actuators)
    b_delay = fltarr(rtr.n_slope,adsec.n_actuators, rtr.n_slope_delay)       ;(1 delay slopes)
    a_delay = fltarr(adsec.n_actuators,adsec.n_actuators,rtr.n_modes_delay) ;(2 delay modes)
    b0[0,0] = identity(adsec.n_actuators)
    ;a_delay[*,*,0] = identity(rtr.n_modes2correct)
    old_m2c_mat = *rtr.m2c_matrix
    if ~keyword_set(MODAL) then begin

        new_m2c_mat = fltarr(rtr.n_modes2correct, adsec.n_actuators)
        vect = fltarr(rtr.n_modes2correct)
        vect[cl_act] = 1.
        new_m2c_mat[0,0] = diagonal_matrix(vect) 
        string_displ = " act# "

    endif else begin
        
        new_m2c_mat = fltarr(rtr.n_modes2correct, adsec.n_actuators)
        new_m2c_mat[cl_act,*] = adsec.ff_p_svec[0:n_cl_act-1, *]
      
    endelse

    err = set_m2c_matrix(new_m2c_mat)
    if err ne adsec_error.ok then return, err


    if ~keyword_set(FF) then begin
        old_ff_mat = adsec.ff_matrix
        if n_elements(old_ff_mat) eq 1 then return, old_ff_mat
        err = send_ff_matrix(fltarr(adsec.n_actuators, adsec.n_actuators))
        if err ne adsec_error.ok then return, err
    endif


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

    data[0].ctrl_current=sys_status.ctrl_current
    data[0].dcommand=sys_status.command
    bias_current=sys_status.bias_current
    bias_command = sys_status.command


    delaytime = settime
    fulltime = (delaytime+adsec.sampling_time*samples) 
    acc_delay = ceil(delaytime/adsec.sampling_time)
    acc_len   = long(samples)
    ;acc_len = 20000L
    ;fulltime = 2

    ; reads the current values for the
    ; preshaper and sets the new preshaper;   DA SISTEMARE!!!!!
    err = get_preshaper(sc.all, old_preshaper_dec)
    if err ne adsec_error.ok then return, err
    err = get_preshaper(sc.all, old_preshaper_ff, /FF)
    if err ne adsec_error.ok then return, err

    preshaper_dec = time2step_preshaper(settime,/VERB)
    preshaper_ff = time2step_preshaper(settime, /VERB)

    err = set_preshaper(sc.all_actuators, preshaper_dec)
    if err ne adsec_error.ok then return, err
    err = set_preshaper(sc.all_actuators, preshaper_ff, /FF)
    if err ne adsec_error.ok then return, err

    ;set the accumulators
    err = set_acc(acc_len,acc_delay, old_sam, old_del)
    if err ne adsec_error.ok then return, err
    err = start_acc(/WAIT)
    if err ne adsec_error.ok then return, err

    ;disable disturbance
    err = set_disturb(/DISABLE)
    if err ne adsec_error.ok then return, err

    err = clear_skip_counter() 
    if err ne adsec_error.ok then return, err

    ;enable master diagnostic
    err = set_diagnostic(/MASTER)
    if err ne adsec_error.ok then return, err

    err = test_skip_frame(OLDPEND=oldpend, OLDSKIP=oldskip)
    if err ne adsec_error.ok then return, err

    slopes = fltarr(rtr.n_slope)
    err = wfs_switch_step(slopes, WAIT=fulltime)
    if err ne adsec_error.ok then return, err

     err = get_fast_diagnostic(diagn_struct)
    if err ne adsec_error.ok then return, err

    idx = 0
    data[idx].dcommand=diagn_struct.ffcommand 
    data[idx].position=diagn_struct.distaverage
    data[idx].framec = diagn_struct.frame_number
    data[idx].rms_pos = diagn_struct.distrms
    data[idx].rms_curr= diagn_struct.currrms
    data[idx].current = diagn_struct.curraverage
    data[idx].ff_current = diagn_struct.ffpurecurrent
    data[idx].ctrl_current = data[idx].current - data[idx].ff_current - bias_current - diagn_struct.intcontrolcurrent
    prev_dcomm = diagn_struct.ffcommand 

    if keyword_set(tn) then begin
        save, file=path+'bias_current.sav', bias_current
        save, file=path+'data_proto.sav', data_proto
        openw, unit, path+'rawdata.bin', /GET_LUN
        disk_data = assoc(unit, data_proto)
        disk_data[idx] = data[idx]
    endif

    sign = [1, -1]
    time0 = systime(/sec)

    new_slopes = fltarr(rtr.n_slope)
    for i=0L, nmeas-1 do begin

        i_cl_act = i mod n_cl_act
        i_act = cl_act[i_cl_act]
        
        err = test_skip_frame(OLDPEND=oldpend, OLDSKIP=oldskip)
        if err ne adsec_error.ok then break
 
        ;positive and negative excitation
        for j=0,1 do begin

            dummy_command=fltarr(1,adsec.n_actuators)

            ;excitation
            if keyword_set(MODAL) then begin
                ; use only the first n_cl_act mode
                peak_force = max(abs(adsec.ff_f_svec[i_cl_act,*]*adsec.ff_sval[i_cl_act]*amp))
                new_amp = (amp*float(max_peak_force)/peak_force) < amp
            endif else new_amp=amp
            dummy_command[0,i_act] = dummy_command[0,i_act] + sign[j]*new_amp 
     
            new_slopes = fltarr(1,rtr.n_slope)
            new_slopes[0,0] = dummy_command
            thh = systime(/sec)
            err = wfs_switch_step(new_slopes, /NOVERB, WFSC=wfsc, WAIT=fulltime)
            print, systime(/sec)-thh
            if err ne adsec_error.ok then return, err
            
            err = get_fast_diagnostic(diagn_struct)
            
            if err ne adsec_error.ok then return, err
            
            idx = 2*i+j+1
            data[idx].dcommand=diagn_struct.ffcommand + prev_dcomm
            data[idx].position=diagn_struct.distaverage
            data[idx].rms_pos = diagn_struct.distrms
            data[idx].rms_curr= diagn_struct.currrms
            data[idx].current = diagn_struct.curraverage
            data[idx].ff_current = diagn_struct.ffpurecurrent
            data[idx].framec = diagn_struct.frame_number
            data[idx].ctrl_current = data[idx].current - data[idx].ff_current - bias_current - diagn_struct.intcontrolcurrent
            prev_dcomm = new_m2c_mat ## dummy_command 
            if keyword_set(tn) then disk_data[idx] = data[idx]

            ;if (j eq 0) and (i gt 0) then display, ((data[idx-2].position-data[idx-1].position)[cl_act])/2., cl_act        $
                                ;                              ,
;                              SMOOTH=do_smooth, /SHOWBAR , /NO_NUM,
;                              TITLE=string_displ+strtrim(i_act,2)
            
            if (j eq 0) and (i gt 0) then display, ((data[idx].position-data[idx-1].position)[cl_act])/2., cl_act        $
                                          , SMOOTH=do_smooth, /SHOWBAR , /NO_NUM, TITLE=string_displ+strtrim(i_act,2)

        endfor

        max_rmsc = max(data[idx].rms_curr[cl_act], idx_temp)
        max_rmsp = max(data[idx].rms_pos[cl_act], idx_temp2)
        log_print, 'Iteration       :'+strtrim(i,2)+ "  amp:"+strtrim(sign[1]*new_amp)+ string_displ+ strtrim(i_act,2)
        log_print, 'Max CURRENT  RMS: '+ strtrim(max_rmsc,2), " @ act#"+ strtrim(cl_act[idx_temp])
        log_print, 'Max POSITION RMS: '+ strtrim(max_rmsp,2), " @ act#"+ strtrim(cl_act[idx_temp2])

 
    endfor

    time1 = systime(/sec)
   log_print, "Time: "+string(time1-time0)

    slopes = fltarr(rtr.n_slope)
    err = wfs_switch_step(slopes, WAIT=fulltime)
    if err ne adsec_error.ok then return, err

    err = get_fast_diagnostic(diagn_struct)
    if err ne adsec_error.ok then return, err
    
    idx = 2*nmeas+1
    data[idx].dcommand=diagn_struct.ffcommand + prev_dcomm
    data[idx].position=diagn_struct.distaverage
    data[idx].framec=diagn_struct.frame_number
    data[idx].rms_pos = diagn_struct.distrms
    data[idx].rms_curr= diagn_struct.currrms
    data[idx].current = diagn_struct.curraverage
    data[idx].ff_current = diagn_struct.ffpurecurrent
    data[idx].ctrl_current = data[idx].current - data[idx].ff_current - bias_current - diagn_struct.intcontrolcurrent
    prev_dcomm = 0;diagn_struct.ffcommand    
    if keyword_set(tn) then disk_data[idx] = data[idx]

    err = wfs_switch_step(slopes, WAIT=fulltime)
    if err ne adsec_error.ok then return, err
    
    idx = 2*nmeas+2
    data[idx].dcommand=diagn_struct.ffcommand + prev_dcomm
    data[idx].position=diagn_struct.distaverage
    data[idx].framec=diagn_struct.frame_number
    data[idx].rms_pos = diagn_struct.distrms
    data[idx].rms_curr= diagn_struct.currrms
    data[idx].current = diagn_struct.curraverage
    data[idx].ff_current = diagn_struct.ffpurecurrent
    data[idx].ctrl_current = data[idx].current - data[idx].ff_current - bias_current - diagn_struct.intcontrolcurrent
    if keyword_set(tn) then disk_data[idx] = data[idx]

    ;data.dcommand += shift(data.dcommand,0,-1)
    ;shift and cut in order to match the
    ;accumulators result with the diagnostic data

    err = set_diagnostic(MASTER=0.)

    err = update_status(samples)
    if err ne adsec_error.ok then begin
        log_message, "Error updating the status.",CONT= (sc.debug eq 0B)
        return, err
    endif

    ;restoring initial conditions and clearing the ff matrix
    ;TODO!!!!!

    ;err=write_bias_curr(sc.all_actuators,bias_current, /CHECK)
    ;if err ne adsec_error.ok then begin
    ;    message, "Error restoring the old bias current.",CONT= (sc.debug eq 0B)
    ;    return, err
    ;endif

    err=clear_ff()
    if err ne adsec_error.ok then begin
        log_message, "Error clearing the feed-forward contribution.",CONT= (sc.debug eq 0B)
        return, err
    endif
    

    err = set_preshaper(sc.all_actuators, old_preshaper_dec)
    if err ne adsec_error.ok then return, err
    err = set_preshaper(sc.all_actuators, old_preshaper_ff, /FF)
    if err ne adsec_error.ok then return, err

    err = set_acc(old_sam, old_del)
    if err ne adsec_error.ok then return, err

    err = update_status(samples)
    if err ne adsec_error.ok then begin
        log_message, "Error updating the status.",CONT= (sc.debug eq 0B)
        return, err
    endif
 
 
    err = set_g_gain(sc.all, /reset)
    if err ne adsec_error.ok then return, err

    err = set_m2c_matrix(old_m2c_mat)
    if err ne adsec_error.ok then return, err
    
    if ~keyword_set(FF) then begin
        err = send_ff_matrix(old_ff_mat)
        if err ne adsec_error.ok then return, err
    endif

    
    if ~old_pending then begin
        err = disable_pending()
        if err ne adsec_error.ok then return, err
    endif

    if keyword_set(tn) then free_lun, unit

    ;position
    tmp = data.position
    tmp = shift(temporary(tmp), 0,-1)
    data.position = temporary(tmp)

    ;current
    tmp = data.current
    tmp = shift(temporary(tmp), 0,-1)
    data.current = temporary(tmp)

    ;rms_pos
    tmp = data.rms_pos
    tmp = shift(temporary(tmp), 0,-1)
    data.rms_pos = temporary(tmp)

    ;rms_curr
    tmp = data.rms_curr
    tmp = shift(temporary(tmp), 0,-1)
    data.rms_curr = temporary(tmp)

    ;data cut
    data = temporary(data[0:2*nmeas+1])

   return,adsec_error.ok
end
