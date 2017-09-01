Function update_msg, msg, varname, val, LEV=lev, CLEAR=clear, POPMSG=popmsg
    
    @adsec_common
    subs = (strsplit(process_list.adsecarb.msgd_name, '.', /EX))[0]

    if keyword_set(clear) then begin
        val=0 
        msg=""
    endif
    if keyword_set(popmsg) then begin
        if n_elements(varname) gt 0 then begin 
            err = write_var(varname, val, subs=subs)
            if err ne adsec_error.ok then return, err
        endif
        if n_elements(msg) ne 0 then begin
            err = write_var('POPMSG', msg, subs=subs)
            if err ne adsec_error.ok then return, err
        endif
    endif else begin
        if n_elements(lev) eq 0 then lev = 3
        if n_elements(varname) gt 0 then begin 
            err = write_var(varname, val, subs=subs)
            if err ne adsec_error.ok then return, err
        endif
        if n_elements(msg) ne 0 then begin
            err = write_var('MSG', string(lev, format='(I1)')+":"+msg, subs=subs)
            if err ne adsec_error.ok then return, err
        endif
    endelse
    
    return, adsec_error.ok

End
