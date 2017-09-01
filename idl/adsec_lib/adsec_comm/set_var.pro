; $Id: set_var.pro,v 1.3 2005/11/29 18:28:15 labot Exp $$
;
;+
;
; SET_VAR
;
;  PURPOSE:
;   This function permits the access on write to the MSG-RTDB variables.
;
;  USAGE:
;   err = set_var(var_name, send_data_buffer)
;
;  INPUTS:
;
;   name:        string. Name of the MsgRTDB variable to get.
;                The name is CASE SENSITIVE.
;
;   send_data_buffer: Named variable. Idl float/ulong/long type array input.
;                     If TYPE, ULONG, LONG keywords are set the corresponding
;                     data type is output. By default the output is floating 
;                     point values.
;
;  OUTPUT:
;
;   err:             long-int scalar. Error code.
;
; KEYWORDS
;   
;   None.
;
; HISTORY:
;
;  Created by Marco Xompero (MX) on 10 May 2005
;  Osservatorio Astrofisico di Arcetri, ITALY
;  <marco@arcetri.astro.it>
;
;   03 Aug 2005
;    Main debug ended. Major bug removed.
;-
;

Function set_var, name, send_data_buffer 

@adsec_common

;=================================================
; checks for the input parameters
;=================================================

;test type of "first_dsp" variable
;check int type
if test_type(name, /STRING, N_EL=n_el) then begin
    message, "name must be a string!", CONT=(sc.debug eq 0B)
    return, adsec_error.input_type
endif

;check n elements (must be 1)
if n_el eq 0 then begin
    message, "variable name is empty.", CONT=(sc.debug eq 0B)
    return, adsec_error.input_type
endif

;get the MsgRTDB variable types from shared library)
err = get_vartypes(msg_types)
if err ne adsec_error.ok then return, err

;test types and set the correstponding msg_types
if test_type(send_data_buffer, /LONG, /INT, /STRING, /BYTE, /ULONG, /UL64, /FLOAT, N_EL=n_el) then begin
    message, "send_data_buffer has an unpermitted data type!", CONT=(sc.debug eq 0B)
    return, adsec_error.input_type
endif

if ~ test_type(send_data_buffer, /LONG, /INT,  N_EL=n_el) then begin
    if n_el eq 1 then send_data_buffer = send_data_buffer[0]
    data_len = long(n_el)
    var_type = msg_types[0]
endif

if ~ test_type(send_data_buffer, /FLOAT, N_EL=n_el) then begin
    if n_el eq 1 then send_data_buffer = send_data_buffer[0]
    data_len = long(n_el)
    var_type = msg_types[1]
endif

if ~ test_type(send_data_buffer, /STRING, N_EL=n_el) then begin
    data_len = long(strlen(send_data_buffer))
    var_type = msg_types[2]
endif

if ~ test_type(send_data_buffer, /BYTE, N_EL=n_el) then begin
    if n_el eq 1 then send_data_buffer = send_data_buffer[0]
    data_len = long(n_el)
    var_type = msg_types[3]
endif

if ~ test_type(send_data_buffer, /UINT, N_EL=n_el) then begin
    if n_el eq 1 then send_data_buffer = send_data_buffer[0]
    data_len = long(n_el)
    var_type = msg_types[4]
endif

if ~ test_type(send_data_buffer, /ULONG, N_EL=n_el) then begin
    if n_el eq 1 then send_data_buffer = send_data_buffer[0]
    data_len = long(strlen(send_data_buffer))
    var_type = msg_types[5]
endif

if ~ test_type(send_data_buffer, /UL64, N_EL=n_el) then begin
    if n_el eq 1 then send_data_buffer = send_data_buffer[0]
    data_len = long(strlen(send_data_buffer))
    var_type = msg_types[6]
endif

;end of the checks


;=============================================================
;Begin of the real routine
;=============================================================

;calculating the number of dsp and board to read

if sc.ll_debug then begin
    print, 'Debugging data'
    print, 'data len ' ,strtrim(string(data_len,format='(Z8.8)'),2)
    print, 'Buffers Dimensions ', size(receive_data_buffer, /dim)
endif

;Dummy case
if sc.host_comm eq "Dummy" then begin
    common dummy_comm_block, dummy_comm_seed
    err = adsec_error.ok
endif else begin

;Natural case and call external to .so
    err = CALL_EXTERNAL(sc.dll_name, 'setvar',               $
                        name, send_data_buffer, var_type, data_len)
    
    if err ne adsec_error.ok then begin
        message, "READING ERROR BCU", CONT=(sc.debug eq 0B)
        return, err
    endif

;    err = get_var(name, data_len, receive_buffer, /LONG)
    
endelse

    if err ne adsec_error.ok then begin
        message, "READING ERROR BCU", CONT=(sc.debug eq 0B)
        return, err
    endif
    
return, err

end
