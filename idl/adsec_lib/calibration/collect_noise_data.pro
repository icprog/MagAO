Function collect_noise_data

    @adsec_common
    ao_status = ptr2value(!AO_STATUS)
    tn = tracknum()
    err = write_var('LAST_MEAS_TRACKNUM', tn)
    if err ne adsec_error.ok then return, err

    samples = 2048
    times = 3
    dir = meas_path('noise', DAT=tn)

    ;updating the status of IDL registers.
    err = update_status()
    if err ne adsec_error.ok then return, err
    ;Control the closed loop actuators
    cl_act = adsec.act_w_cl
    err = test_noise(times, max_min, sigma_pos, coeff, covar,SAMPLES=samples, NO_LIN=1)
    if err ne adsec_error.ok then return, err
    adsec_save = adsec
    err = get_status(status_save)
    name = dir+'noise_notlin.sav'
    save, file=name, m_covar, m_covar_mir, sigma_pos, sigma_pos_reb, covar, adsec_save, status_save, max_min, ao_status

    err = test_noise(times, max_min, sigma_pos, coeff, covar,SAMPLES=samples, NO_LIN=0)
    if err ne adsec_error.ok then return, err
    adsec_save = adsec
    err = get_status(status_save)
    name = dir+'noise_lin.sav'
    save, file=name, m_covar, m_covar_mir, sigma_pos, sigma_pos_reb, covar, adsec_save, status_save, max_min, ao_status

End

