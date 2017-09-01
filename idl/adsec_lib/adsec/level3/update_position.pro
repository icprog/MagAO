; $Id: update_position.pro,v 1.4 2004/11/02 16:14:25 marco Exp $
;+
;  HISTORY
;   02 Nov 2004, MX
;     Adsec.all changed in sc.all.
;-

pro update_position, position_command, FF=feedforward

@adsec_common 

if test_type(position_command, /INT, N_EL=n_el) then $
	message, "position command must be integer."
if n_el ne adsec.n_actuators then $
	message, "position command must have "+strtrim(adsec.n_actuators,2)+" elements."

long_position_command = long(position_command)

if keyword_set(feedforward) then begin
	err = get_commands(sc.all_actuators, commands)
	if err ne adsec_error.ok then return

	delta_position_command = long_position_command-commands
	if (max(delta_position_command) gt 2L^15-1) or (min(delta_position_command) lt -2L^15) then $
		message,"delta position command goes beyond the acceptable range."

	delta_position_command = fix(delta_position_command)
	err = set_position_vector(position_command, delta_position_command)
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

