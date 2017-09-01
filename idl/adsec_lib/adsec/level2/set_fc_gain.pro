;+
;   NAME:
;    SET_FC_GAIN
;
;   PURPOSE:;    Write BF0, BF1, AF1 into the DSP memory by splittng the matrix in adsec.n_dsp part of size
;    adsec.n_act_per_dsp, or reset all the gain of int_ctrl_current.
;
;   USAGE:
;    err = set_fc_gain(gain_diag,/bf0,/bf1,/af1, /block, /reset_all)
;
;   INPUT:
;
;
;   OUTPUT:
;    Error code.
;
;   KEYWORD:
;    BLOCK = if set gain diagonal is written on block B.
;    RESET_ALL = if set reset chosen gain to zero.
;    BF0, AF1, BF1 = gains to reset/modify
;    ALL_RESET = shortcut for resetting all gains.
;
;   NOTE:
;    None.
;
; HISTORY
;   8 Feb 2005 Daniela Zanotti(DZ)
;   20 May 2005 Marco Xompero
;       Dsp_map updated to rtr_map. Help updated.
;   07 Feb 2007 MX
;       LUT of variable deleted and name refurbished.
;       TODO: the matrices have to be inserted in a IDL structure
;-

function set_fc_gain, fc_gain,BF1=bf1, BF0=bf0, AF1=af1, RESET_ALL=reset_all, $
     BLOCK = block, ALL_RESET=all_reset

    @adsec_common

    if keyword_set(all_reset) then begin
        bf0 = 1
        bf1 = 1
        af1 = 1
        reset_all = 1
    endif

    if keyword_set(reset_all) then fc_gain=fltarr(adsec.n_actuators)


    if test_type(fc_gain, /real, dim=dim) then $
           message, "The gain matrix must be real"

    if dim[0] ne 1 then $
         message, "The gain matrix must be a diagonal matrix" else $
               if dim[1] ne adsec.n_actuators then $
                        message, "The gain must be of " $
                        +strtrim(adsec.n_actuators,2)+" elements."

    gain_diag=reform(float(fc_gain), adsec.n_act_per_dsp,adsec.n_dsp)

 
    if keyword_set(bf0) then begin
        if keyword_set(reset_all) then begin
             err = write_seq_dsp(sc.all, dsp_map.gain_bf0_a, gain_diag , /CHECK)
             err = write_seq_dsp(sc.all, dsp_map.gain_bf0_b, gain_diag , /CHECK)
             if err ne adsec_error.ok then return, err
        endif else begin if not(keyword_set(block)) then $
                            err = write_seq_dsp(sc.all, dsp_map.gain_bf0_a, gain_diag , /CHECK) else $
                            err = write_seq_dsp(sc.all, dsp_map.gain_bf0_b, gain_diag, /CHECK)
                            if err ne adsec_error.ok then return, err
        endelse
    endif

    if keyword_set(bf1) then begin
        if keyword_set(reset_all) then begin
            err = write_seq_dsp(sc.all, dsp_map.gain_bf1_a, gain_diag , /CHECK)
            err = write_seq_dsp(sc.all, dsp_map.gain_bf1_b, gain_diag , /CHECK)
            if err ne adsec_error.ok then return, err
        endif else begin if not(keyword_set(block)) then $
                            err = write_seq_dsp(sc.all, dsp_map.gain_bf1_a, gain_diag , /CHECK) else $
                            err = write_seq_dsp(sc.all, dsp_map.gain_bf1_b, gain_diag, /CHECK)
                        if err ne adsec_error.ok then return, err
        endelse
    endif

    if keyword_set(af1) then begin
        if keyword_set(reset_all) then begin
            err = write_seq_dsp(sc.all, dsp_map.gain_af1_a, gain_diag , /CHECK)
            err = write_seq_dsp(sc.all, dsp_map.gain_af1_b, gain_diag , /CHECK)
            if err ne adsec_error.ok then return, err
            endif else begin if not(keyword_set(block)) then $
                            err = write_seq_dsp(sc.all, dsp_map.gain_af1_a, gain_diag , /CHECK) else $
                            err = write_seq_dsp(sc.all, dsp_map.gain_af1_b, gain_diag, /CHECK)
                          if err ne adsec_error.ok then return, err
        endelse
    endif

    return,adsec_error.ok

end
