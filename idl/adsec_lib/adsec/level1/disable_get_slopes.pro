Function disable_get_slopes

    @adsec_common
    log_print, "------- Disable slopes receiving from SLOPE COMPUTER -------"
    err = write_same_dsp(0, '100000'xl, lindgen(4))
    if err ne adsec_error.ok then begin
        log_message, "Error disable slopes receiving.", CONT = (sc.debug eq 0)
        return, err
    endif
    err = write_same_dsp(0, switch_map.SCStartRTRPTR, '100000'xl, /CHECK, /SWITCH)
    if err ne adsec_error.ok then begin
        log_message, "Error disable slopes receiving.", CONT = (sc.debug eq 0)
        return, err
    endif
    log_print, "------------------------  Done -----------------------------"
    return, adsec_error.ok 

End
