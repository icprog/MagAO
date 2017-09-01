; $Id: integrator.pro,v 1.8 2007/07/13 12:25:38 labot Exp $
;+
; NAME:
; 	INTEGRATOR
;
; PURPOSE:
;   This .pro provides to integrate the position of a list of actuators in order to 
;	a maximum number of iteration or maximum value of current, or a position error threshold.
;   The delta currents, between average and status bias currents, are written in every step.   
;
;
; CATEGORY:
;   Adsec Hardware Function, Level 3
;
; CALLING SEQUENCE:
;   Err = integrator(act_list,THR=max_amp_pos,MAXCURR=max_curr,MAXITER=maxiter,
;                    PERR=perr,ITER=iter,PAMP=pamp,XADSEC=xadsec)
; INPUTS:
;   act_list: actuators list for which the values of distance and current are integrate.  
; OUTPUTS:
;   Error code (see INIT_ERROR_CODES.pro)
;
; KEYWORDS:
;   THR:     Float. Position error threshold. 
;	MAXCURR: Float.Maximum current threshold.
;	MAXITER: Int.Maximum number of iterations for integration.
;	PERR:    Output variable of position error.
;	ITER:    Final number of iterations.
;	PAMP:    Final output of difference between maximum and minimum position amplitude.
;   XADSEC:  Provide to update the user interface.
;
; HISTORY
;
;   Written by Guido Brusa-Zappellini (GBZ) and A. Riccardi (AR)
;   Osservatorio Asrofisico di Arcetri, ITALY
;   <riccardi@arcetri.astro.it>
;	27 Jul D.Zanotti (DZ)
;	Added parameters in adsec structure:
;	thr_perr,max_curr,max_iter_integ
;    
;       04 Aug 2004, AR
;         removed stop commands
;       02 Nov 2004, MX
;         Adsec.all changed in sc.all.
;   07 Feb 2007 MX
;       LUT of variable deleted and name refurbished.
;   26 Feb 2006
;       Better management of bias current applied.
;   13 Jul 2007, MX
;       Fixed bug: when iter is equal to 1, now only move ctrl currents into bias currents.
;-
function integrator,act_list,THR=thr,MAXCURR=maxcurr,MAXITER=maxiter $
			,PERR=perr,ITER=iter,PAMP=pamp,XADSEC=xadsec
@adsec_common

if n_elements(thr) eq 0 then thr = adsec.thr_perr 
if n_elements(maxcurr) eq 0 then maxcurr= adsec.max_curr
if n_elements(maxiter) eq 0 then maxiter= adsec.max_iter_integ
if act_list[0] eq sc.all_actuators then list=indgen(adsec.n_actuators) else list=act_list
err = get_commands(act_list,comm)
if err ne 0 then return, err

iter=0
repeat begin
    err = get_ave_pc(ave_pos,ave_curr,min_pos,max_pos,SAMPLES=1024UL,DELAY=0UL)
    if err ne adsec_error.ok then return, err

    ;err= read_seq_ch(list,dsp_map.ff_ud_current,1L,ff_current)
    ;if err ne adsec_error.ok then return, err
    err= read_seq_ch(list,dsp_map.cmd_current,1L,cmd_curr)
    if err ne adsec_error.ok then return, err
    err= read_seq_ch(list,dsp_map.bias_current,1L,bias)
    if err ne adsec_error.ok then return, err


    ;err=write_bias_curr(list,ave_curr[list]-reform(ff_current))
    ;if err ne adsec_error.ok then return, err

    err=write_bias_curr(list,ave_curr[list]-reform(cmd_curr)+reform(bias))
    if err ne adsec_error.ok then return, err

    if keyword_set(xadsec) then update_panels,/opt

    pamp=max_pos-min_pos
    perr=comm-ave_pos[list]
    iter=iter+1

    wait, 0.1
endrep until  (max(abs(perr)) le thr) or (iter ge maxiter) or max(abs(ave_curr)) gt maxcurr

return,0
end
