;$Id: clear_commands.pro,v 1.1 2007/11/05 16:14:39 marco Exp $$
;+
;   NAME:
;    CLEAR_COMMANDS
;   
;   PURPOSE:
;    Clear all position commands to the Adaptive Secondary.
;
;   USAGE:
;    err = clear_commands(act_list)
;
;   INPUT:
;    act_list: actuators list
;   
;   OUTPUT:
;    err:   error code
;
;   KEYWORDS:
;    None.
;
;   HISTORY
;    Created on 01 Nov 2007, Marco Xompero
;    marco@arcetr.astro.it
;-


function clear_commands, act_list
;+
; stop the control and set to zero all the commands to the specified actuators
;

    @adsec_common
    if n_elements(act_list) eq 0 then act_list = sc.all_actuators
    
;    err = stop_control(act_list)
;    if err ne adsec_error.ok then return, err
    
    if act_list[0] eq sc.all_actuators then begin
        curr = replicate(0.0, adsec.n_actuators)
    endif else begin
        curr = replicate(0.0, n_elements(act_list))
    endelse
    
    err = set_offload_cmd(/RESET, /NOCHECK)
    if err ne adsec_error.ok then return, err
    
    err = write_bias_cmd(fltarr(adsec.n_actuators), /CHECK)
    if err ne adsec_error.ok then return, err
    
    err = write_same_ch(act_list, dsp_map.fullbias_cmd, 0.0, /CHECK)
    if err ne adsec_error.ok then return, err

    err = write_des_pos(sc.all, fltarr(adsec.n_actuators))
    if err ne adsec_error.ok then return, err
    
    return, err
    
end

