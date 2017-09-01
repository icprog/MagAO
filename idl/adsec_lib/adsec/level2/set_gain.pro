; $Id: set_gain.pro,v 1.10 2007/02/07 16:57:57 marco Exp $

;+
; SET_GAIN
;
; HISTORY
;
;   Written by G. Brusa and A. Riccardi (AR)
;   Osservatorio Asrofisico di Arcetri, ITALY
;   <riccardi@arcetri.astro.it>
;
;   13 Mar 2002, AR
;     The routine handles correctly the "Dummy" communication
;   02 Dec 2003, AR
;     Added lines of code to fix the write on actuator subset problem. The
;     new lines are not active, they are commented. For future use.
;   27 May 2004, AR
;     adsec.max_ngain_step used for gain step check instead of a constant number.
;   05 Aug 2004, AR and M. Xompero
;     Test to gain data type added
;   02 Nov 2004, MX
;     Adsec.all changed in sc.all.
;   Feb 2005
;     Corrected an error of rounding float in the adsec.max_ngain_step. 
;   07 Feb 2007 MX
;       LUT of variable deleted and name refurbished.
;-
function set_gain, act_list, gain, NO_CHECK=no_check, SPEED_LOOP_GAIN=speed_loop_gain

@adsec_common

if test_type(gain, /FLOAT, N_EL=n_el) then begin
    message, "gain must be a float.", CONT=(sc.debug eq 0B)
    return, adsec_error.input_type
endif    
    
if keyword_set(speed_loop_gain) then begin
    addr = dsp_map.speed_loop_gain
endif else begin
    addr = dsp_map.err_loop_gain
endelse

if min(gain) lt 0 then begin
    message,' Gain cannot be negative. New gain not applied', CONT=(sc.debug eq 0)
    return, adsec_error.idl_input_type
endif

if not (keyword_set(no_check) or (sc.host_comm eq "Dummy")) then begin
    err = get_gain(act_list, curr_gain, SPEED_LOOP_GAIN=speed_loop_gain)
    if err ne adsec_error.ok then return, err

    if n_elements(gain) eq 1 then begin
        if max(gain[0]-curr_gain) gt float(adsec.max_ngain_step)*1.001 then begin
            message,' Gain step too high! New gain not applied.', CONT=(sc.debug eq 0)
            return, adsec_error.idl_input_type
        endif
        err = write_same_ch(act_list, addr, gain[0])
;       err=get_gain(sc.all_actuators, gg, SPEED_LOOP_GAIN=speed_loop_gain)
;       if act_list[0] eq sc.all_actuators then gg[*]=gain[0] else gg[act_list]=gain[0]
;       err=write_seq_ch(sc.all_actuators, addr, gg)
    endif else begin
       if n_elements(curr_gain) ne n_elements(gain) then begin
           message,' act_list size and gain size mismatch', CONT=(sc.debug eq 0)
           return, adsec_error.idl_input_type
       endif
       if max(gain-curr_gain) gt 100 then begin
           message,' Gain step too high! New gain not applied.', CONT=(sc.debug eq 0)
           return, adsec_error.idl_input_type
       endif
       err = write_seq_ch(act_list, addr, gain)
;       err=get_gain(sc.all_actuators, gg, SPEED_LOOP_GAIN=speed_loop_gain)
;       if act_list[0] eq sc.all_actuators then gg[*]=gain else gg[act_list]=gain
;       err=write_seq_ch(sc.all_actuators, addr, gg)
    endelse
endif else begin
    if n_elements(gain) eq 1 then begin
        err = write_same_ch(act_list, addr, gain[0])
;       err=get_gain(sc.all_actuators, gg, SPEED_LOOP_GAIN=speed_loop_gain)
;       if act_list[0] eq sc.all_actuators then gg[*]=gain[0] else gg[act_list]=gain[0]
;       err=write_seq_ch(sc.all_actuators, addr, gg)
    endif else begin
        err = write_seq_ch(act_list, addr, gain)
;       err=get_gain(sc.all_actuators, gg, SPEED_LOOP_GAIN=speed_loop_gain)
;       if act_list[0] eq sc.all_actuators then gg[*]=gain else gg[act_list]=gain
;       err=write_seq_ch(sc.all_actuators, addr, gg)
    endelse
endelse
return, err

end
