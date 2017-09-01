Function select_wfs_port, port

    @adsec_common
    if n_elements(port) eq 0 then port=1
    port= ulong(port)
    if port lt 0 or port gt 3 then return, adsec_error.input_type
    sport = ishft(port, 16)
    data = [2UL^27, sport]
    log_print, "Set  WFS port #"+string(port, format='(I1)')+" on switch BCU", LOG_LEV=!AO_CONST.log_lev_info
    err = reset_devices_wrap(sc.mirrctrl_id[adsec.n_bcu], 255, 255, 0, 2L, data)
    if err ne adsec_error.ok then return, err
    log_print, "WFS port #"+string(port, format='(I1)')+" on switch BCU successfully set.", LOG_LEV=!AO_CONST.log_lev_info
    return, adsec_error.ok

End
 
