Function fsm_savestatus, filename, SAVE_ACCEL = SAVE_ACCEL

    @adsec_common

    status = ptr2value(!AO_STATUS)
    for i=0, n_elements(tag_names(status))-1 do $
        if n_elements(status.(i)) eq 1 then status.(i)=(status.(i))[0]

    mygr = gr
    myadsec = adsec
    myadsec_shell = adsec_shell
    mysc = sc
    ; AP120423
    g_gain = *rtr.g_gain_a


    if keyword_set(SAVE_ACCEL) then begin
        accel = get_accel(DATA=DATA)
        save, status, accel, data, mygr, myadsec, myadsec_shell, mysc, g_gain, FILE=filename
    endif else begin
        save, status, mygr, myadsec, myadsec_shell, mysc, g_gain, FILE=filename
    endelse

    return, adsec_error.ok

End

