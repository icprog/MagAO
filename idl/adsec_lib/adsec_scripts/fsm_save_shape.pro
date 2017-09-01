Function fsm_save_shape, name, instrument, NF=nf, USERMEAN=usermean, TIME2MEAN=time2mean, MARKASDEFAULT=markasdefault
    
    @adsec_common
    err = update_status()
    if err ne adsec_error.ok then return, err
    flattened_status = sys_status
    if keyword_set(usermean) then begin
        if n_elements(TIME2MEAN) eq 0 then time2mean = 10
        if time2mean gt 100 or time2mean lt 0 then begin
            log_print, "Time requested too high. Returning."
            return, adsec_error.IDL_INPUT_TYPE
        endif
        err = getdiagnparam(process_list.fastdiagn.msgd_name, 'ChDistAverage ChCurrAverage', -1,-1, bufp)
        if err ne adsec_error.ok then return, err
        err = setdiagnparam(process_list.fastdiagn.msgd_name, 'ChDistAverage ChCurrAverage', -1,-1, MEAN_PERIOD=time2mean)
        if err lt 0 then return, err
        wait, time2mean
        ;NB: you cannot get the buffer because exceed MSGD-RTDB max message length
        err = getdiagnvalue(process_list.fastdiagn.msgd_name, 'ChDistAverage ChCurrAverage', -1,-1, val)
        if err ne adsec_error.ok then return, err
        err = setdiagnparam(process_list.fastdiagn.msgd_name, 'ChDistAverage ChCurrAverage', -1,-1, MEAN_PERIOD=0)
        if err lt 0 then return, err
        idpos = where(strlowcase(val.family) eq 'chdistaverage')
        idcur = where(strlowcase(val.family) eq 'chcurraverage')
        clact = where(flattened_status.closed_loop, compl=n_clact)

        
        
        position = val[idpos].average
        current  = val[idcur].average
        position_sd = val[idpos].stddev
        current_sd  = val[idcur].stddev
        

        flattened_status.position[clact] = position[clact]
        flattened_status.position[n_clact] = 0.
        flattened_status.position_sd[clact] = position_sd[clact]
        flattened_status.position_sd[n_clact] = 0.
        flattened_status.current[clact] = current[clact]
        ;flattened_status.current[n_clact] = 0.
        flattened_status.current_sd[clact] = current_sd[clact]
        flattened_status.current_sd[n_clact] = 0.
        pos_cube='Undefined'
        cur_cube='Undefined'

    endif else begin

        if n_elements(nf) eq 0 then nf =1d
        pos = sys_status.position/nf
        cur = sys_status.current/nf
        pos_cube = fltarr(nf, adsec.n_actuators)
        cur_cube = fltarr(nf, adsec.n_actuators)
        pos_cube[0,*] = sys_status.position
        cur_cube[0,*] = sys_status.current

        for i=1, nf-1 do begin
            log_print, "Iteration "+string(i, FORMAT='(I5.5)')+" "+string(nf, FORMAT='(I5.5)')
            err = update_status()
            if err ne adsec_error.ok then return, err
            pos += sys_status.position/nf
            cur += sys_status.current/nf
            pos_cube[i,*] = sys_status.position
            cur_cube[i,*] = sys_status.current
            wait, 0.1
        endfor
        flattened_status.position = pos
        flattened_status.current = cur
    endelse


    ;newname = get_flat_shortname(instrument, name)
    newname = name
    ffn = meas_path('flat')+newname
    if (file_info(ffn)).exists eq 1 then return, adsec_error.IDL_INPUT_TYPE
    save, FILE=ffn, flattened_status, pos_cube, cur_cube
    linkfile = filepath(root=adsec_path.meas, sub=['flat','Applicable'], file_basename(ffn))
    if (file_info(linkfile)).exists eq 1 then file_delete, linkfile
    file_link, ffn, linkfile
    if keyword_set(MARKASDEFAULT) then begin
        file_delete, filepath(root=adsec_path.meas, sub=['flat','Applicable'], 'default.sav')
        file_link, ffn, filepath(root=adsec_path.meas, sub=['flat','Applicable'], 'default.sav')
    endif
;    newname = meas_path('flat')+shortname+'.'+file_basename(name)
    !AO_STATUS.LAST_FLAT_WRITTEN = newname
;    !AO_STATUS.LAST_FLAT_LOADED  = shortname
    
    if sc.name_comm eq "Dummy" then return, adsec_error.ok

    err = update_flat_table(/READ)
    if err ne adsec_error.ok then return, err
    
    return, update_rtdb()

end
