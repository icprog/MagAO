Function fsm_load_disturb, disturb_file_fits

    @adsec_common
   
    if ~((file_info(disturb_file_fits)).exists) then return, adsec_error.invalid_file
    if sc.name_comm eq "Dummy" then return, adsec_error.ok

    ;TEST on disturbance status
    err = read_seq_dsp(0, rtr_map.param_selector, 1L, bb, /UL)
    if err ne adsec_error.ok then return, err
    if ((bb and 2L^12) gt 0) or ((bb and 2L^15) gt 0) then begin
        log_message, "The disturbance command application is still enabled. Please stop it and retry.", CONT = (sc.debug eq 0), err=adsec_error.IDL_DIST_ACTIVE
        return, !AO_ERROR
    endif

    ;m_time_hist = readfits(disturb_file_fits)
    ;m2c_mat = float(adsec.ff_p_svec)
    ;time_hist = float(m2c_mat ## m_time_hist)
    time_hist = readfits(disturb_file_fits)
    st = size(time_hist, /DIM)
    if test_type(time_hist, /FLOAT, /DOUBLE) then begin
        log_print, "ERROR: The disturbance vector must be float of size TIMExN_ACT"
        log_message, "The disturbance vector must be float of size TIMExN_ACT", CONT = (sc.debug eq 0), ERR=adsec_error.input_type
        return, adsec_error.input_type
    endif
    time_hist = float(time_hist)
        
    if st[0] ne rtr.disturb_len or st[1] ne adsec.n_actuators then begin
        log_print, "Wrong disturbance length (should be "+strtrim(rtr.disturb_len,2)+"x"+strtrim(adsec.n_actuators,2)+"). Not applied"
        return, adsec_error.input_type
    endif

    if max(abs(time_hist)) gt  15e-6 then begin
        log_print, " ----------------------- TOO LARGE AMPLITUDE! Returning -------------------------- "
        return, adsec_error.input_type
    endif

    if max(abs(adsec.ff_matrix ## time_hist)) gt 0.78 then begin
        log_print, " ----------------------- TOO LARGE FORCE! Returning --------------------------- "
        return, adsec_error.input_type
    endif


;    err = set_disturb(/MASTER_DISABLE)
;    if err ne adsec_error.ok then return, err
;    err = set_disturb(/DISABLE)
;    if err ne adsec_error.ok then return, err
    wait,0.1
    err = set_disturb(/RESET, /COUNTER)
    if err ne adsec_error.ok then return, err
    wait,0.1

    err = set_disturb(time_hist, FITS=disturb_file_fits)
    wait,0.1
    if err ne adsec_error.ok then return, err

    !AO_STATUS.disturb = disturb_file_fits
    return, update_rtdb()

End
