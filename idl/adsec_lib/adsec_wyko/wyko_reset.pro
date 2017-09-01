; $Id: wyko_reset.pro,v 1.4 2003/11/18 14:12:06 riccardi Exp $

function wyko_reset, com_data

	if n_params() ne 1 then begin
		message, "Wrong number of parameters", /CONT
		return, -12
	endif

	if size(com_data, /TYPE) eq 8 then begin
		comm_type = "socket"
		ip_address = com_data.ip_address
		port = com_data.port
	endif else begin
		comm_type = "serial"
		com_port_str = com_data
	endelse


	; reset the serial communication sending a CTRL-C


	if comm_type eq "serial" then begin
		err = com_query(com_port_str, string([3B]), /NO_APPEND_CR, RESP_END_STR="> ", /VERBOSE)
	endif else begin
		err = socket_query(ip_address, port, string([3B]), /NO_APPEND_CR, RESP_END_STR="> ", /VERBOSE)
	endelse

	if err ne 0 then begin
		message, "Error resetting the Wyko communication.", /CONT
		return, -1
	endif

	return, 0
end

