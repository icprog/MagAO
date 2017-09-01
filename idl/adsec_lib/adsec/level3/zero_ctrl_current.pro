; $Id: zero_ctrl_current.pro,v 1.9 2008/08/25 14:22:57 labot Exp $
;+
; NAME:
;   ZERO_CTR_CURRENT
;
; PURPOSE:
;   This .pro provides to set the control currents to zero.
;	First the control currents of closed loop actuators are redistributed to the bias currents, 
;	after the commands of cl_act are placed to their actual positions, so the control currents become zero. 
; CATEGORY:
;   Adsec Hardware Function, Level 3
;
; CALLING SEQUENCE:
;   Err =  ZERO_CTR_CURRENT ()
;
; INPUT:
;   None.
;
; OUTPUT:
;   Err: Error code.
;
; KEYWORDS:
;   OFFLOAD:    use mode offload to perform the nulling of control
;               currents
;   USE_OFFLOAD_TO_ZERO_CTRL: ctrl current is zeroed adding the
;                             position error to the command offload
;                             and control force to force offload. 
;   XADSEC:     if set, it forces an update of xadsec panels
;
; COMMON BLOCKS:
;   Adsec std common blocks (see ADSEC_COMMON.pro )
;
; HISTORY
;   28 May 2004, A. Riccardi
;     Modifications to match LBT formats
;   29 Jul 2004, D.Zanotti
;     Added waiting time after write_des_pos
;   02 Nov 2004, MX
;     Adsec.all changed in sc.all.
;   04 Nov 2007, MX
;     OFFLOAD keyword added.
;   14 Aug 2009, AR
;     USE_OFFLOAD_TO_ZERO_CTRL keyword added
;   01 Mar 2010, AR
;     Added XADSEC keyword
;-

function zero_ctrl_current, OFFLOAD=offload, USE_OFFLOAD_TO_ZERO_CTRL=use_offload, XADSEC=xadsec

@adsec_common

err = update_status()
if err ne adsec_error.ok then return,err
pos0 = sys_status.position
time_ps = time_preshaper()

if keyword_set(use_offload) then begin
    dcomm=sys_status.position-sys_status.command
    dforce=sys_status.ctrl_current
    err = set_offload_cmd(dcomm[adsec.act_w_pos],/DELTA, /NOCHECK)
    if err ne adsec_error.ok then return,err
    err = set_offload_force(dforce[adsec.act_w_curr],/DELTA,/START, /NOCHECK)
    if err ne adsec_error.ok then return,err
    wait, time_ps*0.1
endif else if keyword_set(OFFLOAD) then begin

    err = set_curr2bias(/FULL, /CONTROL, XADSEC=xadsec)
    if err ne adsec_error.ok then return,err

endif else begin
    err = get_preshaper(sc.all_actuators, pp) ;read cmd preshaper
    if err ne adsec_error.ok then return,err
    time_ps=adsec.preshaper_len/min(pp)*adsec.sampling_time

    cl_act = where(sys_status.closed_loop, count)
    print, "count", count
    if count ne 0 then begin
        err = integrator(cl_act,MAXITER=1)
        if err ne adsec_error.ok then return,err
        
        err = write_des_pos(cl_act,pos0[cl_act])
        if err ne adsec_error.ok then return,err
        wait, time_ps
    endif
    return,adsec_error.ok
endelse
end
