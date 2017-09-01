Function get_diag_frames, n_samples2restore_in, data, SWITCHB=is_switch, DSP=is_dsp, ACC=is_acc

    @adsec_common
    if n_elements(n_samples2restore_in[0]) lt 0 then begin
        print, "Invalid number of frames requested"
        return, adsec_error.input_type
    endif
    n_samples2restore = n_samples2restore_in[0]
    old_ovs=rtr.oversampling_time
    
    err = set_diagnostic(over=0.0)
    if err ne adsec_error.ok then return, err
    err = dspstorage(/DISABLE)
    if err ne adsec_error.ok then return, err

    if keyword_set(is_acc) then begin
        accsamples = round(n_samples2restore *4000*old_ovs)
        err=load_diag_data_acc(n_samples2restore, dacc,/readfc)
    endif


    if keyword_set(is_switch) then begin

        err = load_diag_data_switch(n_samples2restore+1, drw, /NOFASTCHECK, /READFC, XADSEC=xadsec)
        if err ne adsec_error.ok then return, err

    endif

    err = set_diagnostic(over=old_ovs)
    if err ne adsec_error.ok then return, err

    if keyword_set(is_dsp) then begin
        err = load_diag_data_dsp(n_samples2restore+1, dr, /NOFASTCHECK, /READFC, XADSEC=xadsec)
        if err ne adsec_error.ok then return, err
    endif

    err = set_diagnostic(over=0.0)
    if err ne adsec_error.ok then return, err

    err = dspstorage()
    if err ne adsec_error.ok then return, err

    err = set_diagnostic(over=old_ovs)
    if err ne adsec_error.ok then return, err


    if n_elements(dacc) eq 0 then dacc = ""
    if n_elements(drw) eq 0 then  drw = ""
    if n_elements(dr) eq 0 then   dr=""
    
    data = {       $
        dr: dr,    $
        drw: drw,  $
        dacc: dacc $
    }
 
    return, adsec_error.ok

End
