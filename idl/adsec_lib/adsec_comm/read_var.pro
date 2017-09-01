;+
; MODIFICATION HISTORY
; 18 Oct 2010: A.R.
; removed string ".C." in case of common variables to be compliant with AO5 on
Function read_var, varname, out, COMMONVAR=commonvar, SUBS=subs

    @adsec_common
    if keyword_set(commonvar) then side = 'C' else side = getenv("ADOPT_SIDE")
    if n_elements(subs) eq 0 then subs = 'ADSEC'
    timeout = 100L

    if keyword_set(COMMONVAR) then begin
        err = read_var_wrap(subs+'.'+varname, out, timeout)
    endif else begin
        err = read_var_wrap(subs+'.'+side+'.'+varname, out, timeout)
    endelse
    if err ne adsec_error.ok then return, err

End
