; $Id: get_preshaper.pro,v 1.6 2007/02/07 16:57:57 marco Exp $
;
;+
; GET_PRESHAPER
;
; err = get_preshaper(act_list, preshaper [, /FF])
;
; HISTORY
;   27 May 2004, A. Riccardi (AR)
;     long data type (1L) used in read_seq_ch for LBT compatibility
;
;   03 Aug 2004, AR and M. Xompero
;     preshaper is read as ulong type
;   07 Feb 2007 MX
;       LUT of variable deleted and name refurbished.
;-
function get_preshaper, act_list, preshaper, FF=ff

@adsec_common

if keyword_set(ff) then $
    addr = dsp_map.step_ptr_preshaper_curr $
else $
    addr = dsp_map.step_ptr_preshaper_cmd

err = read_seq_ch(act_list, addr, 1L, preshaper,/ULONG)

return, err

end
