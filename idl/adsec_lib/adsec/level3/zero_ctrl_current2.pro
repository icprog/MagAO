; $Id: zero_ctrl_current2.pro,v 1.1 2009/05/22 16:03:50 labot Exp $
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
;-

function zero_ctrl_current2

@adsec_common

err = update_status()
if err ne adsec_error.ok then return,err
pos0 = sys_status.position
time_ps = time_preshaper()

if rtr.oversampling_time gt 0 then begin

    newcmd = sys_status.position-sys_status.bias_command
    err = set_offload_cmd(newcmd[adsec.act_w_pos], /NOCHECK, /START)

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
