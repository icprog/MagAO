Function set_fsm_state, state

    @adsec_common
    stateu = long(byte(state))
    nel = n_elements(stateu)
    if nel gt 30 then return, adsec_error.generic_error
    if nel lt 30 then stateu = [stateu, lonarr(30-nel)+32]
;    if state ne "PowerOff" then begin
;        err = write_same_dsp(0, '100000'xl, stateu, /SWITCH, /CHECK)
;        if err ne adsec_error.ok then return, err
;    endif
    !AO_STATUS.FSM_STATE = state
    return, update_rtdb()

End
