; $Id: test_sensor.pro,v 1.2 2006/06/20 10:08:05 labot Exp $
;
;+
; NAME:
;       test_sensor
;
; PURPOSE:
;       The function test the capacitive sensor. 
;
; CATEGORY:
;       General utilities.
;
; CALLING SEQUENCE:
;       error = test_sensor(VERBOSE=verbose)
;
; INPUTS:
;       None
;
; OUTPUTS:
;       error: error code.
; KEYWORDS:
;       VERBOSE : if sets print ad display a minimum explanation.
;
; COMMON BLOCKS:
;       common adsec_common_block
; HISTORY
;   March 2006 Daniela Zanotti(DZ)   
;
;-
function test_sensor,  VERBOSE=verbose

@adsec_common
 xadsec = (xregistered('lbt_xadsec') gt 0)

  
 err = update_status()
 if err ne adsec_error.ok then return, err

 sys0 = sys_status   
 volt0 = sys0.voltage
 pos0 = sys0.position
 d0 = adsec.d0

 if total((pos0[adsec.true_act] gt 1.5*d0[adsec.true_act])) gt 1.  or total((pos0[adsec.true_act] lt 0.5*d0[adsec.true_act])) gt 1.  then begin
    out_range = where((pos0 gt 1.5*adsec.d0),cc)
    if keyword_set(verbose) then print, 'The capacitive sensor is out of RIP secure position range for the actuators ', out_range
    return, adsec_error.sensor_fail
    
 endif

 ;idx_no_sat = where((volt0[adsec.true_act] ne adsec.max_volt_val) , c_no_sat)
 ;if c_no_sat eq 0 then begin 
  ; if  keyword_set(verbose) then  print,'All the capacitive voltage are saturated'
   ; return, adsec_error.sensor_fail
 ;endif


 if xadsec then begin
    update_panels, /OPT
    wait, 1
 endif

 return,adsec_error.ok

end







