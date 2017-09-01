Function init_adsec_lin, NOFILL=nofill, STRUCT=struct, ACT_LIST=act_list, SAFE=use_safe

@adsec_common

if n_elements(act_list) eq 0 then act_list=indgen(adsec.n_actuators)
n_acts=n_elements(act_list)

;

if keyword_set(use_safe) then begin

   dist_threshold = replicate(1.0, n_acts) ;[adcbit]
   dist_B_coeff = replicate(1.0, n_acts) ;[adcbit]
   dist_A_coeff = replicate(0.0, n_acts) ;[m*adcbit]
   max_lin_dist = replicate(1.0, n_acts) ;[m] V_adc+dist_B_coeff=1bit
   min_lin_dist = replicate(0.0, n_acts) ;[m] V_adc = 2^adc_bits
   m_per_lu     = replicate(1.0, n_acts) ;[lu]=[m]

endif else begin

   adc_bits = adsec.adc_bits
   vratio = adsec.capsens_vmax/adsec.capsens_vref
   ;; d = d0/(vratio*V_adc/2^adc_bits - c) = dist_A_coeff/(V_adc + dist_B_coeff)
   
   dist_threshold = replicate(1.0, n_acts) ;[adcbit]
   dist_B_coeff = float(-(adsec.c_ratio[act_list]*2d0^adc_bits/vratio)) ;[adcbit]
   dist_A_coeff = float(2d0^adc_bits/vratio*adsec.d0[act_list]) ;[m*adcbit]
   max_lin_dist = dist_A_coeff/dist_threshold ;[m] V_adc+dist_B_coeff=1bit
   min_lin_dist = dist_A_coeff/((2UL^adc_bits-1)+dist_B_coeff) ;[m] V_adc = 2^adc_bits
   m_per_lu     = replicate(1.0, n_acts) ;[lu]=[m]

endelse

struct = $
  { $
   m_per_pcount  : fltarr(adsec.n_actuators), $
   max_lin_output: fltarr(adsec.n_actuators), $
   max_lin_dist  : fltarr(adsec.n_actuators), $
   min_lin_dist  : fltarr(adsec.n_actuators), $
   max_pos_val   : fltarr(adsec.n_actuators), $
   min_pos_val   : fltarr(adsec.n_actuators)  $
 }


struct.m_per_pcount[act_list]  = m_per_lu
struct.max_lin_output[act_list]= max_lin_dist
struct.max_lin_dist[act_list]  = max_lin_dist
struct.min_lin_dist[act_list]  = min_lin_dist
struct.max_pos_val[act_list]   = max_lin_dist/m_per_lu ; max available linearized position [lu]
struct.min_pos_val[act_list]   = min_lin_dist/m_per_lu ; min available linearized position [lu]

 

;==================adsec calibration initizialzation==========================
;
; End of: Restoring the capsens calibrations
;*
    ;;;*** Capacitive Sensors ***
    
if not keyword_set(nofill) then begin

   adsec.m_per_pcount[act_list]  = m_per_lu
   adsec.max_lin_output[act_list]= max_lin_dist
   adsec.max_lin_dist[act_list]  = max_lin_dist
   adsec.min_lin_dist[act_list]  = min_lin_dist
   adsec.max_pos_val[act_list]   = max_lin_dist/m_per_lu ; max available linearized position [lu]
   adsec.min_pos_val[act_list]   = min_lin_dist/m_per_lu ; min available linearized position [lu]


endif

return, adsec_error.ok
end
