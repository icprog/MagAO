; $Id: find_dust.pro,v 1.3 2006/06/20 10:03:11 labot Exp $
;
;+
; NAME:
;       find_dust
;
; PURPOSE:
;       The function finds a dust contamination on the mirror and returns
;       the mirror status. 
;
; CATEGORY:
;       General utilities.
;
; CALLING SEQUENCE:
;       error = find_dust(status, act_dust,VERBOSE=verbose, POS0=pos0, POS1=pos1, VOLT0=volt0, VOLT1=VOLT1)
;
; INPUTS:
;       status : returns the mirror dust contamination.
;       act_dust : if dust was found then the near actuators were returned.
;
;
; OUTPUTS:
;       error: error code.
; KEYWORDS:
;       VERBOSE : if sets print ad display a minimum explanation.
;       POS0    : if sets returns the initial mirror position.
;       POS1    : if sets returns the final mirror position.
;       VOLT0   : if sets returns the sensor voltage initial position. 
;       VOLT1   : if sets returns the sensor voltage final position. 
;
; COMMON BLOCKS:
;       common adsec_common_block
; HISTORY
;   January 2006 Daniela Zanotti(DZ)   
;   Added an update_panels.
;
;-
function find_dust, status, act_dust, VERBOSE=verbose, POS0=pos0, POS1=pos1, VOLT0=volt0, VOLT1=VOLT1

@adsec_common
 xadsec = (xregistered('lbt_xadsec') gt 0)

 init_curr = adsec.weight_curr/2.0;0.03 ;3000
 
 curr = fltarr(adsec.n_actuators)
 curr[*] = init_curr
  
 err = update_status()
 if err ne adsec_error.ok then return, err

 sys0 = sys_status   
 volt0 = sys0.voltage
 pos0 = sys0.position

 err = stop_control(sc.all)  
 if err ne adsec_error.ok then return, err

;test disable coils
 err = test_coils(status_coil, /DISABLE) ; !!!!!!!!!!!!!!da debuggare!!!!!!!!!!!!
 if err ne adsec_error.ok then return, err

 if status_coil then begin
    err = clear_dacs(sc.all)
    if err ne adsec_error.ok then return, err
;Enables coils
    err = enable_coils()
    if err ne adsec_error.ok then return, err
    if (verbose) then print, 'Control stopped and DACs cleaned. Coils enabled.'
 endif
  

 err = write_bias_curr(adsec.act_w_curr, curr[adsec.act_w_curr])
 if err ne adsec_error.ok then return, err

 wait,.2

 err = update_status()
 if err ne adsec_error.ok then return, err
 
 sys1 = sys_status   
 volt1 = sys1.voltage
 pos1 = sys1.position
  
 err = clear_dacs(sc.all)
 if err ne adsec_error.ok then return, err

 err = disable_coils()    
 if err ne adsec_error.ok then return, err

; Check dust contamination
 delta_pos = pos1-pos0

 idx_no_sat = where((volt0 ne adsec.max_volt_val) and (volt1 ne adsec.max_volt_val), c_no_sat)
 if c_no_sat eq 0 then message,'All the actuators are saturated'

;introdotto a causa del buco nel reference plate============ 
 act_w_hole = [2,14,15,26,34]
 err = complement(act_w_hole, adsec.act_w_pos, new_act_list)
;==========da togliere per lbt==============================

 is_empty = intersection(idx_no_sat, new_act_list, idx_good)
 if is_empty then begin
    if keyword_set(verbose) then print, 'No dust was found, and no actuators to display'
    status = 0
 endif else begin
    status = 1
    act_dust = idx_good    
    if keyword_set(verbose)  then begin
	    minv = min([(pos0[idx_good])*1e6,(pos1[idx_good])*1e6], MAX=maxv)
        if (!d.flags and 256) eq 256 then window,/free,xs=900,ys=700
        display,(delta_pos*1e6)[idx_good],idx_good,/sm,/sh, min_v=-6.0, max_v=1.5,tit='Delta Position [um]'
        if (!d.flags and 256) eq 256 then window,/free,xs=900,ys=700
        display,pos0[idx_good]*1e6,idx_good,/sh,/sm,tit='Initial Position [um]', MIN=minv, MAX=maxv
        if (!d.flags and 256) eq 256 then window,/free,xs=900,ys=700
        display,pos1[idx_good]*1e6,idx_good,/sh,/sm,tit='Final Position [um]', MIN=minv, MAX=maxv
        print, 'The actuators near to dust are '+strtrim(act_dust)
    endif
 endelse


 if xadsec then begin
    update_panels, /OPT
    wait, 1
 endif

 return,adsec_error.ok

end







