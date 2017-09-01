; $Id: update_delta_position.pro,v 1.6 2005/12/07 11:53:49 labot Exp $
;+
; HISTORY:
;   02 Nov 2004, MX
;     Adsec.all changed in sc.all.
;   Feb 2005, MX & DZ
;    Now position_command is delta_position_command+commands.
;-

pro update_delta_position, delta_position_command, FF=feedforward

    @adsec_common

    if test_type(delta_position_command, /FLOAT, N_EL=n_el) then $
        message, "delta position command must be float."
    if n_el ne adsec.n_actuators then $
        message, "delta position command must have "+strtrim(adsec.n_actuators,2)+" elements."

    ;long_delta_position_command = delta_position_command

    err = get_commands(sc.all_actuators, commands)
    if err ne adsec_error.ok then return

    position_command = delta_position_command+commands
    ;if (max(position_command) gt 2L^15-1) or (min(position_command) lt -2L^15) then $
    ;message,"position command goes beyond the acceptable range."

    ;position_command = fix(position_command)

    if keyword_set(feedforward) then begin

        ;err = set_position_vector(position_command,delta_position_command) changed to use set_delta_position_vector
        err = set_delta_position_vector(delta_position_command)
        if err ne adsec_error.ok then return

        err = start_ff()
        if err ne adsec_error.ok then return

    endif else begin
        err = set_position_vector(position_command)
        if err ne adsec_error.ok then return

        err = update_command()
        if err ne adsec_error.ok then return

    endelse

end

