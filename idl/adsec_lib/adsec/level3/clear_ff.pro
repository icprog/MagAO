; $Id: clear_ff.pro,v 1.11 2007/02/07 17:21:41 marco Exp $
;+
; HISTORY
;   13 July 2004: A. Riccardi
;     Forced write-back check
;   02 Nov 2004, MX
;     Adsec.all changed in sc.all.
;   08 Feb 2005, D.Zanotti(DZ), AR, MX
;     Reset filter keyword added to clean the feed-forward filter.
;     Set to zero all the current.
;   27 June 2005,DZ,AR,MX
;     Delete initialization of  ff_current, because now is read only variable.
;   29 Nov 2005, MX
;     Comments added.
;   07 Feb 2007 MX
;       LUT of variable deleted and name refurbished.
;-
function clear_ff, RESET_FILTER=reset_filter

    @adsec_common

    ;cleaning the mean control current actual contribution on feed forward
    err = write_same_ch(sc.all_actuators, dsp_map.mean_control_current, 0.0, /CHECK)
    if err ne adsec_error.ok then return, err
    
    ;cleaning the integrated control current contribution on feed-forward
    err = write_same_ch(sc.all_actuators, dsp_map.int_control_current, 0.0, /CHECK)
    if err ne adsec_error.ok then return, err

    ;cleaning the integrated ff current
    err = write_same_ch(sc.all_actuators, dsp_map.ff_pure_current, 0.0, /CHECK)
    if err ne adsec_error.ok then return, err

    ;clening the total feed-forward contribution
    err = write_same_ch(sc.all_actuators, dsp_map.ff_ud_current, 0.0, /CHECK)
    if err ne adsec_error.ok then return, err

    ;cleaning the control current filters MATRICES, not delay lines!
    if keyword_set(reset_filter) then begin
        err = set_fc_gain(/BF0, /RES)
        if err ne adsec_error.ok then return, err
        err = set_fc_gain(/BF1, /RES)
        if err ne adsec_error.ok then return, err
        err = set_fc_gain(/AF1, /RES)
        if err ne adsec_error.ok then return, err
    endif

    ;cleanig the ff_command
    err = set_delta_position_vector(fltarr(adsec.n_actuators), /CHECK)
    if err ne adsec_error.ok then return, err

    return, err

end

