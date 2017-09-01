Function enable_get_slopes

    @adsec_common
    log_print, "------- Enable slopes receiving from SLOPE COMPUTER -------"
    err = write_same_dsp(0, switch_map.SCStartRTRPtr, rtr_map.switch_SCStartRTR, /CHECK, /SWITCH)
    if err ne adsec_error.ok then begin
        log_message, "Error enabling slope receiving.", CONT= (sc.debug eq 0)
        return, err
    endif
    log_print, "------------------------  Done -----------------------------"
    return, adsec_error.ok 

End
