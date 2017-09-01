; $Id: clear_dacs.pro,v 1.6 2007/11/05 17:22:42 marco Exp $

function clear_dacs, act_list
;+
;   NAME:
;    CLEAR_DACS
;
;   PURPOSE:
;    Stop the control and set to zero the current to the specified actuators
;
;   USAGE:
;    err=clear_dacs([act_list])
;
; HISTORY
;   13 July 2004: A. Riccardi
;     currents are passed as floating point values to match LBT specs
;   02 Nov 2004, MX
;     Adsec.all changed in sc.all.
;   04 Nov 2007, MX
;     Added force offload cleaning
;-

    @adsec_common
    if n_elements(act_list) eq 0 then act_list = sc.all_actuators
    
    err = stop_control(act_list)
    if err ne adsec_error.ok then return, err
    
    if act_list[0] eq sc.all_actuators then begin
        curr = replicate(0.0, adsec.n_actuators)
    endif else begin
        curr = replicate(0.0, n_elements(act_list))
    endelse
    
    err = set_offload_force(/RESET, /NOCHECK)
    if err ne adsec_error.ok then return, err
    
    err = write_bias_curr(act_list, curr)
    if err ne adsec_error.ok then return, err
    
    err = write_same_ch(act_list, dsp_map.fullbias_curr, 0.0, /check)
    if err ne adsec_error.ok then return, err
    
    if act_list[0] eq sc.all_actuators or n_elements(act_list) eq adsec.n_actuators then begin
        err = clear_ff()
        if err ne adsec_error.ok then return, err
    endif
    
    return, err
    
end

