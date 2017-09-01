;+
; GET_DAC_CTRL
;
;   err = get_ dac_ctrl(act_list, count [,/NOT_READY])
;
; KEYWORDS:
; NOT_READY: if set, dac_not_ready is read.    
;
; HISTORY
; 7 December Daniela Zanotti
; 
;-
function get_dac_ctrl, dsp_list, count, NOT_READY=not_ready

@adsec_common

if keyword_set(not_ready) then begin
    addr = dsp_map.dac_not_ready
endif else begin
    addr = dsp_map.dac_not_sent
endelse

err = read_seq_dsp(dsp_list, addr, 1L, count,/ULONG)
return, err

end
