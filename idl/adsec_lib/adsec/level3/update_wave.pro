Function update_wave, filename
    
    @adsec_common
    if n_elements(filename) eq 0 then  filename=filepath(ROOT=adsec_path.commons, sub=['ref_wave'], 'wave_1o8.txt')
    old_ovs=rtr.oversampling_time 
    if old_ovs gt 0 then begin
        err = set_diagnostic(OVER=0, TIM=5000)
        if err ne adsec_error.ok then return, err
        log_print, 'Oversampling frames disabled.'
    endif
    log_print, 'Disabling DSP WATCHDOG...'
    err = disable_watchdog(/DSP)
    if err ne adsec_error.ok then begin
        log_print, '... error on  DSP watchdog disabling.'
        return, err 
    endif
    log_print, '... done.'

    err = init_adsec_wave(filename)
    if err ne adsec_error.ok then begin
        log_print, 'Reference wave parameters initialization failed!'
        return, err
    endif
    log_print, 'Uploading reference wave paramenters...'

    err = send_wave()
    if err ne adsec_error.ok then begin
        log_print, 'Reference wave parameters initialization failed!'
        return, err
    endif

    err =  init_adsec_cals()
    if err ne adsec_error.ok then begin
        log_print, 'Calibration failed!'
        return, err
    endif

    err = send_linearization()
    if err ne adsec_error.ok then begin
        log_print, '... error on uploading the linearization coeffs.'
        return, err
    endif
    log_print, '... done.'
    log_print, 'Enabling DSP WATCHDOG...'

    err = enable_watchdog(/DSP)
    if err ne adsec_error.ok then begin
        log_print, '... error on  DSP watchdog enabling.'
        return, err
    endif
    log_print, '... done.'
    err = update_status()
    if err ne adsec_error.ok then begin
        log_print, '... error on  status updating'
        return, err
    endif
    wait, 0.1 
    if old_ovs gt 0 then begin
        err = set_diagnostic(OVER=old_ovs, TIM=1000)
        if err ne adsec_error.ok then return, err
        log_print, 'Oversampling frames disabled.'
    endif

End
