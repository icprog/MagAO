
;NB NON e' stato ben capito perche' funziona meglio dell'offload del
;comando integrato
Function piston_shell, final_piston, max_amp_pos_tmp, XADSEC=xadsec


    @adsec_common
    if max(sys_status.icc_cutfreq) eq 0 then t_icc=0 else t_icc = -alog(1e-3)/2/!pi/5.
    time_ps = time_preshaper()

    err = update_status()
    if err ne adsec_error.ok then return, err
    if keyword_set(xadsec) then update_panels, /NO_READ, /OPT

    err = offload_icc()
    if err ne adsec_error.ok then return, err
    wait, (time_ps+2*rtr.oversampling_time+  t_icc) > 0.1

    err = update_status()
    if err ne adsec_error.ok then return, err
    if keyword_set(xadsec) then update_panels, /NO_READ, /OPT
    

    if keyword_set(xadsec) then update_panels, /NO_READ, /OPT
    current_piston = mean(sys_status.position[adsec.act_w_pos])
    cl_list = adsec.act_w_cl
    if n_elements(max_amp_pos_tmp) eq 0 then max_amp_pos_tmp=2e-6
    max_amp_pos_tmp  = max_amp_pos_tmp < 1e-6
    if n_elements(final_piston) eq 0 then final_piston = 60e-6
    delta_piston = final_piston-current_piston
    nsteps = ceil(delta_piston/max_amp_pos_tmp)
    max_amp_pos = delta_piston/float(nsteps)

    mvec = fltarr(adsec.n_actuators)
    for i=0, adsec.n_actuators-1 do mvec[i] = mean(adsec.ff_p_svec[i,adsec.act_w_cl])
    mm = max(mvec, id_pist)

    pist = (adsec.ff_p_svec[id_pist,cl_list] - mean(adsec.ff_p_svec[id_pist, cl_list])+1)*max_amp_pos
    
    for i_step=0,nsteps-1 do begin

        dcomm = fltarr(adsec.n_actuators)+ max_amp_pos
        dummy = fltarr(adsec.n_actuators)
        dummy[cl_list] = dcomm[cl_list]
        ;dummy[cl_list] = pist
        dummy = dummy[adsec.act_w_pos]
        
        err = update_status()
        if err ne adsec_error.ok then return,err

        cmd_curr = sys_status.offload_curr[adsec.act_w_curr]
        err = set_offload_force(cmd_curr, /SWITCH, /NOCHECK)
        if err ne adsec_error.ok then return,err
           
        err = set_offload_cmd(dummy, /SWITCH, /NOCHECK, /DELTA, /START )
        if err ne adsec_error.ok then return,err
        wait, (time_ps+2*rtr.oversampling_time+  t_icc) > 0.1

        cnt = 0
        dcount = getdiagnvalue(process_list.fastdiagn.msgd_name, 'ChDistRMS', 0, 671, buf)
        if dcount lt 0 then return, err

        val= buf.last
        while 1 do begin
                        
            pos0 = sys_status.position

   
            ;err = zero_ctrl_current2()
            ;if err ne adsec_error.ok then return, err
            ;wait, (2*rtr.oversampling_time+t_icc) > 0.01

            err = offload_icc()
            if err ne adsec_error.ok then return, err
            wait, (2*rtr.oversampling_time+t_icc) > 0.01

            ;err = zero_ctrl_current2()
            ;if err ne adsec_error.ok then return, err
            ;wait, (2*rtr.oversampling_time+t_icc) > 0.01

            err = update_status()
            if err ne adsec_error.ok then return, err
            if keyword_set(xadsec) then update_panels, /NO_READ, /OPT
            pos1=sys_status.position
            pos_diff = mean(double(abs(pos1[cl_list] - pos0[cl_list])))
            log_print, "Act not in position:  "+strtrim(string(total(pos_diff[cl_list] gt 5*val[cl_list])), 2)
            cond_pos_diff = total(pos_diff[cl_list] gt 5*val[cl_list]) eq 0
            if (cnt eq 2) or cond_pos_diff then break
            cnt += 1

        endwhile
        
        err = update_status()
        if err ne adsec_error.ok then return, err
        if keyword_set(xadsec) then update_panels, /NO_READ, /OPT
        log_print,"command/position: "+strtrim(mean(double(sys_status.command[adsec.act_w_cl]+dummy[adsec.act_w_cl])),2)+" " $
                  +strtrim(mean(double(sys_status.position[adsec.act_w_cl])), 2)
        ;if mean(sys_status.position[cl_list]) ge mean(final_pos[cl_list]) then break     
    endfor

    return, adsec_error.ok
end
