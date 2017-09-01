Function init_adsec_forces, NOFILL=nofill, STRUCT=struct, ACT_LIST=act_list, SAFE=use_safe

@adsec_common

if n_elements(act_list) eq 0 then act_list=indgen(adsec.n_actuators)
n_acts=n_elements(act_list)

;  f_for_dac = dac_A2bit_gain*dac_N2A_gain*f_in_fu + dac_bit_offset
;  dac_A2bit_gain = [dacbit/A]
;  dac_N2A_gain   = [A/fu]
;  dac_bit_offset = [dacbit]
;

if keyword_set(use_safe) then begin

   N_per_fu = replicate(1.0, n_acts)
   fu_str = "N"
   N_per_A = replicate(1.0, n_acts)
   dac_N2A_gain = replicate(1.0, n_acts)
   dac_A2bit_gain = replicate(0.0, n_acts)
   dac_bit_offset = replicate(float('8000'XL), n_acts)
   dac_gain = dac_A2bit_gain*dac_N2A_gain
   inv_dac_gain = replicate(0.0, n_acts)

endif else begin

   N_per_fu = replicate(1.0, n_acts)
if n_acts le 48 then N_per_A = adsec.act_efficiency[act_list]*sqrt(adsec.coil_res[act_list]) else  N_per_A = adsec.force_gain
   dac_N2A_gain = 1/N_per_A
   dac_A2bit_gain = float(1/adsec.A_per_dacbit[act_list])
   dac_bit_offset = float(adsec.dac_offset[act_list])
   dac_gain = dac_A2bit_gain*dac_N2A_gain
   inv_dac_gain = 1.0/dac_gain

   idx = where((finite(dac_gain) eq 0B) or (finite(inv_dac_gain) eq 0B), count)
   if count ne 0 then message, "dac_coil_gain contains zero, NaN or Inf values!"

endelse


dac_bits = adsec.dac_bits
min_curr_val = fltarr(adsec.n_actuators)
max_curr_val = fltarr(adsec.n_actuators)
max_curr_val[act_list] = float((         0d0 - dac_bit_offset)*inv_dac_gain)
min_curr_val[act_list] = float((2d0^dac_bits - dac_bit_offset)*inv_dac_gain)

struct = $
  { $
    min_curr_val : min_curr_val ,$ ; minimum current value in internal force units [fu]
    max_curr_val : max_curr_val ,$ ; maximum current value in internal force units [fu]
    A_per_ccount : N_per_fu/N_per_A ,$ ; amperes per current count [A/count]
    N_per_ccount : N_per_fu $ ; Newton per current count [N/count]
 }

;==================adsec calibration initizialzation==========================
;
; End of: Restoring the capsens calibrations
;*
    ;;;*** Capacitive Sensors ***
    
if not keyword_set(nofill) then begin

   adsec.N_per_ccount[act_list] = N_per_fu
   adsec.A_per_ccount[act_list] = N_per_fu/N_per_A
   adsec.max_curr_val[act_list] = float((         0d0 - dac_bit_offset)*inv_dac_gain)
   adsec.min_curr_val[act_list] = float((2d0^adsec.dac_bits - dac_bit_offset)*inv_dac_gain)

endif

return, adsec_error.ok
end

