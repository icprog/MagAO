Function get_fsm_state, state

    @adsec_common
    err = read_seq_dsp(0, '100000'xl, 30L,stateb, /SWITCH, /ULONG)
    if err ne adsec_error.ok then return, err
    state = strtrim(string(byte(stateb)),2)
    !AO_STATUS.FSM_STATE = state
    return, update_rtdb()

End
