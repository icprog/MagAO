; $Id: get_global_counter.pro,v 1.1 2004/08/09 17:44:47 riccardi Exp $
;
;+
; GET_GLOBAL_COUNTER
;
; err = get_global_counter(dsp_list, gc)
;
; HISTORY
; 07 Aug 2004, written by A. Riccardi (AR)
;-
function get_global_counter, dsp_list, gc

@adsec_common

err = read_seq_dsp(dsp_list, dsp_map.global_counter, 1L, gc, /ULONG)

return, err

end
