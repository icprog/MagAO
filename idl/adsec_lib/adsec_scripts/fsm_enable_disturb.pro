Function fsm_enable_disturb

    @adsec_common
    if sc.name_comm eq "Dummy" then return, adsec_error.ok
    log_print, "------------------------ ENABLING NOISE ADDING --------------------------"
;    err = set_disturb(/RESET, /COUNTER)


;    err= set_disturb(/MASTER_ENA)
;    if ne adsec_error.ok then return, err

    err= set_disturb(/ENA)
    if err ne adsec_error.ok then begin
        log_print, "--------------------------- FAILED TO ENABLE NOISE ADDING ------------------"
        log_message, "Returning", CONT= (sc.debug eq 0), ERR=err
        return, err
    endif
    log_print, "------------------------          DONE         --------------------------"
    return, adsec_error.ok


End
