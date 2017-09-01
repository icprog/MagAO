; $Id: remove_tilt.pro,v 1.13 2009/04/10 13:44:14 labot Exp $
;
;+
; NAME:
;   REMOVE_TILT
;
; PURPOSE: 
;   Remove from the gap modal shapes.
;
; USAGE:
;   err = remove_tilt(cl_act) 
;
; INPUT:
;   cl_act: closed loop actuator to use
;
; OUTPUT:
;   err: error code.
;
; KEYWORDS:
;   INTEGRATOR: use sw integrator in the process (*)
;   MAX_AMP_POS: see integrator.pro; used only if integrator is set
;   XADSEC: update GUI during the correction
;   ZERN_IDX: zernike mode index to remove (start from 1 !!)
;   STEP_SIZE: maximum step size from current shape to corrected shape.
;   OFFLOAD: use mode offload ASM feature (**)
;   MODAL_FIT: correct the zern modes choosen considering a modal fit on the ff pos eigenvactors.
;
;   !!!! NB !!!! : (*)    use ONLY WITHOUT OVERSAMPLING FRAMES OR SLOPE COMPUTER RUNNING
;                  (**)   use ONLY WITH OVERSAMPLING FRAMES OR SLOPE COMPUTER RUNNING
;
; HISTORY
;   28 May 2004, A. Riccardi
;       Modifications to match LBT formats
;   29 Jul 2004, D.Zanotti
;	    A slower preshaper inserted before write_des_pos and 
;	    then the initial condition of preshaper was red-estabilshed.
;	    Added time_ps, a waited time like the raising time of preshaper.
;	    Deleted the count2meter and meter2count transformation because no longer in use 
;   02 Nov 2004, MX
;       Adsec.all changed in sc.all.
;   07 Feb 2007 MX
;       LUT of variable deleted and name refurbished.
;   13 Jul 2007, MX
;       Function expanded in order to fit a list of zernike modes
;-
function remove_tilt, cl_act, INTEGRATOR=integrate, MAX_AMP_POS=max_amp_pos $
         ,XADSEC=xadsec, ZERN_IDX=zern_idx, STEP_SIZE=step_size, OFFLOAD=offload, MODAL_FIT=modal_fit

@adsec_common

if n_elements(max_amp_pos) eq 0 then max_amp_pos = adsec.max_amp_pos_tilt

err = update_status()
if err ne adsec_error.ok then return, err
if keyword_set(XADSEC) then update_panels, /NO_READ
sys_status_save = sys_status

x = adsec.act_coordinates[0,adsec.act_w_pos]/adsec_shell.out_radius
y = adsec.act_coordinates[1,adsec.act_w_pos]/adsec_shell.out_radius
z = sys_status_save.position[adsec.act_w_pos]

if n_elements(zern_idx) eq 0 then zern_idx = [2,3]
fit = fltarr(adsec.n_actuators)

zern_degree, max(zern_idx), n, m
zern_list = zern_order(n)
zern_list = [1, zern_list[sort(zern_list)]]
idz = where(zern_list eq zern_idx[0])
if n_elements(zern_idx) gt 1 then begin
    for i=1, n_elements(zern_idx)-1 do idz = [idz, where(zern_list eq zern_idx[i])]
endif


fit0 = float(reform(surf_fit(x,y,z,zern_list,COEFF=old_coeff, /ZERN, UMAT=umat)))
fit[adsec.act_w_pos] = fit0
fit = fit[cl_act]
fit_red = umat[idz,*] ## transpose(old_coeff[idz])

print,"zernike coeffs before correction "
print,[[fix(zern_list[idz])],[reform(old_coeff[idz])]]

idx_cl_act=intarr(n_elements(cl_act))
for i=0,n_elements(cl_act)-1 do begin
   idx_cl_act[i]=where(adsec.act_w_pos eq cl_act[i],count)
   if count ne 1 then message,'tragical error !?@'
endfor

step_ps = 1L
time_ps	= adsec.preshaper_len/step_ps*adsec.sampling_time
time_ps = time_preshaper()
;err = get_preshaper(sc.all_actuators, pp)        ;read cmd preshaper
;if err ne adsec_error.ok then return,err
;err = get_preshaper(sc.all_actuators, fp,/FF)    ;read curr preshaper
;if err ne adsec_error.ok then return,err

;err = set_preshaper(sc.all_actuators, step_ps)
;if err ne adsec_error.ok then return,err
;err = set_preshaper(sc.all_actuators, step_ps, /FF)
;if err ne adsec_error.ok then return,err


err = get_commands(cl_act,comm0, OFFLOADCMD=offload)
if err ne adsec_error.ok then return, err

if keyword_set(MODAL_FIT) then begin
;    dcomm = -float(fit_red)
    full_dcomm = fltarr(672)
    full_dcomm[adsec.act_w_pos] = -fit_red

    coeff_dcomm = transpose(adsec.ff_p_svec) ## full_dcomm
    coeff_dcomm[11:*] = 0d
    dcomm_cut = float(adsec.ff_p_svec ## coeff_dcomm)
    dcomm = dcomm_cut[adsec.act_w_pos]

endif else begin

    dcomm = -float(fit_red)

endelse


if n_elements(step_size) eq 0 then step_size = adsec.max_amp_pos

;stop
;return, 0
if keyword_set(OFFLOAD) then begin

        step_size_amp = step_size < adsec.max_amp_pos
        n_step = (max(floor(dcomm / step_size_amp))) > 1
        c_step = dcomm/n_step
        
        ;for i_step=1, n_step do begin
        ;    temp_comm = comm0+i_step*c_step
        ;    err = write_des_pos(cl_act, temp_comm)
        ;    if err ne adsec_error.ok then return, err
        ;    wait, time_ps
        ;    err = update_status()
        ;    if err ne adsec_error.ok then return,err
        ;    if keyword_set(xadsec) then update_panels, /NO_READ
        ;    
                                ;endfor        

        err = get_commands(adsec.act_w_curr, cmdcurr, /CURR)
        if err ne adsec_error.ok then return,err 

        err = set_offload_force(cmdcurr, /SWITCH )
        if err ne adsec_error.ok then return,err

        for i_step=1, n_step do begin
            temp_comm =comm0+i_step*c_step
;            if adsec.act_wo_cl_on_set[0] gt 0 then begin
                dummy = fltarr(adsec.n_actuators)
                dummy[cl_act] = temp_comm
                dummy = dummy[adsec.act_w_pos]
;            endif else begin
;                dummy[cl_act] = temp_comm
;                dummy = dummy[adsec.act_w_pos]
;            endelse
            
            err = set_offload_cmd(dummy, /SWITCH, /NOCHECK, /START)
            if err ne adsec_error.ok then return,err
            wait,time_ps 
            if err ne adsec_error.ok then return,err
            err = update_status()
            if keyword_set(xadsec) then update_panels, /NO_READ

        endfor   

endif else begin

    if step_size eq -1 then begin
        
        temp_comm = comm0+dcomm
        err = write_des_pos(cl_act, temp_comm)
        if err ne adsec_error.ok then return, err
        wait, time_ps
        err = update_status()
        if err ne adsec_error.ok then return,err
        if keyword_set(xadsec) then update_panels, /NO_READ
        
    endif else begin    
        
        step_size_amp = step_size < adsec.max_amp_pos
        n_step = max(floor(dcomm / step_size_amp))
        c_step = dcomm/n_step
        
        for i_step=1, n_step do begin
            temp_comm = comm0+i_step*c_step
            err = write_des_pos(cl_act, temp_comm)
            if err ne adsec_error.ok then return, err
            wait, time_ps
            err = update_status()
            if err ne adsec_error.ok then return,err
            if keyword_set(xadsec) then update_panels, /NO_READ
            
        endfor
        
    endelse


    if keyword_set(integrate) then begin
        err = integrator(cl_act,THR=max_amp_pos,MAXCURR=max_curr,PERR=perr,ITER=iter,PAMP=pamp)
        if err ne adsec_error.ok then message, "ERROR!!"
    endif

endelse

err = update_status()
if err ne adsec_error.ok then return,err
if keyword_set(xadsec) then update_panels, /NO_READ
sys_status_save = sys_status

z = sys_status_save.position[adsec.act_w_pos]
fit = reform(surf_fit(x,y,z,zern_list,COEFF=new_coeff, /ZERN))

print,"zernike coeffs after correction "
print, [[zern_list[idz]],[reform(new_coeff[idz])]]

;err = set_preshaper(sc.all_actuators, pp)
;if err ne adsec_error.ok then return,err
;err = set_preshaper(sc.all_actuators, fp, /FF)
;if err ne adsec_error.ok then return,err

return, err
end
