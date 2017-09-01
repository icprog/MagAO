Function fsm_get_slopes, filename, nsamples

    @adsec_common
    ;;TEST FOR NO SC ACTIVITIES IN PROGRESS
    err = test_skip_frame(/NOVERB, WFS_CNT = cnt0, PENDING=psf0, SAFE=ssf0)
    if err ne adsec_error.ok then return, err
    wait, 0.2
    err = test_skip_frame(/NOVERB, WFS_CNT = cnt1, PENDING=psf1, SAFE=ssf1)
    if err ne adsec_error.ok then return, err


    if (cnt1 - cnt0) ne 0 then begin
        log_message, "Slope Computer is now routing slopes do the Adaptive Secondary. Please stop it and retry.", CONT = (sc.debug eq 0), ERR=adsec_error.IDL_SC_SENDING_SLOPES
        return, adsec_error.IDL_SC_SENDING_SLOPES
    endif


    if n_elements(nsamples) gt 0 then ns = nsamples < 20000 else ns = 20000
    old_ovs = rtr.oversampling_time
    err = set_diagnostic(over=0.0)
    if err ne adsec_error.ok then return, err
 
    n_samples2restore = ns
    err = load_diag_data_switch(n_samples2restore, drw, /NOFASTCHECK, /READFC)
    if err ne adsec_error.ok then return, err
 
    err = set_diagnostic(over=old_ovs)
    if err ne adsec_error.ok then return, err

    stat = !AO_STATUS
    save, FILE=filename, drw, ns, stat
    return, adsec_error.ok

End

