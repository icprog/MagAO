; $Id: raise_curr.pro,v 1.8 2007/06/26 13:56:16 labot Exp $
;+
; NAME:
;   RAISE_CURR
;
; PURPOSE:
;   To raise up or down the current in the coils by 100 (number fixed) little steps
;   on all actuators.
;
; CATEGORY:
;   Adsec Hardware Function, Level 3
;
; CALLING SEQUENCE:
;
;   Err = RAISE_CURR(initial_current)
;
; OPTIONAL INPUTS:
;   initial_current:  if not set, the current chosen is the safety current
;               that press the shell on th e reference plate.
;
; OUTPUTS:
;   Error code (see INIT_ERROR_CODES.pro)
;
;
; COMMON BLOCKS:
;   Adsec std common blocks (see ADSEC_COMMON.pro )
;
; EXAMPLE:
;
;   Raise the applied currents up to 200 counts:
;
;     err = raise_curr(200)
;
; HISTORY
;
; ?? ??? ????: written by A. Riccardi (AR)
;
; 03 Feb 2004: AR and M. Xompero (MX)
;     *adsec.curr4bias_mag introduced.
; 17 Feb 2004: MX
;     *adsec.secure_factor introduced.
; 28 May 2004, AR
;     *modifcations to match LBT formats
; 02 Nov 2004, MX
;    Adsec.all changed in sc.all.
; 25 June 2007, DZ
;    Added the variable sys0
;-
function raise_curr, initial_current

@adsec_common

if n_elements(initial_current) eq 0 then begin
    currents=replicate(adsec.secure_factor*adsec.weight_curr, adsec.n_actuators) $
             + (adsec.curr4bias_mag * adsec.weight_curr)
endif else $
    currents=replicate(initial_current,adsec.n_actuators) + $
              (adsec.curr4bias_mag * adsec.weight_curr)

err=stop_control(sc.all_actuators)
if err ne adsec_error.ok then return,err

err=clear_ff()
if err ne adsec_error.ok then return,err

err = update_status(1)
if err ne adsec_error.ok then return,err
sys0 = sys_status
nstep=100
d_curr = currents[adsec.act_w_curr]-sys0.current[adsec.act_w_curr]
;d_curr = d_curr*((findgen(nstep+1)/nstep)^(0.5))

for i=0,nstep do begin
    fact = (float(i)/nstep)^(0.5)
    err = write_bias_curr(adsec.act_w_curr, d_curr*fact+sys0.current[adsec.act_w_curr])
    if err ne adsec_error.ok then return,err
    wait, 0.01
endfor

return,adsec_error.ok

end
