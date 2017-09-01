
function i4d_chop,nframes, OFFSET=offset, TRACKNUM=tracknum $
                         , FFPURE=ffpure, SAVEDIAG=savediag, HIST=hist, NOINTERF= nointerf
    
    @adsec_common
    intflag = ~keyword_set(nointerf)
    default_file=adsec_path.conf+'scripts/4d_chop_default.txt'
    def=read_ascii_structure(default_file)
    file4d='$ADOPT_MEAS/adsec_calib/4d/'+def.folder4d+def.file4d
    if file_test(file4d) ne 0 then begin
        log_print, 'A TRACKNUM file was found! Delete it or wait last acquisition to be completed'
        return, -1
    endif

    if n_elements(nframes) ne 0 then def.nframes =nframes
    if n_elements(offset) eq 0 then offset=def.offset
    nstep=n_elements(offset)
 ;   if nstep ne 0 then def.offset=offset
 ;   nstep=n_elements(def.offset)
   ; if n_elements(decimation) ne 0 then def.dec_factor=decimation

    log_print,'Current sampling configuration:'
    log_print, 'Offset=', offset, 'Decimation factor=', def.dec_factor

    file_chop_a=def.chop_a
    file_chop_b=def.chop_b
    err = update_status()
    if err ne adsec_error.ok then return, err

    if file_chop_a eq "" and file_chop_b eq "" then begin

        amp = 50e-6
        make_chopping_tilt, adsec.act_w_cl, tilt, force, mc
        pos_a = sys_status.position - mean(sys_status.position[adsec.act_w_cl]);+amp*tilt
        pos_b = sys_status.position - mean(sys_status.position[adsec.act_w_cl])+amp*tilt

    endif else begin

        if file_chop_a ne "" and file_chop_b eq "" then begin
            pos_a = sys_status.position - mean(sys_status.position[adsec.act_w_cl])
            restore, adsec_path.meas+file_chop_a, /ver
            pos_b=flattened_status.position - mean(flattened_status.position[adsec.act_w_cl])
            undefine, data, rec_mat
        endif else begin
            if file_chop_a ne "" and file_chop_b ne "" then begin
                restore, adsec_path.meas+file_chop_a, /ver
                pos_a=flattened_status.position - mean(flattened_status.position[adsec.act_w_cl])
                restore, adsec_path.meas+file_chop_b, /ver
                pos_b=flattened_status.position - mean(flattened_status.position[adsec.act_w_cl])
                undefine, data, rec_mat
            endif else begin
                log_print, "No valid configuration file for chopping. Returning."
            endelse
        endelse

    endelse

    log_print, "set preshaper to correct value: "

    old_ovs = rtr.oversampling_time
    ;timepcmd = 15e-3  ;max 231e-3
    timepcmd = 15e-3 / floor(15e-3/def.ovs)
    timepcurr = timepcmd
    acc_period = 0.8e-3

    err = set_diagnostic(OVER=0.0)
    if err ne adsec_error.ok then return, err

    cmd_preshaper = time2step_preshaper(timepcmd, APPLIED=cmd_preshaper_sec, /VERB)
    cur_preshaper = time2step_preshaper(timepcurr, APPLIED=cur_preshaper_sec, /VERB)

    preshaper_sec = max([cmd_preshaper_sec,cur_preshaper_sec])
    max_settling_time = preshaper_sec

    ;    acc_delay = ceil(max_settling_time/adsec.sampling_time)
    ;    acc_len   = long(acc_period/adsec.sampling_time-acc_delay)
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
    err = set_preshaper(sc.all, pr_cmd)
    if err ne adsec_error.ok then return, err
    err = set_preshaper(sc.all, pr_cur, /FF)
    if err ne adsec_error.ok then return, err
    err = set_diagnostic(over=old_ovs)



        err = test_skip_frame(safe=safecnt)
        if err ne adsec_error.ok then return, err
        
        old_ovs = rtr.oversampling_time
        err = set_diagnostic(over=0.0, TIM=2400)
        if err ne adsec_error.ok then return, err  
        wait, 0.1
         err = set_disturb(/disable)
         if err ne adsec_error.ok then return, err
        wait, 0.1
         err = wfs_switch_step(/DIS_DIS) ;???doppio comando
         if err ne adsec_error.ok then return, err
        wait, 0.1
         err = wfs_switch_step(/DIS_DIS)
         if err ne adsec_error.ok then return, err
         err = wait_for_wfsgt()
        wait, 0.1
         if err ne adsec_error.ok then return, err
         err = set_diagnostic(over=def.ovs)
         if err ne adsec_error.ok then return, err
        wait, 0.1
         tracknum=''
         err =  make_chop_disturb(/send, /save, ovstime=def.ovs, TRACK=track, CHOP_A=pos_a, CHOP_B=pos_b, HIST=hist)
         if err ne adsec_error.ok then return, err

        for i=0, nstep-1 do begin
        wait_loop  =  0
             while file_test(file4d) ne 0 do begin
                
                log_print, 'A TRACKNUM file was found! Waiting for the end of the previous sampling.....'
                log_print, 'Waiting loop.... '+strtrim(wait_loop,2)
                wait_loop  +=1
                wait,3
             endwhile
             wait,3
             
             opdbase=get_img4d(nmeas=10, out=maskbase)

             log_print, 'Setting if trigger to', def.dec_factor+ offset[i]
             err = set_if_trigger(def.dec_factor, offset[i])
             if err ne adsec_error.ok then return, err
             track=strmid(add_timestamp(''),0,15)
;             err =  make_chop_disturb( /send, /save, ovstime=def.ovs, TRACK=track, CHOP_A=pos_a, CHOP_B=pos_b)
             tracknum=[tracknum,track]

             if intflag then begin
                 openw, unit,file4d, /get_lun,  bufsize=0
                 printf, unit, track
                 close,/all
                 free_lun, unit
             endif
         
             err = set_diagnostic(over=0.0, TIM=2300)
             if err ne adsec_error.ok then return, err
            wait, 0.1
             err = set_disturb(/enable)
             if err ne adsec_error.ok then return, err
            wait, 0.1
             err = wfs_switch_step(/ENA_DIS)
             if err ne adsec_error.ok then return, err
             err = wfs_switch_step(/ENA_DIS)
             if err ne adsec_error.ok then return, err
            wait, 0.1
             err = wait_for_wfsgt()
             if err ne adsec_error.ok then return, err
            
             log_print, 'Chopping commands started'
             err = set_diagnostic(over=def.ovs)
             if err ne adsec_error.ok then return, err

            nacq=def.nframes
             if not keyword_set(manual) then begin
                if intflag then begin
                    err = idl_4d_init()
                    if err ne adsec_error.ok then message, "ERROR!!"
                    log_print, 'Acquiring RAW frames from 4D interferometer'
                    log_print, nacq
                    log_print, idl_4d_capture(nacq, 'acq')
                    log_print, '4D Capturing completed'
                    err = idl_4d_finalize()
                    if err ne adsec_error.ok then message, "ERROR!!"
                endif else begin
                    wait, (rtr.oversampling_time *def.dec_factor*nacq )*1.2
                endelse
             endif else begin
                wait, (rtr.oversampling_time *def.dec_factor*nacq )*1.2
             endelse    
             ;print, save_master(/DUMP)
             if keyword_set(SAVEDIAG) then begin
                 nf = round((nframes+1)*def.dec_factor*1.1)
                 err = get_diag_frames(nf, data, /DSP, /SWITCH, /ACC)            
                 save, file=meas_path('chopping', date=track)+'diag_data.sav', data
             endif
 
             log_print, 'Chopping commands terminated'
             err = set_diagnostic(over=0.0, TIM=2400)
             if err ne adsec_error.ok then return, err
            wait, 0.1       
             err = set_disturb(/disable)
             if err ne adsec_error.ok then return, err
            wait, 0.1
             err = wfs_switch_step(/DIS_DIS)
             if err ne adsec_error.ok then return, err
            wait, 0.1
             err = wfs_switch_step(/DIS_DIS)
             if err ne adsec_error.ok then return, err
            wait, 0.1
            err = set_disturb(/reset, /counter)
            if err ne adsec_error.ok then return,err
            wait, 0.1    
             err = wait_for_wfsgt()
             if err ne adsec_error.ok then return, err
             err = set_diagnostic(over=def.ovs)
             if err ne adsec_error.ok then return, err
         
             resultdir='$ADOPT_MEAS/adsec_calib/if_functions/'+track+'/'
             file_mkdir, resultdir
             file_des=resultdir+'info.txt'
    ;    here begins the creation of the log_file
             ;file_copy,default_file , resultdir+'info.txt'
             info=create_if_conf(/dochop, num_of_frames=nacq, offset=offset[i])
             openw, unit, file_des, /get_lun, bufsize=0
             printf, unit, info
             close,/all
             free_lun, unit
            
             logfile='$ADOPT_MEAS/adsec_calib/'+def.log4d
             info=create_if_conf(/short, track=track,num_of_frames=nacq, /dochop, offset=offset[i])
             openw, unit1, logfile, /get_lun, bufsize=0,/append
             printf, unit1, info
             close,/all
             free_lun, unit1
    ;     endfor to be removed????
         
             err = set_diagnostic(over=old_ovs)
             if err ne adsec_error.ok then return, err
             err = set_if_trigger(0, 0)
             if err ne adsec_error.ok then return, err
             save, file=meas_path('chopping', date=track)+'opdref.sav', opdbase, maskbase
         
        endfor  ;?????
        if n_elements(tracknum) gt 1 then  tracknum=tracknum[1:*]

        err=set_if_trigger(0, 0)
        if err ne adsec_error.ok then return, err
        
        err  = read_seq_ch(sc.all, dsp_map.ff_pure_current, 1l, bb)
        if err ne adsec_error.ok then return, err
        ffpure=bb
        err = test_skip_frame(safe=safecnt2)
        if err ne adsec_error.ok then return, err
        if (safecnt2 ne safecnt) or max(abs(bb)) gt 0 then begin
           ; err= set_diagnostic(over=0)
            log_print, "Not null FF PURE CURRENT detected during measurements: ";+string(bb, format='(F10.8)')
            ;err  = write_seq_ch(sc.all, dsp_map.ff_pure_current, lonarr(672), /check)
            ;err = set_diagnostic(over=old_ovs)
        endif
        


       ; resultdir='$ADOPT_MEAS/adsec_calib/if_functions/'+track+'/'
       ; file_mkdir, resultdir
       ; file_des=resultdir+'info.txt'
       ; info=create_if_conf(form, amprms=maxrms)
       ; openw, unit, file_des, /get_lun, bufsize=0
       ; printf, unit, info
       ; close,/all
       ; free_lun, unit
        if n_elements(tracknum) eq 1 then tracknum = tracknum[0]
        log_print, 'Tracking number is   '+strjoin(tracknum, ' -- ')
     return, err

end
