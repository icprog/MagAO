; $Id: apply_mode.pro,v 1.4 2004/05/19 15:49:37 labot Exp $

function apply_mode, mode, amp, XADSEC=xadsec, FF=feedforward

@adsec_common


if test_type(mode, /INT, DIM=dim) then return, adsec_error.input_type
if total(dim ne [0]) ne 0 then return, adsec_error.input_type

max_abs_curr = 10000

delta_position_command_float = amp*adsec.ff_p_svec[mode,*]
delta_force = adsec.ff_matrix ## delta_position_command_float
err = update_status(1024)
if err ne adsec_error.ok then return,err

if max(abs(sys_status.current+delta_force)) gt max_abs_curr then begin
	message,'Maximum allowed current exceeded, delta command not applied. Returning...', CONT = (sc.debug eq 0B)
    return, adsec_error.input_type
endif
delta_position_command = delta_position_command_float
update_delta_position,delta_position_command, FF=feedforward
;if err ne adsec_error.ok then return,err

wait, 0.1

if keyword_set(xadsec) then update_panels, /OPT

return, adsec_error.ok
end
