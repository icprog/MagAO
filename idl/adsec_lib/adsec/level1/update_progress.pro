Function update_progress, val, msg, CLEAR=clear, LEV=lev
    
    @adsec_common
    subs = (strsplit(process_list.adsecarb.msgd_name, '.', /EX))[0]

    if keyword_set(clear) and n_elements(val) gt 0 then begin
        log_print, 'Bad request: CLEAR keyword and value set.', LOG_LEV=!AO_CONST.log_lev_error
        return, adsec_error.input_type
    endif
    if keyword_set(clear) then val=0 
    strp = string(val, format='(I3.3)')
    if n_elements(msg) gt 0 then begin
        strout = strp+':'+msg 
        err = write_var('PROGRESS', strout, subs=subs)
        if err ne adsec_error.ok then return, err
        err = update_msg(msg, LEV=lev)
        if err ne adsec_error.ok then return, err
    endif else begin
        strout=strp
        err = write_var('PROGRESS', strout, subs=subs)
        if err ne adsec_error.ok then return, err
    endelse
    return, adsec_error.ok

End
