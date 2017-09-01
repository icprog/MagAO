Function test_hkpr

    @adsec_common
    time = systime(/SEC)
    is_not_ok =1
    log_print, process_list.HOUSEKEEPER.MSGD_NAME +" test started."
    while ((systime(/sec) - time) lt 60L) and is_not_ok do begin
        wait, 11
        err = getdiagnvalue(process_list.HOUSEKEEPER.MSGD_NAME, 'checkdewpoint', 0, 0, bb )
        if n_elements(bb) ne 0 then begin
            if err ne adsec_error.ok or bb.last lt 0 or bb.last gt 200 or finite(bb.last, /NAN) then is_not_ok = 1 else is_not_ok = 0
        endif else begin
            is_not_ok=1
        endelse
        if is_not_ok eq 0 then break
        log_print, process_list.HOUSEKEEPER.MSGD_NAME +" non yet started or giving wrong reading in the CHECK DEW POINT variable (Nan, Inf, >200 deg or <0)."
        wait, 1
    endwhile
    if is_not_ok then begin
        if err ne adsec_error.ok then log_message, "The "+ process_list.HOUSEKEEPER.MSGD_NAME +" process not yet initialized.", ERR=adsec_error.IDL_HKPR_NOT_INIT, CONT=(sc.debug eq 0)
        if bb.status eq 2 then log_message, "The AdSec has severe dew point problems. The CHECK DEW POINT VARIABLE value is: "+string(bb.last), ERR=adsec_error.IDL_DEW_POINT_ERROR, CONT=(sc.debug eq 0)
        return, !AO_ERROR
    endif else begin
        log_print, process_list.HOUSEKEEPER.MSGD_NAME +" test done."
        log_print, "Set thresholds for check dew point variable: ALARM: 0, Inf C Deg"
        log_print, "Set thresholds for check dew point variable: WARINING: 2, 50 C Deg"
        err = setdiagnparam(process_list.HOUSEKEEPER.MSGD_NAME, 'checkdewpoint', 0, 0 $
                            , ALARM_MIN=0, WARNING_MIN=2, WARNING_MAX=50, ALARM_MAX=!values.D_INFINITY)
        wait, 5
        if err ne 1 then return, err else return, adsec_error.ok
    endelse

end
