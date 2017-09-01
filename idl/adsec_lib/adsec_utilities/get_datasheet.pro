;$Id: get_datasheet.pro,v 1.1 2004/07/13 16:01:12 riccardi Exp $
;+
; HISTORY
;   13 July 2004, written by A. Riccardi
;-
function get_datasheet, BCU=is_bcu, SIGGEN=is_siggen

@adsec_common

if keyword_set(is_bcu)+keyword_set(is_siggen) gt 1 then begin
    message, "BCU and SIGGEN keywords are exclusive"
endif
case 1B of
    keyword_set(is_bcu): return, bcu_datasheet
    keyword_set(is_siggen): return, siggen_datasheet
    else: return, dsp_datasheet
endcase

end
