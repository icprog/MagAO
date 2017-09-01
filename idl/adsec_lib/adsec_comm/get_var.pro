; $Id: get_var.pro,v 1.2 2005/08/03 17:07:11 labot Exp $$
;
;+
;
; GET_VAR
;
;  PURPOSE:
;   This function permits the access on read to the MSG-RTDB variables.
;
;  USAGE:
;   err = get_var(var_name, data_len, receive_data_buffer, /LONG, /ULONG, TYPE=type)
;
;  INPUTS:
;
;   name:        string. Name of the MsgRTDB variable to get.
;                The name is CASE SENSITIVE.
;
;   data_len:    idl long, short or unsigned short int type scalar.
;                Number of words to read.
;
;
;  OUTPUTS
;
;   receive_data_buffer: Named variable. Idl float/ulong/long type array output.
;                       If TYPE, ULONG, LONG keywords are set the corresonding
;                       data type is output. By default the output is floating
;                       point values.
;
;   err:             long-int scalar. Error code.
;
; KEYWORDS
;
;    ULONG:      If set the output is translated to unsigned long (same bit sequence)
;    LONG:      If set the output is translated to signed long (same bit sequence)
;    TYPE:       int scalar. If passed the output is translated to the corresponding
;                data type. Allowed values: 3(long), 4(float) and 13(ulong). TYPE setting
;                overrides the ULONG and LONG keyword setting. If no ULONG, LONG or TYPE keyword
;                are set, default data type is float.
;
; HISTORY:
;
;   Created by Marco Xompero (MX)
;   Osservatorio Astrofisico di Arcetri, ITALY
;   <marco@arcetri.astro.it>
;   03 Aug 2005 Marco Xompero
;    CALL_EXTERNAL call fixed.
;-
;

Function get_var, name, data_len_tmp, receive_data_buffer $
         , ULONG=set_ulong, LONG=set_long, TYPE=data_type

@adsec_common

;=================================================
; checks for the input parameters
;=================================================

;std definitions for testing
data_len = data_len_tmp

if n_elements(data_type) eq 0 then begin
    if keyword_set(set_long)+keyword_set(set_ulong) gt 1 then begin
        message, "ULONG and LONG keyword are exclusive", CONT=(sc.debug eq 0B)
        return, adsec_error.input_type
    endif
    case 1B of
        keyword_set(set_ulong): data_type=13
        keyword_set(set_long): data_type=3
        else: data_type=4
    endcase
endif
case data_type of
   13: receive_data_buffer = ulonarr(data_len) ;ulong
   3: receive_data_buffer = lonarr(data_len) ;long
   4: receive_data_buffer = fltarr(data_len) ;float
   else: begin
     message, "Requested data type not valid (TYPE="+strtrim(data_type,2)+").", $
              CONT=(sc.debug eq 0B)
     return, adsec_error.input_type
       end
endcase

str="short int (IDL int)"

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

;test type of "data_len" variable
if test_type(data_len, /LONG, /INT, /UINT, N_EL=n_el) then begin
    message, "data_len must be a short or long-int", CONT=(sc.debug eq 0B)
    return, adsec_error.input_type
endif
;check n elements (must be 1)
if n_el ne 1 then begin
    message, "data_len must be a scalar", CONT=(sc.debug eq 0B)
    return, adsec_error.input_type
endif
data_len = long(data_len[0])
;end of the checks


;=============================================================
;Begin of the real routine
;=============================================================

;calculating the number of dsp and board to read

timeout = long(data_len * 1000d * dsp_const.mean_transfer_time * 1.2) > sc.min_timeout

if sc.ll_debug then begin
    print, 'Debugging data'
    print, 'Tiemot ', strtrim(string(timeout),2)
    print, 'data len ' ,strtrim(string(data_len,format='(Z8.8)'),2)
    print, 'Buffers Dimensions ', size(receive_data_buffer, /dim)
endif


;Dummy case
if sc.host_comm eq "Dummy" then begin
   common dummy_comm_block, dummy_comm_seed
   receive_data_buffer[*] = 10*randomu(dummy_comm_seed, size(receive_data_buffer,/DIM))
    err = adsec_error.ok
endif else begin

;Natural case and call external to .so
    err = CALL_EXTERNAL(sc.dll_name, 'getvar',               $
                        name, receive_data_buffer, data_len, timeout)
endelse

    if err ne adsec_error.ok then begin
        message, "READING ERROR BCU", CONT=(sc.debug eq 0B)
        return, err
    endif

return, err

end
