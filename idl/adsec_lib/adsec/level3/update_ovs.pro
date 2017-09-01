Function update_ovs, period, DECIMATION_MASTER=decm

    @adsec_common
    if n_elements(decm) gt 0 then decm = fix(decm)
    freq = 1./float(period)

    case 1 of
    
        freq gt 0   && freq le 50:   begin
                                    ft  = 0.5
                                    gdc = 10.
                                end
        freq gt 50  && freq le 100:  begin
                                    ft  = 1.0
                                    gdc = 10.
                                end
        freq gt 100 && freq le 200:  begin
                                    ft  = 2.0
                                    gdc = 10.
                                end
        freq gt 200 && freq le 300:  begin
                                    ft  = 5.0
                                    gdc = 10.
                                end
        freq gt 300 && freq le 2000: begin
                                    ft  = 5.0
                                    gdc = 10.
                                end
        else:   begin
                    log_message, "Wrong input parameters.", CONT = (sc.debug eq 0), ERR=adsec_error.input_type
                    return, adsec_error.input_type
                end
                
    endcase

; period = 1./freq

;    if period gt rtr.oversampling_time then begin
        
        err = set_icc(/HOLD)
        if err ne adsec_error.ok then return, err
        err = set_diagnostic(OVER=period, DECIMATION_MASTER=decm, /BCU, /SWITC)
        if err ne adsec_error.ok then return, err
        err = set_icc(ft, gdc)
        if err ne adsec_error.ok then return, err

;    endif else begin

;        err = set_diagnostic(OVER=period)
;        if err ne adsec_error.ok then return, err
;        err = set_icc(ft, gdc)
;        if err ne adsec_error.ok then return, err

;    endelse


    return, adsec_error.ok
    
 End

