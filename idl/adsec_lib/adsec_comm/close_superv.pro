; $Id: close_superv.pro,v 1.3 2007/04/20 13:01:45 labot Exp $

;+
;
; CLOSE_SUPERV
;
; This function close the communication with Supervisor currentely open.
;
; err = close_superv(num_device, receive_channel, WPHL_RESULT=wphl_result)
;
;
; OUTPUTS
;
;   err:             long-int scalar. (see adsec_error in init_gvar.pro)
;
; KEYWORDS
;
;
; HISTORY:
;
;       Wen Mar 17 2004, Marco Xompero
;       <marco@arcetri.astro.it>
;
;-

function close_superv

@adsec_common

;=================================================
; checks for the input parameters
;

if not sc.busy_comm then begin
    message, "The communication must be just opened", CONT=(sc.debug eq 0B)
    return, adsec_error.input_type
endif

;**** end of the checks
;==============================================================

if sc.host_comm eq "Dummy" then begin
    err = adsec_error.ok
    print, "DUMMY COMMUNICATION ENABLED"
endif else begin
	err = idl_close_wrap()
endelse

if err eq adsec_error.ok then sc.busy_comm =0b $
else message, "adsec_comm error: "+strtrim(err,2),CONT=(sc.debug eq 0B)

return, err

end
