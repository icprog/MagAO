; $Id: write_bias_curr.pro,v 1.9 2007/02/20 15:29:15 labot Exp $

;+
; WRITE_BIAS_CURR
;
; HISTORY
;
;   Written by A. Riccardi (AR)
;   Osservatorio Asrofisico di Arcetri, ITALY
;   <riccardi@arcetri.astro.it>
;
;   13 Mar 2002, AR
;     Bug fixed in the CONT keyword setting of the MESSAGE call.
;   02 Dec 2003, AR
;     Added lines of code to fix the write on actuator subset problem. The
;     new lines are not active, they are commented. For future use.
;   27 Jul 2004, MX
;     CHECK keyword added.
;   02 Nov 2004, MX
;     Adsec.all changed in sc.all.
;   07 Feb 2007 MX
;       LUT of variable deleted and name refurbished.
;       Polling on nendof_ff removed.
;-
;
;TODO:::: SOTTRARRE/SOMMARE FMOL
function write_bias_curr, act_list, curr_vector, SAME_VALUE=same_value, CHECK=check

@adsec_common

; same value keyword is maintained for back compatibility

if keyword_set(same_value) or n_elements(curr_vector) eq 1 then begin
    if n_elements(curr_vector) ne 1 then begin
        message, "The input current must be a scalar", CONT= (sc.debug eq 0B)
        return, adsec_error.input_type
    endif
    err = write_same_ch(act_list, dsp_map.bias_current, curr_vector[0], CHECK=check)
    if err ne adsec_error.ok then return, err

;    err = write_same_ch(act_list, dsp_map.cmd_current, curr_vector[0], CHECK=check)
;    if err ne adsec_error.ok then return, err
;    err = read_seq_ch(sc.all_actuators, dsp_map.bias_current, 1, cc)
;    if act_list[0] eq sc.all_actuators then cc[*]=curr_vector[0] else cc[act_list]=curr_vector[0]
;    err = write_seq_ch(sc.all_actuators, dsp_map.bias_current, cc)

endif else begin
    err = write_seq_ch(act_list, dsp_map.bias_current, curr_vector, CHECK=check)
    if err ne adsec_error.ok then return, err
;    err = write_seq_ch(act_list, dsp_map.cmd_current, curr_vector, CHECK=check)
;    if err ne adsec_error.ok then return, err
;    err = read_seq_ch(sc.all_actuators, dsp_map.bias_current, 1, cc)
;    if act_list[0] eq sc.all_actuators then cc[*]=curr_vector else cc[act_list]=curr_vector
;    err = write_seq_ch(sc.all_actuators, dsp_map.bias_current, cc)
endelse

return, adsec_error.ok

end
