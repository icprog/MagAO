;+
;   NAME:
;    DSP_STORAGE
;
;   PURPOSE:
;    This routine enable the storage of diagnostic data in the DSP SDRAM.
;
;   USAGE:
;    err = (IS_PENDING=is_pending)
;
;   INPUT:
;    None.
;
;   OUTPUT:
;    err: error code.
;
;   KEYWORDS:
;    IS_ENABLE: variable filled with the current status of dsp storage flag.
;    DISABLE:   disable the dsp SDRAM storage.     
;-

function dspstorage, DISABLE=disable, IS_ENABLE=is_enable 
@adsec_common

err = read_seq_dsp(0, switch_map.EnableDSPStorage, 1L, is_enable, /ULONG, /SWITCH)
if err ne adsec_error.ok then return, err

if keyword_set(disable) then begin
    if is_enable then  err = write_same_dsp(0, switch_map.EnableDSPStorage, 0l, /SWITCH, /CHECK)
endif else begin
    err = write_same_dsp(0, switch_map.EnableDSPStorage, 1l, /SWITCH, /CHECK)
endelse

return, err

end
