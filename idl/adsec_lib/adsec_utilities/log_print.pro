Pro log_print, log_str,astr, bstr, cstr, dstr, estr,  LOG_LEV=log_lev


    on_error, 2

    log_str = string(log_str)
    
    help, /dlm, 'idl_wraplib', OUTPUT=out
    str= stregex(out[0], '\(.*\)', /EXT)
    
    if str eq "(loaded)" then begin

        print, log_str
        if n_elements(log_lev) eq 0 then err = idl_log_wrap(log_str) $
                                    else err = idl_log_wrap(log_lev, log_str)
        if err ne 0 then message, "LOGGING ERROR.... Returning to main"

    endif else begin
        
        print, log_str

    endelse

    return

End
