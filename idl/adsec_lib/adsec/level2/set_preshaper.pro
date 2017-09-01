; $Id: set_preshaper.pro,v 1.9 2007/02/07 16:57:57 marco Exp $
;+
; SET_PRESHAPER
;
; err = set_preshaper(act_list, preshaper [,/FF])
;
; HISTORY
;   13 July 2004: A. Riccardi
;       long2float conversion no longer needed
;       read-back check done by default
;   04 Aug 2004: AR
;       added tests on input type/size
;   05 Aug 2004, AR and M. Xompero
;       bug on the test of the preshaper value fixed
;   06 Aug 2004, AR and MX
;       another bug on the test of the preshaper value fixed
;   07 Feb 2007 MX
;       LUT of variable deleted and name refurbished.
;-
function set_preshaper, act_list, the_preshaper, FF=ff

@adsec_common

if test_type(the_preshaper, /INT, /UINT, /LONG, /ULONG, N_EL=n_ps) then begin
    message, "preshaper must be integer or long type.", CONT=sc.debug eq 0B
    return, adsec_error.input_type
endif
preshaper = ulong(the_preshaper)
if (total(preshaper le 0L) ne 0) or (total(preshaper gt adsec.preshaper_len) ne 0) then begin
    message, "Unexpected value for preshaper.", CONT=sc.debug eq 0B
    return, adsec_error.input_type
endif

if keyword_set(ff) then $
    addr = dsp_map.step_ptr_preshaper_curr $
else $
    addr = dsp_map.step_ptr_preshaper_cmd


if n_ps eq 1 then begin
    err = write_same_ch(act_list, addr, preshaper[0], /CHECK)
endif else begin
    err = write_seq_ch(act_list, addr, preshaper, /CHECK)
endelse

return, err

end
