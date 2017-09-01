;$Id: int_ctrl_curr_coeffs.pro,v 1.3 2008/02/21 09:55:27 labot Exp $
;+
; HISTORY
;   2007 written by Marco Xompero(MX)
;   13 Feb 2008 modified by MX, A.Riccardi(AR), D.Zanotti(DZ)
;               added check on the cut-off frequency.
;-
Function int_ctrl_curr_coeffs, cut_freq, DC_GAIN=dc_gain, SAMPLING_TIME=sampling_time, BFO = bf0, BF1 = bf1, AF1 = af1

    if n_elements(sampling_time) eq 0 then F = 1/1e-3 $
    else if sampling_time[0] eq 0 then F = 1/1e-3 else F=1./sampling_time
    if n_elements(dc_gain) eq 0 then dc_gain=1.0 ;else dc_gain = dc_gain < 0.9

    if cut_freq gt F/4 then begin
        message, "WRONG INPUT PARAMETERS! Cut-off frequency too high wrt sampling freq.", /INFO
        bf0=0.0
        bf1=0.0
        af1=0.0
        return, adsec_error.ok
    endif
    num_c = [cut_freq*2*!dpi, 0]
    den_c = [cut_freq*2*!dpi, 1]
    tustin, num_c, den_c, F, num_d, den_d, /DOUBLE

    ;dc_gain ADJUSTING
    num_d *= dc_gain
    bf0 = float(1/den_d[0]*num_d[0])
    bf1 = float(1/den_d[0]*num_d[1])
    af1 = -float(1/den_d[0]*den_d[1])

    if af1 lt 0.0 then begin
        message, "WRONG INPUT PARAMETERS! Unstable filter", /INFO
        bf0=0.0
        bf1=0.0
        af1=0.0
        return, adsec_error.ok
    endif

    print, "Cut-off frequency [Hz]:", cut_freq
    print, "DC_gain [-]:", dc_gain
    print, "Sampling freq [Hz]:", F
    print, "Num H(z):",num_d
    print, "Den H(z):",den_d
    print, "bf0,bf1,af1:",bf0,bf1,af1

    return, 0

End
