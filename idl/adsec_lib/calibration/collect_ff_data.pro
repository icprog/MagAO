Function collect_ff_data, NO_FLAT=no_flat

    @adsec_common
    tn = tracknum()
    err = fsm_set_flat(/FF, NO_FLAT=no_flat)
    if err ne adsec_error.ok then return, err

    wait, 10
    
    err = ff_acquire_v2(tn)
    if err ne adsec_error.ok then return, err

    err = write_var('LAST_MEAS_TRACKNUM', tn)
    if err ne adsec_error.ok then return, err
    log_print, "Measure done: "+tn

    err = fsm_rip()
    if err ne adsec_error.ok then return, err

    return, adsec_error.ok

End

