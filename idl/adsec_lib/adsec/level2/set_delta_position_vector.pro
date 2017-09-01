; $Id: set_delta_position_vector.pro,v 1.9 2007/02/07 16:57:57 marco Exp $

;+
; set_delta_position_vector
;
; HISTORY
;
;   Written by A. Riccardi (AR)
;   Osservatorio Asrofisico di Arcetri, ITALY
;   <riccardi@arcetri.astro.it>
;
;   13 Mar 2002, AR
;       Bug fixed in the CONT keyword setting of the MESSAGE call.
;   27 May 2004, AR
;       added CHECK keyword
;   02 Nov 2004, MX
;       Adsec.all changed in sc.all.
;   18 Gen 2005, MX and Daniela Zanotti(DZ)
;       New_delta_command writing for the v5.00 logic version compatibility.
;   08 Feb 2005, AR, MX & DZ
;       'Check' keyword fixed.
;   20 May 2005, MX
;       Dsp_map updated to rtr_map.
;   07 Feb 2007 MX
;       LUT of variable deleted and name refurbished.
;-
function set_delta_position_vector, ff_command_vector, CHECK=check

@adsec_common

if n_elements(ff_command_vector) ne adsec.n_actuators then begin
    message, "the delta position vector must have "+strtrim(adsec.n_actuators,2)+" elements.", $
      CONT = (sc.debug eq 0B)
    return, adsec_error.input_type
endif

err = write_same_dsp(sc.all, rtr_map.ff_command_vector, $
                     ff_command_vector+adsec.offset_delta_pos, CHECK=check)
if err ne adsec_error.ok then return, err

err = read_seq_ch(sc.all, dsp_map.pos_command, 1L, pos_command)
if err ne adsec_error.ok then return, err

err = read_seq_ch(sc.all, dsp_map.bias_command, 1L, bias_ch0)
if err ne adsec_error.ok then return, err

new_delta_command = reform(ff_command_vector) - bias_ch0 + pos_command

err = write_seq_ch(sc.all, dsp_map.new_delta_command, new_delta_command,CHECK=check)
if err ne adsec_error.ok then return, err

return, err

end
