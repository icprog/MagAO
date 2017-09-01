; da testare
Function test_time_hist_get, XADSEC=xadsec, OLD_OVS=old_ovs

    @adsec_common
;    nsamples = rtr.disturb_len
;    WORK_FREQ = 1000.
    if n_elements(old_ovs) eq 0 then old_ovs=0.03
    ;restore, meas_path('optical_loop')+'data_to_reduce.sav', /ver


    err = set_diagnostic(over=0.0)
    if err ne adsec_error.ok then return, err
    err = set_disturb(/DISABLE)
    if err ne adsec_error.ok then return, err
    err = dspstorage(/DISABLE)
    if err ne adsec_error.ok then return, err

    err = set_diagnostic(over=old_ovs)
    if err ne adsec_error.ok then return, err
    n_samples2restore=19000
    ;err = load_diag_data_dsp(4*nsamples+1, data_reduced, /NOFASTCHECK, /READFC, XADSEC=xadsec)
    err = load_diag_data_dsp(n_samples2restore+1, data_reduced, /NOFASTCHECK, /READFC, XADSEC=xadsec)
    if err ne adsec_error.ok then return, err

    ;err = load_diag_data_switch(nsamples+1, data_reduced_sw, /NOFASTCHECK, /READFC, XADSEC=xadsec)
    err = load_diag_data_switch(n_samples2restore+1, data_reduced_sw, /NOFASTCHECK, /READFC, XADSEC=xadsec)
    if err ne adsec_error.ok then return, err

;    err = set_diagnostic(over=0.0)
;    if err ne adsec_error.ok then return, err
;    err = set_disturb(/DISABLE)
;    if err ne adsec_error.ok then return, err

    err = set_diagnostic(over=0.0)
    if err ne adsec_error.ok then return, err

    err = dspstorage()
    if err ne adsec_error.ok then return, err

    err = set_diagnostic(over=old_ovs)
    if err ne adsec_error.ok then return, err


    save, FILE=meas_path('optical_loop')+'slope_1000hz_skip_frame.sav', data_reduced, nsamples, work_freq, base_frame, time_hist, data_reduced_sw, hpos, hfor, status_save, status_save2


    return, adsec_error.ok

End

