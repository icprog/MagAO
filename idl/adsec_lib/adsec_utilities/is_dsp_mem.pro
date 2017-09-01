;$Id: is_dsp_mem.pro,v 1.1 2004/07/13 16:01:12 riccardi Exp $
;+
; IS_DSP_MEM
;
; ret = is_dsp_mem(set_pm)
;
; ret is 1B or 0B if set_pm is related to dsp or board memory respectively
;
; HYSTORY
;   13 July 2004 written by A. Riccardi
;-
function is_dsp_mem, set_pm

    @adsec_common

    case set_pm of
       dsp_datasheet.data_mem_id: return, 1B
       dsp_datasheet.prog_mem_id: return, 1B
       else: return, 0B
    endcase
end
