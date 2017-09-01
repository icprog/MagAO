; $Id: set_position_vector.pro,v 1.9 2007/02/07 16:57:57 marco Exp $
;+
; set_position_vector
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
;       Removed repeated read-back test used for MMT.
;       Keyword CHECK added.
;   02 Nov 2004, MX
;       Adsec.all changed in sc.all.
;   08 Feb 2005, MX
;       'set_delta_position_vector' function used instead of direct writing.
;       Commented lines unused cleared.
;   20 May 2005: Marco Xompero (MX)
;       Dsp_map updated to rtr_map.
;   07 Feb 2007 MX
;       LUT of variable deleted and name refurbished.
;-
function set_position_vector, command_vector, ff_command_vector, CHECK=check

@adsec_common

if n_elements(command_vector) ne adsec.n_actuators then begin
    message, "the position vector must have "+strtrim(adsec.n_actuators,2)+" elements.", $
      CONT = (sc.debug eq 0B)
    return, adsec_error.input_type
 endif
n_dpv = n_elements(ff_command_vector)
if n_dpv eq 0 then begin
    set_delta_pos = 0B
endif else begin
    set_delta_pos = 1B
    if n_dpv  ne adsec.n_actuators then begin
        message, "the position vector must have "+strtrim(adsec.n_actuators,2)+" elements.", $
             CONT = (sc.debug eq 0B)
        return, adsec_error.input_type
    endif
endelse

if set_delta_pos then begin

    err = set_delta_position_vector(ff_command_vector, CHECK=check)
    if err ne adsec_error.ok then return, err

endif

    err = write_same_dsp(sc.all_dsp, rtr_map.command_vector, command_vector, CHECK=check)
    if err ne adsec_error.ok then return, err

return, err

end
