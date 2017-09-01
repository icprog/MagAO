Function fsm_enable_ovs

    @adsec_common
    err = set_diagnostic(OVER=0.02)
    if err ne adsec_error.ok then begin
        ;err2 = adam_disable_coils()
        ;if err2 ne adsec_error.ok then return, err2
        return, err
    endif
    return, adsec_error.ok

end
