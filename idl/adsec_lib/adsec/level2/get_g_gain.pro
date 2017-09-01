;$Id: get_g_gain.pro,v 1.2 2007/02/07 16:57:57 marco Exp $$
;+
;   NAME:
;     GET_G_GAIN
;   
;   PURPOSE:
;     Retrive from the AdSec the optical loop gain.
;
;   USAGE:
;     err = get_g_gain(g_gain)
; 
;
;   HISTORY:
;     Made on Nov 2005
;   07 Feb 2007 MX
;       LUT of variable deleted and name refurbished.
;-

function get_g_gain, g_gain, BLOCK=block

    @adsec_common

    if not(keyword_set(block)) then  begin

        err = read_seq_ch(sc.all, dsp_map.gain_g_a, 1L, g_gain)
        return, err

    endif else begin

        err = read_seq_ch(sc.all, dsp_map.gain_g_b, 1L, g_gain)
        return, err

    endelse

end
