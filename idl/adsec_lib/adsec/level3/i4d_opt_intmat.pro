;function wait_for_wfsgt
;
    
;    @adsec_common
;    expired = 0
;    cnt = 0
;    bb = 0
;    while (~expired) or (cnt ge 30) do begin
;        wait, 0.02
;        err = read_seq_dsp(0, switch_map.wfsglobaltimeout, 1l, /switch, bb, /UL)
;        if err ne adsec_error.ok then return, err
;        if bb eq 1 then begin
;            expired = 1
;            print, "WfsGlobalTimeout expired. Proceeding"
;        endif
;        cnt +=1
;    endwhile
;    if cnt gt 30 then begin
;        log_print, "I'm waiting too long......"
;        return, adsec_error.generic_error
;    endif else begin
;        return, adsec_error.ok
;    endelse
;        
;
;End

function i4d_opt_intmat,from, to, SIMULATION=simulation, AMPRMS=amprms, AVERAGE=average, KL=kl, ZONAL=zonal, TRACKNUM=tracknum, MANUAL=manual, FFpure=ffpure, SAVEDIAG=savediag, VERBOSE=verbose, passedbase=passedbase
    
    @adsec_common
    default_file=adsec_path.conf+'scripts/4d_opt_intmat_default.txt'
    def=read_ascii_structure(default_file)
    file4d='$ADOPT_MEAS/adsec_calib/4d/'+def.folder4d+def.file4d
    if file_test(file4d) ne 0 then begin
        log_print, 'A TRACKNUM file was found! Delete it or wait last acquisition to be completed'
        return, -1
    endif
    maxrms=def.mode_amp
    stemplate=''
    if (n_elements(amprms) ne 0) then maxrms=amprms
    if (n_elements(from) eq 0 and n_elements(to) eq 0) then begin
        nstep=def.nstep
        nmodes=def.nmodes
        nn=intarr(2,nstep)
        stepbase=fix(nmodes/nstep)
        for i=0, nstep-1 do nn[*,i]=[i*stepbase,(i+1)*stepbase-1]
        rem = nmodes-stepbase*nstep
        nn[1,nstep-1]+=rem
    endif else begin
        nstep=1
        nn = [from, to]
        nmodes=to-from+1
    endelse

    if n_elements(average) ne 0 then begin
            template=intarr(average)-1
            template[0:*:2] = 1
            form=template
    endif else begin
            form=def.template
    endelse
    
    string2show='Template size= '+strtrim(n_elements(form))+', OVS= '+strtrim(def.ovs,2)+' Dec_Factor= '+strtrim(def.dec_factor, 2) + 'NStep= '+strtrim(nstep,2)+'NModes= '+strtrim(nmodes,2)

       log_print, string2show
       if keyword_set(simulation) then begin
                    def.dec_factor=3
                    def.ovs=0.02
        endif


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
         err = set_if_trigger(def.dec_factor, def.offset)
         if err ne adsec_error.ok then return, err
         tracknum=''
         
         for i=0, nstep-1 do begin  ;was def.nstep
             nmodes_thisrun = nn[1,i]-nn[0,i]+1
            go=1
            while go eq 1 do begin
              err =  make_modal_disturb(nn[0,i],nn[1,i], /send, template=form, /save, trigg_amp=def.trigmode_amp, ini_0=def.ini_zeros, end_0=def.end_zeros, dec_factor=def.dec_factor, MAX_rms=maxrms, TRACK=track, KL=kl, ZONAL=zonal, passedbase=passedbase)
            log_print, 'Tracknum =     '+track
             tracknum=[tracknum,track]
             if err ne adsec_error.ok then return, err
            if ~keyword_set(manual) then begin
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
            
             log_print, 'Modal commands started, from mode '+strtrim(nn[0,i], 2)+', to mode '+strtrim(nn[1,i],2)
             err = set_diagnostic(over=def.ovs)
             if err ne adsec_error.ok then return, err

             if not keyword_set(manual) then begin
                err = idl_4d_init()
                 if err ne adsec_error.ok then message, "ERROR!!"
                 log_print, 'Acquiring RAW frames from 4D interferometer'
                 nacq=(n_elements(form)*nmodes_thisrun+def.ini_zeros+def.end_zeros+1 )+25
                 log_print, nacq
                 log_print, idl_4d_capture(nacq, 'acq')
                 log_print, '4D Capturing completed'
                 err = idl_4d_finalize()
                 if err ne adsec_error.ok then message, "ERROR!!"
             endif else begin
                wait, rtr.oversampling_time *def.dec_factor*(n_elements(form)*nmodes_thisrun+def.ini_zeros+def.end_zeros+1 )*1.2
             endelse    
             if keyword_set(SAVEDIAG) then begin
                 err = get_diag_frames(10000, data, /DSP, /SWITCH)            
                 save, file=meas_path('disturbance', date=track)+'diag_data.sav', data
             endif

             log_print, 'Modal commands terminated'
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
             err = wait_for_wfsgt()
        wait, 0.1
             if err ne adsec_error.ok then return, err
             err = set_diagnostic(over=def.ovs)
             if err ne adsec_error.ok then return, err
        wait, 0.1
         err = set_diagnostic(over=old_ovs)
         if err ne adsec_error.ok then return, err    
         err = set_if_trigger(0, 0)
         if err ne adsec_error.ok then return, err


        go=0    
        if keyword_set (verbose) then begin
            ans=dialog_message('Do you want to save and continue?',/quest)                
            if (ans eq 'No') then go=1
            
        endif

        endwhile
             resultdir='$ADOPT_MEAS/adsec_calib/if_functions/'+track+'/'
             file_mkdir, resultdir
             file_des=resultdir+'info.txt'
             ;file_copy,default_file , resultdir+'info.txt'
             info=create_if_conf(form, amprms=amprms)
             openw, unit, file_des, /get_lun, bufsize=0
             printf, unit, info
             close,/all
             free_lun, unit
            
             logfile='$ADOPT_MEAS/adsec_calib/'+def.log4d
             info=create_if_conf(form, amprms=maxrms,/short, track=track, KL=kl,usermodes=keyword_set(passedbase), nmodes=[nn[0,i],nn[1,i]])
             openw, unit1, logfile, /get_lun, bufsize=0,/append
             printf, unit1, info
             close,/all
             free_lun, unit1
         endfor
         
         trackum=tracknum[1:*]
         
         if keyword_set(simulation) then begin 
            err=fsm_rip()
            if err ne adsec_error.ok then return, err
         endif

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
    log_print, 'Tracking number is   '+track    
 return, err

end
