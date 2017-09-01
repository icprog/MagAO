Function fsm_disable_disturb

    @adsec_common
    if sc.name_comm eq "Dummy" then return, adsec_error.ok
    log_print, "------------------------ DISABLING NOISE ADDING --------------------------"
    err= set_disturb(/DIS)
    if err ne adsec_error.ok then return, err

    err = set_disturb(/RESET, /COUNTER)
    if err ne adsec_error.ok then return, err

    log_print, "------------------------          DONE          --------------------------"
    return, adsec_error.ok


End
