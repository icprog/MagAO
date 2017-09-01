; function get_accel, np
; call:  spectra=get_accel(np)
;
;   np: number of points to be sampled. please note that as the accelerometers are sampled at 4KHz, you will need thousands of pts (20000) to observe nicely 'low' frequencies, i.e. 5Hz, 13Hz as detected at the telescope
;   DATA: get accelerometer raw data
;
;horizontal accelerometers :  0,3,5
;vertical accelerometers   :  2,4,6
;accelerometers #1,7 are not connected. they can be identified, however, observing their data plot (that contains just noise)

function get_accel, np, DATA = DATA
    ;horizontal accelerometers:0,3,5
    if n_elements(np) eq 0 then np=20000 
    err=load_diag_data_acc(np,data,/readfc)
    dt= 1/4000.
    if err eq 0 then begin
        h=data.acc_0+data.acc_3+data.acc_5
        v=data.acc_2+data.acc_4+data.acc_6
        fft1, h, dt, fspec=freqh, psd=psdh, /noplo
        fft1, v, dt, fspec=freqv, psd=psdv, /noplo
        acc=[[freqh], [psdh], [psdv]]

        save, freqh, psdh, psdv,data, filename=meas_path('accelerometer', 'acc.sav')

        return, acc
    endif else begin
        log_print, 'Accelerometer acquisition failed'
        return, -1
    endelse

end


