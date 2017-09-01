; $Id: set_new_gap.pro,v 1.5 2007/04/27 09:57:08 labot Exp $
;+
; NAME:
;   SET_NEW_GAP
;
; PURPOSE:
;   This .pro provides to set a new gap by the three actuators chosen for the isostatic setting.
;   First the all currents are set to zero, after the control of the closed actuator, except 
;   the three_act control, is stopped and the commands of the three_act are  raised by steps
;   (every time removed tilt).
;   When the gap is setted, the control of all actuators is switched on.
;
; CATEGORY:
;   Adsec Hardware Function, Level 3
;
; CALLING SEQUENCE:
;            err = set_new_gap(three_act, three_pos_fin, NSTEPS=nsteps)		
;
; INPUTS:
;   Three_act:  actuators to use to isostatic configuration setting
;   Three_pos:  position in which close the loop in isostatic configuration
;
; OUTPUTS:
;   Error code (see INIT_ERROR_CODES.pro)
;   
; KEYWORDS:
;   NSTEPS: number of steps to raise the gap
;
; COMMON BLOCKS:
;   Adsec std common blocks (see ADSEC_COMMON.pro )
;
;	HISTORY:
;		??? A.Riccardi
;		28 Jul 2004 D.Zanotti(DZ)
;			Data type adapted for LBT	
;
;-
function set_new_gap, three_act, three_pos_fin, NSTEPS=nsteps
@adsec_common

if n_elements(nsteps) eq 0 then nsteps = 10

time_ps = time_preshaper(ACT_LIST_TIME=three_act)

err = update_status(1024)
if err ne adsec_error.ok then message, "ERROR!"

cl_act = where(sys_status.closed_loop)
err = complement(three_act, cl_act, act2open, n_act2open)
if err ne 0 then message, "Error with complement"

err = zero_ctrl_current() 
if err ne adsec_error.ok then message, "ERROR!"

wait,0.1

if n_act2open ne 0 then begin
    err = stop_control(act2open)
    if err ne adsec_error.ok then message, "ERROR!"
endif

comm0=sys_status.command[three_act] 
dcomm=(three_pos_fin-comm0)/nsteps

for i=1,nsteps do begin
    err = write_des_pos(three_act,(comm0+i*dcomm))
    if err ne adsec_error.ok then message, "ERROR!"
    wait,time_ps
    print, "av. current ",mean(sys_status.current[adsec.act_w_curr])
    print, "ctrl currents",sys_status.ctrl_current[three_act]
    err = remove_tilt(three_act, /XAD) ;, /INTEGRATOR)
    if err ne adsec_error.ok then message, "ERROR!"    


endfor

update_panels, /OPT

if n_act2open ne 0 then begin
    
    err = write_des_pos(act2open, sys_status.position[act2open])
    if err ne adsec_error.ok then message, "ERROR!"
    wait, time_ps
    err = start_control(act2open)
    if err ne adsec_error.ok then message, "ERROR!"
 
endif

wait,time_ps

update_panels, /OPT

err = write_des_pos(three_act, sys_status.position[three_act])
if err ne adsec_error.ok then message, "ERROR!"

wait,time_ps

update_panels, /OPT

end




