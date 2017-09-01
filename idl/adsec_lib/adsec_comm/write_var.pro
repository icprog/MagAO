Function write_var, varname, values, COMMONVAR=commonvar, SUBS=subs

;   - INT_VARIABLE: 1001 (16 bit IDL_INT, 32 bit IDL_LONG)
;   - REAL_VARIABLE: 1002 (32 bit IDL_FLOAT, 64 bit IDL_DOUBLE)
;   - CHAR_VARIABLE: 1003

    @adsec_common
    if keyword_set(commonvar) then side = 'C' else side = getenv("ADOPT_SIDE")
    if n_elements(subs) eq 0 then subs = 'ADSEC'
    sv = size(values)
    id = sv[n_elements(sv)-2]
    timeout = 100L  ;[ms]

    case id of
        2:  type=1001
        3:  type=1001
        12: type=1001
        13: type=1001
        4:  type=1002
        5:  type=1002
        7:  type=1003
    else: return, adsec_error.input_type
    endcase

    if keyword_set(COMMONVAR) then begin
        ;err = write_var_wrap(side+'.'+subs+'.'+varname,  values, type, timeout)
        err = write_var_wrap(subs+'.'+varname,  values, type, timeout)
    endif else begin
        err = write_var_wrap(subs+'.'+side+'.'+varname,  values, type, timeout)
    endelse
    return, err

End
