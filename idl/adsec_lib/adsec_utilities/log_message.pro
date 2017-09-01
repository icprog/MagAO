Pro log_message, log_str, LOG_LEV=log_lev, CONT=cont, ERRCODE=errcode


    on_error, 2
    @adsec_common
    if n_elements(errcode) eq 0 then begin
        errcode = adsec_error.generic_error
    endif

    help, /trace
    !AO_ERROR = errcode
    help, /dlm, 'idl_wraplib', OUTPUT=out
    str= stregex(out[0], '\(.*\)', /EXT)
    
    if str eq "(loaded)" then begin

        if n_elements(log_lev) eq 0 then err = idl_log_wrap(log_str) $
                                    else err = idl_log_wrap(log_lev, log_str)
        print, log_str
        print, lao_strerror_wrap(errcode)
        err = idl_log_wrap(lao_strerror_wrap(errcode))
        if err ne 0 then message, "LOGGING ERROR.... Returning to caller routine.", CONT=cont
        message, " -----------> "+log_str, LEVEL=-1, CONT = cont

    endif else begin
        
        message, " -----------> "+log_str, LEVEL=-1, CONT = cont

    endelse

    return

End
