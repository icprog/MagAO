function true_bias_force, position, act_list
	@adsec_common
	m_bias=-3.16/9.7;[(delta_f/f_nominal)/mm]
	if n_elements(act_list) eq 0 then begin
		if n_elements(position) ne adsec.n_actuators then begin
			message, "position must have adsec.n_actuators length"
		endif
		act_list = indgen(adsec.n_actuators)
	endif else begin
		if n_elements(act_list) ne n_elements(position) then begin
			message, "position and act_list must have the same number of elements"
		endif
	endelse
	return, (adsec.curr4bias_mag[act_list]*adsec.weight_curr)*(m_bias*1e3*position)
end
