; $Id: send_linearization.pro,v 1.10 2007/04/11 14:24:57 labot Exp $
;+
; SEND_LINEARIZATION
;
; HISTORY
;  May 2003: Added ACT_LIST keyword. by A. Riccardi. Osservatorio di Arcetri
;  May 2004: AR
;    function totally rewritten to match LBT compatibility
;  01 Dec 2004: Marco Xompero (MX)
;    routine riorganization and cleaning (always send data to adaptive sec.)
;  27 May 2005: MX
;    Alpha_pos position calibration parameter now managed
;   07 Feb 2007 MX
;       LUT of variable deleted and name refurbished.
;   03 April 2007, DZ
;       Added the third parameter in the linearization formula.
;-
function send_linearization, ACT_LIST=act_list, SAFE_VALUES=use_safe_values, BAD_AREA=bad_area

   @adsec_common

   if n_elements(act_list) eq 0 then act_list=indgen(adsec.n_actuators)
   n_acts=n_elements(act_list)
   if not keyword_set(alpha) then alpha = fltarr(n_elements(act_list))+1
   if n_elements(alpha) ne n_elements(act_list) then message, 'Error'

   if keyword_set(use_safe_values) then begin
       dist_threshold = replicate(1.0, n_acts) ;[adcbit]
       dist_B_coeff = replicate(1.0, n_acts) ;[adcbit]
       dist_A_coeff = replicate(0.0, n_acts) ;[m*adcbit]
       dist_C_coeff = replicate(0.0, n_acts)
   endif else begin
       adc_bits = adsec.adc_bits
       vratio = adsec.capsens_vmax/adsec.capsens_vref
       ;; d = d0/(vratio*V_adc/2^adc_bits - c) = dist_A_coeff/(V_adc + dist_B_coeff)
    
       dist_threshold = replicate(1.0, n_acts) ;[adcbit]
       dist_B_coeff = float(-(adsec.c_ratio[act_list]*2d0^adc_bits/vratio)) ;[adcbit]
       dist_A_coeff = float(adsec.alpha_pos[act_list]) * float(2d0^adc_bits/vratio*adsec.d0[act_list]) ;[m*adcbit]
       dist_C_coeff = replicate(0.0, n_acts)

        if keyword_set(bad_area) then begin
           restore, '/home/aosup/adopt/measures/adsec_calib/calibrations/20161026_064602/damaged_area_calibrations.sav', /ver
           dist_B_coeff[idbad] = dist_b_new[idbad]
           dist_A_coeff[idbad] = dist_A_new[idbad]
           dist_C_coeff[idbad] = dist_C_new[idbad]
        endif

   endelse

   err = write_seq_ch(act_list, dsp_map.dist_a_coeff, transpose(dist_A_coeff), /CHECK)
   if err ne adsec_error.ok then return, err
   err = write_seq_ch(act_list, dsp_map.dist_threshold, transpose(dist_threshold), /CHECK)
   if err ne adsec_error.ok then return, err
   err = write_seq_ch(act_list, dsp_map.dist_b_coeff, transpose(dist_B_coeff), /CHECK)
   if err ne adsec_error.ok then return, err
   err = write_seq_ch(act_list, dsp_map.dist_c_coeff, transpose(dist_C_coeff), /CHECK)
   if err ne adsec_error.ok then return, err


   return, adsec_error.ok

end
