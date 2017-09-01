; $Id: send_force_calibration.pro,v 1.6 2007/03/28 15:28:39 labot Exp $
;+
; SEND_FORCE_CALIBRATION
;
; HISTORY
;  30 May 2004: Written by A. Riccardi. Osservatorio Astrofisico di Arcetri
;  15 Jul 2004: code changes to manage new force calibration
;               variables:dac_N2A_gain,dac_A2bit_gain,dac_bit_offset
;  01 Dec 2004: Marco Xompero (MX) 
;               routine riorganization and cleaning (always send data to adaptive sec.)
;   07 Feb 2007 MX
;       LUT of variable deleted and name refurbished.
;-
function send_force_calibration , ACT_LIST=act_list, SAFE_VALUES=use_safe_values

   @adsec_common

   if n_elements(act_list) eq 0 then act_list=indgen(adsec.n_actuators)
   n_acts=n_elements(act_list)

;  f_for_dac = dac_A2bit_gain*dac_N2A_gain*f_in_fu + dac_bit_offset
;  dac_A2bit_gain = [dacbit/A]
;  dac_N2A_gain   = [A/fu]
;  dac_bit_offset = [dacbit]
;
   if keyword_set(use_safe_values) then begin

      N_per_fu = replicate(1.0, n_acts)
      N_per_A = replicate(1.0, n_acts)
      dac_N2A_gain = replicate(1.0, n_acts)
      dac_A2bit_gain = replicate(0.0, n_acts)
      dac_bit_offset = replicate(float('8000'XL), n_acts)
      dac_gain = dac_A2bit_gain*dac_N2A_gain
      inv_dac_gain = replicate(0.0, n_acts)

   endif else begin

      N_per_fu = adsec.N_per_ccount[act_list]
      N_per_A =  N_per_fu / adsec.A_per_ccount[act_list]
      dac_N2A_gain = 1/(N_per_A/N_per_fu)*(adsec.mag_ref/adsec.mag_efficiency)
;      dac_N2A_gain = (1/adsec.force_gain * adsec.mag_efficiency)*adsec.mag_ref
      dac_A2bit_gain = float(1/adsec.A_per_dacbit[act_list])/float(adsec.DAC_gain[act_list])
      dac_bit_offset = float(adsec.dac_offset[act_list]-adsec.dac_offset_cal*dac_A2bit_gain)

      dac_gain_real = dac_A2bit_gain*dac_N2A_gain
      inv_dac_gain_real = 1.0/dac_gain_real
      idx = where((finite(dac_gain_real) eq 0B) or (finite(inv_dac_gain_real) eq 0B), count)
      if count ne 0 then message, "dac_coil_gain contains zero, NaN or Inf values!"

   endelse

   err = write_seq_ch(act_list, dsp_map.dac_N2A_gain, transpose(dac_N2A_gain), /CHECK)
   if err ne adsec_error.ok then return, err
   err = write_seq_ch(act_list, dsp_map.dac_A2bit_gain, transpose(dac_A2bit_gain), /CHECK)
   if err ne adsec_error.ok then return, err
   err = write_seq_ch(act_list, dsp_map.dac_bit_offset, transpose(dac_bit_offset), /CHECK)
   if err ne adsec_error.ok then return, err

   return, adsec_error.ok

end
