Function test_ed_wfs

    @adsec_common
    log_print, 'Send slope one'
    err = wfs_switch_step() ;???doppio comando
    if err ne adsec_error.ok then return, err
    log_print, 'Send slope two'
    err = wfs_switch_step() ;???doppio comando
    if err ne adsec_error.ok then return, err
    log_print, 'Wait OVS start'
    wait, 0.3
    uu= test_skip_frame()
    ;log_print, 'Send slope three'
    ;err = wfs_switch_step() ;???doppio comando
    ;if err ne adsec_error.ok then return, err
    ;log_print, 'Send slope four'
    ;err = wfs_switch_step() ;???doppio comando
    ;if err ne adsec_error.ok then return, err
    ;wait, 0.3
    err = read_seq_dsp(0, switch_map.NumFlTimeout,1L , fl_tim, /SWITCH, /ULONG)
    if fl_tim gt 0 then return, -2
    err = getdiagnvalue(process_list.HOUSEKEEPER.MSGD_NAME , '*', -1,-1, hkpr_vars)
    idb = where(strmatch(hkpr_vars.family, '*Expired*', /FOLD)) 
    val= (hkpr_vars.last)[idb]
    if max(val) gt 0 then return, -1 else return, 0

End
