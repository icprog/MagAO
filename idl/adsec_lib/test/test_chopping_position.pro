;$Id: test_chopping_position.pro,v 1.6 2009/08/06 09:20:35 labot Exp $
;+
; NAME:
;   TEST_CHOPPING_POSITION
;   REDUCE_CHOPPING_POSITION
;
; PURPOSE:
;   Chopping performance test
;
; USAGE
;   err= test_chopping_position (amp, data_reduced, CMD2BIAS=cmd2bias, FILE=filename, NOSETTING=nosetting, CHOP=chop_amp, LOAD_CHOP=load_chop)
;   err = reduce_chopping_position (filename, do_jpeg=do_jpeg)
;
; INPUT:
;   amp:    tilt amplitude to apply to the mirror
;   filename: 
;
; OUTPUT:
;   err: error code
;   data_reduced: data output read by ADM
;
; KEYWORDS:
;   CMD2BIAS: set the actual command as bias
;   FILE: filename to save
;   NOSETTING: not set the preshaper parameters and the matrices in the ASM
;   CHOP: amplitude of chopping (peak to valley)
;   LOAD_CHOP: chop from initial position to two previous-saved status save positions
;
; NOTE:
;   The mirror proportional gain must be set BEFORE. (GAP: 70um ; P=0.09, D=0.25 on all actuators)
;   The preshapers are set by the routine and not reset to the previous state
;   Works only with oversampling frame activated.
; 
; EXAMPLE:
;   print, test_chopping_position(10e-6, data_red, CHOP=-40e-6) :
;   print, test_chopping_position(0e-6, /LOAD_CHOP) :
;
; HISTORY:
;   Created by Marco Xompero on 2008
;-

Function reduce_chopping_position, filename, do_jpeg=do_jpeg

     @adsec_common
     ncl = n_elements(adsec.act_w_cl)
     if n_elements(filename) eq 0 then filename = dialog_pickfile()
     print, filename
     restore, filename, /ver
     ad = status_save.adsec
     ss = status_save.sys_status
     if n_elements(old_ovs) eq 0 then old_ovs = 0.002
     if n_elements(mc) eq 0 then begin
         mc = fltarr(ad.n_actuators)
         mc[indgen(6)] = 1.
     endif
     modal_id = where(mc, nm)
     
     pos = transpose(data_reduced.dist_average)
     pos1= pos -rebin(pos[260,*], size(pos,/DIM), /SAMP)
     pos -= rebin(pos[0,*], size(pos,/DIM), /SAMP)
     curr = transpose(data_reduced.curr_average)
     mstep = transpose(ad.ff_p_svec) ## pos
     mstep1= (transpose(ad.ff_p_svec) ## pos1)/sqrt(ncl)

     time = data_reduced_sw.timestamp
     time -= time[0]
  
     minit_val = fltarr(nm)
     mfinal_val = fltarr(nm)
     mstep_val = fltarr(nm)
     mset_time = fltarr(nm)
     mrise_time = fltarr(nm)
     movershoot = fltarr(nm)
     movershoot_um = fltarr(nm)
     tmps      = fltarr(nm)
     tmprt      = fltarr(nm)
     max_id      = fltarr(nm)

     col = comp_colors(nm)
  
     for i=0, nm-1 do begin

         minit_val[i] = mean(mstep[0:100, modal_id[i]])
         mfinal_val[i] = mean(mstep[n_samples2restore-100: n_samples2restore, modal_id[i]])
         mstep_val[i] = abs(mfinal_val[i]-minit_val[i])
         ns = abs(mstep[*,modal_id[i]]-minit_val[i])/mstep_val[i]
         movershoot[i] = (max( abs(mstep[*,modal_id[i]]-minit_val[i]), id_max)/mstep_val[i]) -1.
         max_id[i] = id_max
         tmps[i] = (where(ns gt 0.01))[0]
         tmprt[i] =  (where(ns gt 0.95))[0]

      endfor

     ids = min(tmps) 
     
     xs = 1024
     ys = 768
;     window, /free, xs=xs, ys=ys
     ts = (time - time[ids])*1e3
;     xr = [-5,25]
;     plot, ts,/nodata, xs=17, xr=xr,ys=17, yr=[-0.1,1.3] $
;           ,  TITLE = 'Decomposed normalized step: MAX OS:'+string(max(movershoot), format='(f5.2)')+'[%]', XTITLE='[ms]'
;     oplot, xr, [1.,1.]*1.05, col=255L, /line
;     oplot, xr, [1.,1.]*0.95, col=255L, /line
;     oplot, xr, [1.,1.], col=255L, line=2
    

 ;COMMENTED TEMPORARY
;     for i=0, nm-1 do begin
;         ns = abs(mstep[*,modal_id[i]]-minit_val[i])/mstep_val[i]
;         mrise_time[i] = time[tmprt[i]-1]-time[ids-1]   ;-1 per il ritardo degli accumulatori
;         dummy = fltarr(1,adsec.n_actuators)
;         movershoot_um[i] = max((abs(pos[max_id[i],*]-ss.position))[ad.act_w_cl])

;     endfor


;     movershoot *= 100

;     print, "SET TIME from 0 to 95% step [ms]:"
;     print, mrise_time*1e3
;     print, "OVERSHOOT[%]:"
;     print, movershoot
;     print, movershoot_um
;END     

     ;tilt step response
     file_shell_ts3 ='/home/adopt/work/AO/releases/test/conf/adsec/commons/TS5/shell.txt'
     shell_ts3 = read_ascii_structure(file_shell_ts3)
     make_chopping_tilt, adsec.act_w_cl, tilt, force, mc
     ;RICOSTRUZIONE COMANDO TO DO
     ncl = n_elements(ad.act_w_cl) 
     tiltff = tilt/sqrt(total(tilt^2))   ;NORMALIZZATO come i modi di FF (somme quadrati unitaria che serve per proiettare corretamente in norma euclidea)
     tiltff_step  = transpose(tiltff) ## pos 
     residue = pos - tiltff ## tiltff_step 
     tilt_step = tiltff_step / sqrt(ncl)
     

     tilt_init_val = mean(tilt_step[0:100])
     tilt_final_val  = mean(tilt_step[n_samples2restore-100: n_samples2restore])
     tilt_step_val = abs(tilt_final_val-tilt_init_val)
     nt = abs(tilt_step-tilt_init_val)/tilt_step_val
     tilt_overshoot = (max( abs(tilt_step-tilt_init_val), id_max_tilt)/tilt_step_val) -1.
     idx_rt_tilt =  (where(nt gt 0.95))[0]
     ids = 1   ;TO BR FIXED
     tilt_rt = time[idx_rt_tilt-1]-time[ids-1]
     tilt_overshoot_um = max((abs(pos[id_max_tilt,*]-ss.position))[ad.act_w_cl])
     tilt_overshoot *=100






     ;window, /free, xs=xs, ys=ys
     ;vec = tilt_step*1e6/5.   ;arcsec on sky
     ;if max(vec) lt 2. then yr = [-23, 8]/5. else yr=[-8, 23]/5.
     ;plot, ts, vec, xs=17, xr=xr , yr= yr, ys=17  $
     ;      , TITLE='Zernike TILT step (ON SKY EDGE AMPLITUDE): SET-TIME:'+string(tilt_rt*1e3, format='(f5.2)')+'[ms], MAX OS:' $
     ;      +string((tilt_overshoot), format='(f5.2)')+'[%]', YTITLE='[arcsec]', XTITLE='[ms]'
     ;oplot, xr, ([1.,1.]*1e6*tilt_final_val+tilt_step_val*1e6*0.05)/5., col=255L, /line
     ;oplot, xr, ([1.,1.]*1e6*tilt_final_val-tilt_step_val*1e6*0.05)/5., col=255L, /line
     ;if keyword_set(do_jpeg) then write_jpeg, filename+'.zernike_tilt_step.jpg', tvrd(true=3), true=3, quality=1000
     ;print, "TILT settling time from 0 to 95% signal[ms]:", tilt_rt*1e3
     ;print, "TILT overshoot [%]", tilt_overshoot
     ;print, "TILT max overshoot [um]", tilt_overshoot_um
     ;print, "Coefficient stroke [um]", tilt_step_val*1e6


    ;RESIDUE DECOMPOSITION
    dec = (transpose(ad.ff_p_svec) ## residue) /sqrt(ncl)
    dec_nobias = dec-rebin(dec[0,*], n_samples2restore+1, ad.n_actuators)
    std_res = fltarr(ncl)
    deccol = comp_colors(ncl)
    ;for i=0, ncl-1 do std_res[i] = stddev(residue[i,ad.act_w_cl])
    window, /FREE, xs=xs, ys=ys

    for i=0, ncl-1 do oplot,ts, dec[*,i]*1e6, col=deccol[i]



;    xr = minmax(dec_nobias[*,0:ncl-1])
;    window, /FREE, xs=xs, ys=ys
;    plot, [0,n_samples2restore], xr, /NODATA, TITLE='MODAL DECOMPOSITION ALL HIST WITH NULL BIAS'
;    for i=0, ncl-1 do oplot, indgen(n_samples2restore+1), dec_nobias[*,i], COL = deccol[i]

    dec_nostep = dec-rebin(rebin(dec[n_samples2restore-100:*,*],1, ad.n_actuators), n_samples2restore+1, ad.n_actuators)
    window, /FREE, xs=xs, ys=ys
    plot, ts, dec_nostep[*,0], /NODATA, TITLE='MODAL DECOMPOSITION STEADY STATE', yr=3*[-1.0e-9, 1.0e-9], xr=[0,500] $
    , YTITLE='coeff [um] RMS', XTITLE='[ms]'
    for i=0, ncl-1 do oplot, ts, (dec_nostep[*,i]), COL = deccol[i]
     
    dec_step = dec-rebin(rebin(dec[0:100,*],1, ad.n_actuators), n_samples2restore+1, ad.n_actuators, /SAMP)
    window, /FREE, xs=xs, ys=ys
    plot, ts, dec_step[*,0], /NODATA, TITLE='MODAL DECOMPOSITION INITIAL STATE', yr=3*[-1.0e-9, 1.0e-9], xr=[-500, 0] $
    , YTITLE='coeff [um] RMS', XTITLE='[ms]'
    for i=0, ncl-1 do oplot, ts, (dec_step[*,i]), COL = deccol[i]
     
    std_res_end = fltarr(ncl)
    std_res_in = fltarr(ncl)
    for i=0, ncl-1 do std_res_end[i] = stddev(dec[600:1000,i])
    for i=0, ncl-1 do std_res_in[i] = stddev(dec[0:400,i])
    window, /FREE, xs=xs, ys=ys
    plot,indgen(ncl), std_res_in, TITLE='MODAL COEFFICIENT RMS', psym=-4, YTITLE='[um] RMS', XTITLE='MODE #'
    oplot, indgen(ncl), std_res_end, col=255L, psym=-4

    window, /FREE, xs=xs, ys=ys
    plot,indgen(ncl), std_res_in*1e6, TITLE='MODAL COEFFICIENT RMS (ZOOM START)', psym=-4 $
                    , xr=[0,50], YTITLE='[um] RMS', XTITLE='MODE #'
    oplot, indgen(ncl), std_res_end*1e6, col=255L, psym=-4

    window, /FREE, xs=xs, ys=ys
    plot,indgen(ncl), std_res_in*1e6, TITLE='MODAL COEFFICIENT RMS (ZOOM END)', psym=-4  $
                    , xr=[ncl-51,ncl-1], YTITLE='[um] RMS', XTITLE='MODE #', yr = [-5e-5, 5e-5]
    oplot, indgen(ncl), std_res_end*1e6, col=255L, psym=-4

    ;MODE PSD CALCULATION
    dd = [dec[0:400,*],dec[600:*,0]]
    psdm = fltarr(size(dd, /DIM))
    for i=0, ncl-1 do begin
        fft1, dd[*,i], old_ovs, PSD=dummy, FSPEC=fspec, /NO
        psdm[*,i] = dummy
    endfor
    
    

    stop
    return, 0

End


Pro make_report_plot

     names = file_search(meas_path('step')+'fast_step_70um_*p0.09d0.25_ps015ms.sav')
     set_plot_defaults, /WHITE
     for i=0, n_elements(names) -1 do err = reduce_chopping_position(names[i], /DO)
     set_plot_defaults
     return
end


Function test_chopping_position, amp, data_reduced, CMD2BIAS=cmd2bias, FILE=filename, NOSETTING=nosetting, CHOP=chop_amp, LOAD_CHOP=load_chop, DELTA=delta, NOCONFIGURE=noconfigure

    @adsec_common


    if n_elements(amp) eq 0 then amp = 0e-6
    
    if abs(amp) gt 40e-6 then begin
        message, "ERROR: too large stroke", /INFO
        return, -1
    endif

    if keyword_set(LOAD_CHOP) then begin
        default_file=adsec_path.conf+'scripts/4d_chop_default.txt'
        def=read_ascii_structure(default_file)
        file4d='$ADOPT_MEAS/adsec_calib/4d/'+def.folder4d+def.file4d
        file1=adsec_path.meas+def.chop_a
        file2=adsec_path.meas+def.chop_b

      ;  file2 = '/home/labot/idl/adsec672a/meas/flat/2009_04_02/flat_data_000_200_70um_chopA+25e-6_10deg.sav'
      ;  file1 = '/home/labot/idl/adsec672a/meas/flat/2009_04_02/flat_data_000_200_70um_chopA-25e-6_10deg.sav'
        restore, /ver, file1
        newpos1 = flattened_status.position
        restore, /ver, file2
        newpos2 = flattened_status.position
    endif

    err = update_status()

    timepcmd = 15e-3  ;max 231e-3
    timepcurr = 15e-3 ;max 231e-3
    acc_period = 1e-3 
    old_ovs = 0.00111
    if ~keyword_set(NOSETTING) then begin

        ;set up the modes/commands/forces thresholds
        err = set_safe_skip(ACT=adsec.act_w_cl, DELTACMD=50e-6, MAXCMD=135e-6)
        if err ne adsec_error.ok then return, err
   
        err = set_diagnostic(OVER=0.0)

       ;set up the dummy recostruction/delay/m2c matrices
        b0 = fltarr(rtr.n_slope,adsec.n_actuators)
        b_delay = fltarr(rtr.n_slope,adsec.n_actuators, rtr.n_slope_delay) ;(1 delay slopes)
        a_delay = fltarr(adsec.n_actuators,adsec.n_actuators,rtr.n_modes_delay) ;(2 delay modes)
        b0[0,0] = identity(adsec.n_actuators)
        old_m2c_mat = *rtr.m2c_matrix
        new_m2c_mat = fltarr(rtr.n_modes2correct, adsec.n_actuators)
        err = set_m2c_matrix(new_m2c_mat)
        if err ne adsec_error.ok then return, err


        cmd_preshaper = time2step_preshaper(timepcmd, APPLIED=cmd_preshaper_sec, /VERB)
        cur_preshaper = time2step_preshaper(timepcurr, APPLIED=cur_preshaper_sec, /VERB)

        preshaper_sec = max([cmd_preshaper_sec,cur_preshaper_sec])
        max_settling_time = preshaper_sec

        ;    acc_delay = ceil(max_settling_time/adsec.sampling_time)
        ;    acc_len   = long(acc_period/adsec.sampling_time-acc_delay)
        acc_delay = ceil(0.7e-3/adsec.sampling_time)
        acc_len   = long(acc_period/adsec.sampling_time-acc_delay)

        print, "ACC_DELAY: ", strtrim(acc_delay,2)," (",strtrim(acc_delay*adsec.sampling_time,2),"s)"
        print, "ACC_LEN  : ", strtrim(acc_len,2)," (",strtrim(acc_len*adsec.sampling_time,2),"s)"
        print, "MAX FREQ ALLOWED: ", strtrim( 1d/acc_period , 2)

        err = set_acc(acc_len,acc_delay)
        if err ne adsec_error.ok then return, err
        wait, 0.1
        err = start_acc(/WAIT)
        if err ne adsec_error.ok then return, err
        wait, 0.1

        pr_cmd = ((round(adsec.preshaper_len*adsec.sampling_time/timepcmd)  > 1) $
                  < dsp_const.fastest_preshaper_step)
        pr_cur = ((round(adsec.preshaper_len*adsec.sampling_time/timepcurr)  > 1) $
                  < dsp_const.fastest_preshaper_step)
        
        print, "PRESHAPER CMD: ", timepcmd, pr_cmd
        print, "PRESHAPER CURR" , timepcurr, pr_cur

        err = set_preshaper(sc.all, pr_cmd)
        if err ne adsec_error.ok then return, err
        err = set_preshaper(sc.all, pr_cur, /FF)
        if err ne adsec_error.ok then return, err
        
        g_gain = replicate(1.0, adsec.n_actuators)
        
        err = set_b0_matrix(b0)
        if err ne adsec_error.ok then message, "Error encountered: "+strtrim(err,2)
        
        err = set_b_delay_matrix(b_delay)
        if err ne adsec_error.ok then message, "Error encountered: "+strtrim(err,2)
        
        err = set_a_delay_matrix(a_delay)
        if err ne adsec_error.ok then message, "Error encountered: "+strtrim(err,2)
        
        err = set_g_gain(sc.all, g_gain)
        if err ne adsec_error.ok then message, "Error encountered: "+strtrim(err,2)
        
        err = set_diagnostic(OVER=old_ovs)
        if err ne adsec_error.ok then return, err
        
;    endif

;    if keyword_set(CMD2BIAS) then begin
        
;        err = zero_ctrl_current(/OFFLOAD)
        old_ovs = rtr.oversampling_time
        err = set_diagnostic(OVER=0.0)
        if err ne adsec_error.ok then return, err
        err = set_cmd2bias()
        if err ne adsec_error.ok then message, "Error encountered: "+strtrim(err,2)
        err = set_diagnostic(OVER=old_ovs)
        if err ne adsec_error.ok then return, err

    endif

 

    wait, 0.1

;    undefine, pbs, wfsc
;    slopes = fltarr(rtr.n_slope)
;    err = wfs_switch_step(slopes, WAIT=fulltime, WFSC=wfsc, PBS=pbs)
;    if err ne adsec_error.ok then return, err
;    wait, 0.1

    err = test_skip_frame(/dsp)
    if err ne adsec_error.ok then return, err

    make_chopping_tilt, adsec.act_w_cl, tilt, force, mc
    
    ;amp = 1. ;;;;; RMS TILT : PEAK 2*  ;PTV 4*
 
        cmd2apply = float(amp * tilt)
        null_cmd2apply = float(0.*tilt)
  

    

    
    if n_elements(chop_amp) gt 0 then begin

        if keyword_set(LOAD_CHOP) then begin
             cmd2apply = newpos1-sys_status.position
             chop_cmd2apply = newpos2 - newpos1
        endif else begin
            chop_cmd2apply = float(chop_amp*tilt)
        endelse
        window, 0, title='cmd2apply'
        display, cmd2apply[adsec.act_w_cl], adsec.act_w_cl,/sh
        window, 1, title='chop_cmd2apply'
        display, chop_cmd2apply[adsec.act_w_cl], adsec.act_w_cl, /sh
        window, 2, title='full_cmd'
        display, cmd2apply[adsec.act_w_cl]+sys_status.bias_command[adsec.act_w_cl], adsec.act_w_cl, /sh
        window, 3, title = 'full_chop_cmd'
        display, sys_status.position[adsec.act_w_cl]+chop_cmd2apply[adsec.act_w_cl]-cmd2apply[adsec.act_w_cl], adsec.act_w_cl, /sh
  
        chopwait=0.05


  ;      if abs(chop_amp) gt 40e-6 then begin 
  ;          message, "ERROR: TOO LARGE CHOP", /INFO
  ;          return, adsec_error.generic_error
  ;      endif
        print, "CHOPPING APPLICATION *********************************************************************"
        wait, 0.8
        err = set_offload_cmd(cmd2apply[adsec.act_w_pos], /START, /NOCHECK)
        if err ne adsec_error.ok then return, err
        wait, chopwait
        for i=0, 10 do begin
            err = set_offload_cmd(chop_cmd2apply[adsec.act_w_pos], /START, /NOCHECK)
            if err ne adsec_error.ok then return, err
            wait, chopwait
            err = set_offload_cmd(cmd2apply[adsec.act_w_pos], /START, /NOCHECK)
            if err ne adsec_error.ok then return, err
            wait, chopwait
        endfor
        err = set_offload_cmd(null_cmd2apply[adsec.act_w_pos], /START, /NOCHECK)
        if err ne adsec_error.ok then return, err

        n_samples2restore = 1000
    endif else begin

        print, "TILT APPLICATION"
        err = set_offload_cmd(cmd2apply[adsec.act_w_pos], /START, /NOCHECK, DELTA=delta)
        if err ne adsec_error.ok then return, err

        err = test_skip_frame(/dsp)
        if err ne adsec_error.ok then return, err
        wait, 0.45
        nsamples2restore=2000

    endelse


    if keyword_set(FILENAME) then begin
        wait, timepcmd+0.5

        err = set_diagnostic(over=0.0)
        if err ne adsec_error.ok then return, err
        ;   err = set_disturb(/DISABLE)    
        ;    if err ne adsec_error.ok then return, err
        err = dspstorage(/DISABLE)
        if err ne adsec_error.ok then return, err

        err = load_diag_data_switch(n_samples2restore+1, data_reduced_sw, /NOFASTCHECK, /READFC, XADSEC=xadsec)
        if err ne adsec_error.ok then return, err
        err = set_diagnostic(over=old_ovs)
        if err ne adsec_error.ok then return, err
        err = load_diag_data_dsp(n_samples2restore+1, data_reduced, /NOFASTCHECK, /READFC, XADSEC=xadsec)
        if err ne adsec_error.ok then return, err
        err = set_diagnostic(over=0.0)
        if err ne adsec_error.ok then return, err

        err = dspstorage()
        if err ne adsec_error.ok then return, err

        err = set_diagnostic(over=old_ovs)
        if err ne adsec_error.ok then return, err

 ;   ans = dialog_message("All ok??", /QUESTION)
;    if ans ne "Yes" then begin
;        err = set_offload_cmd(cmd2apply[adsec.act_w_pos], /START, /NOCHECK)
;    endif

        err = get_status(status_save)
        if err ne adsec_error.ok then return, err
        save, file=meas_path('step')+'fast_step_chop_'+filename, status_save, data_reduced, n_samples2restore, data_reduced_sw, old_ovs, mc
    endif

    return, adsec_error.ok
end


