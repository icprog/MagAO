;
;+
; NAME
;   GET_frames_counter
;
; PURPOSE:
;   Get the frame counter from the adaptive secondary.
;
; USAGE:
;   err = get_wfs_frames_counter(dsp_list, gc)
;
; HISTORY
;   Created on 2005 by ????
;   07 Feb 2007 MX
;       LUT of variable deleted and name refurbished.
;-
function get_frames_counter, dsp_list, gc

@adsec_common

err = read_seq_dsp(dsp_list, rtr_map.wfs_frames_counter, 1L, fc, /ULONG)

return, err

end
