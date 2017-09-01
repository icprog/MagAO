; $Id: save_wyko.pro,v 1.5 2005/03/31 14:07:05 labot Exp $

function save_wyko, com_data, filename, TYPE=type

	if n_params() ne 2 then begin
		message, "Wrong number of parameters", /CONT
		return, -12
	endif

	if not test_type(com_data, /STRUCT) then begin
		comm_type = "socket"
		ip_address = com_data.ip_address
		port = com_data.port
	endif else begin
		comm_type = "serial"
		com_port_str = com_data
	endelse

	prompt = "> "

	if n_elements(type) eq 0 then type = "Wyko"

       wait, 0.5
	err = wyko_reset(com_data)
	if err ne 0 then return, err

	; perform a measurament with the interferometer

       wait, 0.5
	if comm_type eq "serial" then begin
	    err = com_query(com_port_str, "new", RESP_END_STR=prompt, /VERB)
	endif else begin
		err = socket_query(ip_address, port, "new", RESP_END_STR=prompt, /VERB)
	endelse

	if err ne 0 then begin
		message, "Error during the communication with the interferometer", /CONT
		return, err
	endif

	if strlowcase(type) eq "durango" then begin
		wait, 4.0 ;; <--- IMPORTANT!!! trim this parameter depending on the amount of
		          ;;                   computations Durango has to do after a new frame
	endif

	case strlowcase(type) of
		"wyko": begin
			cmd_str = 'save '+filename
		end
		"durango": begin
			cmd_str = 'save "'+filename+'"'
		end
		else: begin
			message, "Not supported interferometer type: "+type, /CONT
			return, -13
		endelse
	endcase

       wait, 0.5
	if comm_type eq "serial" then begin
		err = com_query(com_port_str, cmd_str, RESP_END_STR=prompt, /VERB)
    endif else begin
		err = socket_query(ip_address, port, cmd_str, RESP_END_STR=prompt, /VERB)
	endelse

    if err ne 0 then begin
		if err ne 0 then message, "Error during the communication with the interferometer", /CONT
		return, err
	endif


       if strlowcase(type) eq "wyko" then begin
           wait, 0.5
           if comm_type eq "serial" then begin
               err = com_query(com_port_str, "close", RESP_END_STR=prompt, /VERB)
           endif else begin
               err = socket_query(ip_address, port, "close", RESP_END_STR=prompt, /VERB)
           endelse
           if err ne 0 then message, "Error during the communication with the interferometer"
       endif
          
end
