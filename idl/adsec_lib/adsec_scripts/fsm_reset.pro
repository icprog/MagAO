Function fsm_reset, PROGRESS=progress

    @adsec_common
    log_print, 'fsm_reset() procedure started.'
    if n_elements(progress) eq 0 then progress=[0,100]
    err = update_msg('Adsec '+!AO_CONST.unit+'-'+ !AO_CONST.telescope_side+' reset started.')
    if err ne adsec_error.ok then return, err
    cpr = progress[0]+fix(0.*(progress[1]-progress[0]))
    err = update_progress(cpr)
    if err ne adsec_error.ok then return, err

    log_print, "Disabling timeout on diagnostic frames accepted by fastdiagnostic..."
    err = write_scalar_var_wrap(strupcase(strmid(!AO_CONST.telescope_side,0,1))+'.ADSEC.OVS_P', 0L, 1001, 1000L)
    if err ne adsec_error.ok then return, err
    log_print, "Ok: timeout on frames disabled."

    err = setdiagnparam(process_list.HOUSEKEEPER.MSGD_NAME, 'checkdewpoint', 0, 0 $
                            , ALARM_MIN=-1e6, WARNING_MIN=-1e6, WARNING_MAX=1e6, ALARM_MAX=1e6)
    err = adam_sys_reset()
    if err ne adsec_error.ok then return, err
 
    cpr = progress[0]+fix(0.5*(progress[1]-progress[0]))
    err = update_progress(cpr)
    if err ne adsec_error.ok then return, err

    log_print, "Waiting for Crate Bcus ready..."
    err = isBcuReady(sc.mirrCtrl_id[0], sc.mirrCtrl_id[adsec.n_crates-1], timeout=30)
    if err ne adsec_error.ok then return, err
    log_print, "Ok. All Crate Bcus are ready"

    log_print, "Waiting for Switch Bcu ready..."
    err = isBcuReady(sc.mirrCtrl_id[adsec.n_crates], sc.mirrCtrl_id[adsec.n_crates], timeout=30)
    if err ne adsec_error.ok then return, err
    log_print, "Ok.  Switch Bcu are ready"
    
    cpr = progress[0]+fix(0.3*(progress[1]-progress[0]))
    err = update_progress(cpr)
    if err ne adsec_error.ok then return, err

    err = test_hkpr()
    if err ne adsec_error.ok then return, err

    err = update_msg('Adsec '+!AO_CONST.unit+'-'+ !AO_CONST.telescope_side+' reset done.')
    if err ne adsec_error.ok then return, err
    cpr = progress[1]
    err = update_progress(cpr)
    if err ne adsec_error.ok then return, err

    log_print, 'fsm_reset() procedure done.'

    return, set_fsm_state("PowerOn")

End

