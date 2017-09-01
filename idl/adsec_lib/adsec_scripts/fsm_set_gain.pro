Function fsm_set_gain, gain_fits, BLOCK=block

    @adsec_common
    if ~((file_info(gain_fits)).exists) then return, adsec_error.invalid_file
    g_gain = readfits(gain_fits)
    if n_elements(g_gain) ne adsec.n_actuators then begin
        log_message, "Wrong gain dimension.", CONT = (sc.debug eq 0), ERR=adsec_error.input_type
        return, adsec_error.input_type
    endif
    err = set_g_gain(sc.all, g_gain, BLOCK=block, FITS=gain_fits)
    if err ne adsec_error.ok then return, err

    return, adsec_error.ok

end 

