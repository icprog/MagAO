Function collect_step_data

    @adsec_common
    tn = tracknum()
    err = write_var('LAST_MEAS_TRACKNUM', tn)
    if err ne adsec_error.ok then return, err

    err = fsm_set_flat()
    if err ne adsec_error.ok then return, err
    
    err = step_acquire(tn)
    if err ne adsec_error.ok then return, err

    err = fsm_rip()
    if err ne adsec_error.ok then return, err

    return, adsec_error.ok

End

