;$Id: apply_mode_vector.pro,v 1.3 2007/02/07 17:21:41 marco Exp $$
;+
;   NAME:
;    APPLY_MODE_VECTOR
;
;   PURPOSE:
;    apply a modes command in the AdSec.
;
;   USAGE:
;    err = apply_mode_vector(modes)
;
;   INPUT:
;    Modes:     mode amplitudes to apply.
;
;   OUTPUT:
;    Error code.
;
;   HISTORY:
;    Created by Armando Riccardi (AR) on Aug 2005
;   07 Feb 2007 MX
;       LUT of variable deleted and name refurbished.
;       NOT IMPLEMENTED THE SWITCH BCU AND CRATE BCU MANAGING.
;-
Function apply_mode_vector, modes

    @adsec_common

    force_threshold = 0.4  ;[N]
    deltapos_threshold = 4e-6 ;[m]

    modes = transpose(modes)    
    deltapos = adsec.ff_p_svec ## transpose(modes)
    if max(abs(deltapos[adsec.true_act])) gt deltapos_threshold then begin
        message, "TOO LARGE STROKE REQUIRED. MODE VECTOR NOT APPLIED", CONT=sc.debug eq 0B
        return, adsec_error.stroke_required ;12345L
    endif
    ff_forces = adsec.ff_matrix ## deltapos
    print, minmax(abs(ff_forces[adsec.true_act]))
    if max(abs(ff_forces)) gt force_threshold then begin
        message, "TOO LARGE FORCES REQUIRED. MODE VECTOR NOT APPLIED", CONT=sc.debug eq 0B
        return, adsec_error.force_required   ;12346L
    endif

    err = write_same_dsp(sc.all, rtr_map.modes_vector, modes, /CHECK)
    if err ne adsec_error.ok then return, err
    err = start_mm()
    if err ne adsec_error.ok then return, err

    timeout = 0.1 ;[s]
    rised_timeout = 0B
    t0 = systime(/SEC)
    repeat begin
        err = read_seq_dsp(sc.all_dsp, rtr_map.start_mm, 1L, nendof_mm)
        if err ne adsec_error.ok then return, err
        if total(nendof_mm eq 0B) then break
        if systime(/SEC)-t0 gt timeout then rised_timeout = 1B
    endrep until rised_timeout
    if rised_timeout then begin
        message, "ERROR: TIMEOUT in mode-to-act convertion. Exited after "+strtrim(timeout*1000,2)+"ms", CONT=sc.debug eq 0B
        return, adsec_error.dsp_fail;12347L
    endif

    err = read_seq_ch(sc.all, dsp_map.ff_command, 1L, ff_command)
    if err ne adsec_error.ok then return, err
    err = write_same_dsp(sc.all, rtr_map.ff_command_vector, $
                         ff_command, /CHECK)    
    if err ne adsec_error.ok then return, err
    err= start_ff()
    if err ne adsec_error.ok then return, err


    timeout = 1.0 ;;0.1 ;[s]
    rised_timeout = 0B
    t0 = systime(/SEC)
    repeat begin
        err = read_seq_dsp(sc.all_dsp, rtr_map.start_ff, 1L, nendof_ff)
        if err ne adsec_error.ok then return, err
        if total(nendof_ff ne 0B) then break
        if systime(/SEC)-t0 gt timeout then rised_timeout = 1B
    endrep until not rised_timeout
    if rised_timeout then begin
        message, "ERROR: TIMEOUT in feed-forward computation. Exited after "+strtrim(timeout*1000,2)+"ms", CONT=sc.debug eq 0B
        return, adsec_error.dsp_fail ; 12347L
    endif

    return, adsec_error.ok
end
