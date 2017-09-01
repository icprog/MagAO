; $Id: set_sat_force.pro,v 1.2 2007/02/07 16:57:57 marco Exp $
;+
; SET_SAT_FORCE
;
; err = set_sat_force(act_list, pos_sat_force, neg_sat_force)
;
; HISTORY
;   14 July 2004: written
;   07 Feb 2007 MX
;       LUT of variable deleted and name refurbished.
;-
function set_sat_force, act_list, pos_sat_force, neg_sat_force

@adsec_common

addr = dsp_map.sat_DAC_value
if n_elements(pos_sat_force) eq 1 then begin
    err = write_same_ch(act_list, addr, pos_sat_force[0], /CHECK)
endif else begin
    err = write_seq_ch(act_list, addr, pos_sat_force, /CHECK)
endelse

addr = dsp_map.nsat_DAC_value
if n_elements(neg_sat_force) eq 1 then begin
    err = write_same_ch(act_list, addr, neg_sat_force[0], /CHECK)
endif else begin
    err = write_seq_ch(act_list, addr, neg_sat_force, /CHECK)
endelse

return, err

end
