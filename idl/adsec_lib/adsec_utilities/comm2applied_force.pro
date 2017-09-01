function comm2applied_force, commanded_force, position

	m_coil=-0.3/0.5;[(delta_eff/eff_nominal)/mm]

	if (n_elements(position) ne n_elements(commanded_force)) then begin
		message, "position and commanded_force must have the same number of elements"
	endif

	return, commanded_force*(m_coil*1e3*position)
end
