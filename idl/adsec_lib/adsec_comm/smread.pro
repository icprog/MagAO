; $Id: smread.pro,v 1.2 2005/08/03 17:12:47 labot Exp $
;
;+
;
; SMREAD
;
; PURPOSE:
;  This function permits to read data from PC Shared Memory
;
;  err = SMREAD(name_block,sized, type, timeout,data)
;
; INPUTS:
;  name_block:   idl string type. Shared memory block name to act as consumer.
;  data:         data buffer returned, read from SM name_block. default is float.
;  timeout:      idl int. Set the timeout in [ms] to the sm reading operation. Deafault is 1 [ms].
;
; OUTPUTS:
;  err:             long-int scalar. (see adsec_error in init_gvar.pro)
;
; KEYWORDS:
;
; HISTORY:
;
;   31 Mar 2005, written by Marco Xompero
;       <marco@arcetri.astro.it>
;   03 Aug 2005, MX
;    Timeout remanaged. Fixed bug on error handling.
;-

Function smread, name_block, sized, data, timeout=timeout, ulong=ulong, type=type

@adsec_common


;=================================================
; checks for the input parameters
;=================================================

;std definitions for testing
;test type of "first_dsp" variable
;check int type
if test_type(name_block, /STRING, N_EL=n_el) then begin
    message, "The name must be an IDL string!", CONT=(sc.debug eq 0B)
    return, adsec_error.input_type
endif

if n_elements(timeout) ne 0 then begin
  if test_type(timeout, /LONG, /INT, N_EL=n_el) then begin
       message, "The timeout must be an IDL int!", CONT=(sc.debug eq 0B)
       return, adsec_error.input_type
   endif
endif else timeout=100l
   
if keyword_set(ulong) then data = ulonarr(sized) else data = fltarr(sized)
if not keyword_set(type) then type = 1
if sc.busy_comm then begin
;if sc.host_comm eq "Dummy" then begin
;    err = adsec_error.ok
;    data = randomn(sized)
;endif else begin

;Natural case and call external to .so
     err = CALL_EXTERNAL(sc.dll_name, 'smreadbuf', $
                    name_block, type, timeout, data)
;endelse

 if (err lt 0) then begin
     message, "READING FROM SHARED MEMORY ERROR.", CONT=(sc.debug eq 0B)
     return, (err < 0)
 endif
endif else begin
    print, 'Communication not opened.'
    err = adsec_error.communication
endelse

return, err

end
