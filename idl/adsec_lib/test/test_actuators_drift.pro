; Added the measure of temperature by the PT sensors.
; Added the possibility to acquire images from the interferometer.
; When an error occur or the test is finished a mail is sent.
Pro test_actuators_drift
    @adsec_common
    sc.debug=0
    save_temp=0 ; if set 1 save temperature from PT100

    mem_frames = 0 ; if set 1 save frame from interferometer
    th_bcu = 60
    th_sig = 55
    th_dsp = 55

    boolean = intarr(9)
    path_dir = 'drift/2006_10_31/'
    ext = '_a'  
    filename = adsec_path.meas+path_dir+'actuator_drift'+ext+'.sav'
    filen = adsec_path.meas+path_dir+'actuator_drift'+ext
    dir_drift = adsec_path.meas+path_dir
    comment_to_add = "Set 68um, nuova elettronica,meas on act 32."
    
;============================================================================
    adsec_save = adsec
    save, adsec_save, FILE=filepath(ROOT=dir_drift, "adsec_save"+ext+".sav")

;============================================================================


    delay = 10  ;sampling time in secs
    nmeas = 360;1800;5100;6000;8000;sample number 
    
    time2acq=60.*60./delay
    if save_temp then begin 
        com_data = {ip_address:'193.206.155.41', port:3008}
        err = fp_open(com_data, config, port=port, /ver)
        

    print, 'Test drift on actuators'
    if err ne 0 then message, 'Field point not opened. Data acquisition not started.'
    endif
;############################################
    ;data comunication for interferometer
    interf_type = "wyko"
    radix = "drift"
    
    root_wyko = "\\wyko4100\flat"
    root = filepath(ROOT=adsec_path.temp,SUB=["flat"],"")

    ip_addr = '193.206.155.41'
    port_wyko = 3006
    com_wyko = { $
               ip_address: ip_addr, $
               port:       port_wyko}

    samples =1024
    n_frames = 10
    wcount = 0
;#####################################################

    day=systime()
    print, day
    save, day, file=filen+'_day.sav'
    template = {                     $

        h2o_in      : fltarr(nmeas) ,$      ;cooling in
        h2o_out     : fltarr(nmeas) ,$      ;cooling out
        h2o_in_cp   : fltarr(nmeas) ,$      ;cooling in backplate
        h2o_out_cp  : fltarr(nmeas) ,$      ;cooling out backplate
        
        ;pipe_Al     : fltarr(nmeas) ,$      ;central pipe
        rp_32       : fltarr(nmeas) ,$      ;cold plate on act 22 (Good)
        cp_22       : fltarr(nmeas) ,$      ;cold plate on act 22 (BAD)
        rp_22       : fltarr(nmeas) ,$      ;reference plate on act 22 (BAD)
        ;cp_45       : fltarr(nmeas) ,$      ;cold plate on act 45 (GOOD)
        ;rp_45       : fltarr(nmeas) ,$      ;reference plate on act 45 (GOOD)
        ;power_bp    : fltarr(nmeas) ,$      ;power backplane
        t_env       : fltarr(nmeas) ,$      ;enviroment temperature
        rh          : fltarr(nmeas) ,$      ;relative humidity out voltage
        rh_true     : fltarr(nmeas)  $      ;relative humidity supply voltage

    }

    local_pos = fltarr(adsec.n_actuators, nmeas)
    local_curr = fltarr(adsec.n_actuators, nmeas)
    means = fltarr(adsec.n_actuators)
    rms = fltarr(adsec.n_actuators)
    err = get_slow_diagnostic(cc, /empty)
    deg = replicate(cc, nmeas)
    time=dblarr(nmeas)

    ;moduli array
    mod_temp = fltarr(8,3)
    ;BCU diagnostic single data
    openw, punit, filen+'_local_pos.dat', /GET_LUN
    openw, cunit, filen+'_local_curr.dat', /GET_LUN
    openw, dunit, filen+'_deg.dat', /GET_LUN
    openw, tunit, filen+'_time.dat', /GET_LUN

    if save_temp then begin
    ;Field Point single data
    openw, au, filen+'_h2o_in.dat', /GET_LUN
    openw, bu, filen+'_h20_out.dat', /GET_LUN
    openw, cu, filen+'_h20_in_cp.dat', /GET_LUN
    openw, du, filen+'_h20_out_cp.dat', /GET_LUN
    openw, eu, filen+'_rp_32.dat', /GET_LUN
    openw, gu, filen+'_rp_22.dat', /GET_LUN
    openw, fu, filen+'_cp_22.dat', /GET_LUN
    ;openw, eu, filen+'_pipe_Al.dat', /GET_LUN
    ;openw, fu, filen+'_cp_22.dat', /GET_LUN
    ;openw, gu, filen+'_rp_22.dat', /GET_LUN
    ;openw, hu, filen+'_cp_45.dat', /GET_LUN
    ;openw, iu, filen+'_rp_45.dat', /GET_LUN
    ;openw, lu, filen+'_power_bp.dat', /GET_LUN
    openw, mu, filen+'_t_env.dat', /GET_LUN
    openw, nu, filen+'_rh_out.dat', /GET_LUN
    openw, ou, filen+'_rh_sup.dat', /GET_LUN
    endif   
   ; path_file=adsec_path.meas+'drift/2005_10_07/test_drift.txt'
;    openw, unit, path_file, /GET_LUN
;    printf, unit, '-1'
;    free_lun, unit

    for i=0, nmeas-1 do begin

        print, 'Meas ', i
        
;lettura file testo
       ; openr, unit, path_file, /GET_LUN
;        readf, unit, meas_read
;        free_lun, unit
;verifica file testo???
        time[i]=systime(/sec)
        err=update_status()
        if err ne adsec_error.ok then begin
            err = rip()
            if err ne adsec_error.ok then spawn, '../adsec_lib/adsec_wrapper/sysdown'
            err = disable_coils()
            if err ne adsec_error.ok then spawn, '../adsec_lib/adsec_wrapper/sysdown'
            err = clear_dacs(adsec.act_w_cl)
            spawn, '../adsec_lib/adsec_wrapper/sysdown'
            save, file=filename, local_pos, i, deg, local_curr,template,day,time
            spawn, adsec_path.home+"/sendmail.py"
            free_lun, port
            return
        endif
        update_panels

        local_pos[*,i] = sys_status.position
        local_curr[*,i] = sys_status.current
        
        if  total(abs(local_curr[*,i]) gt 0.2) gt 0 then begin
            err = rip()
            if err ne adsec_error.ok then spawn, '../adsec_lib/adsec_wrapper/sysdown'
            err = disable_coils()
            if err ne adsec_error.ok then spawn, '../adsec_lib/adsec_wrapper/sysdown'
            err = clear_dacs(adsec.act_w_cl)
            spawn, '../adsec_lib/adsec_wrapper/sysdown'
            print, 'Troppa corrente!'
            save, file=filename, local_pos, i, deg, local_curr,template,day,time
            free_lun, port
            spawn, adsec_path.home+"/sendmail.py"
            return
        endif

        if  total(abs(local_pos[adsec.act_w_cl,i]) gt 100e-6 or (local_pos[adsec.act_w_cl,i]) lt 40e-6  ) gt 0 then begin
            err = rip()
            if err ne adsec_error.ok then spawn, '../adsec_lib/adsec_wrapper/sysdown'
            err = disable_coils()
            if err ne adsec_error.ok then spawn, '../adsec_lib/adsec_wrapper/sysdown'
            err = clear_dacs(adsec.act_w_cl)
            spawn, '../adsec_lib/adsec_wrapper/sysdown'
            print, 'Troppo drift posizione!'
            save, file=filename, local_pos, i, deg, local_curr,time, day,template
            spawn, adsec_path.home+"/sendmail.py"
            
            free_lun, port
            return
        endif
        

        
        
        err = get_slow_diagnostic(gd)
        if err ne adsec_error.ok then begin
          
            err = rip()
            if err ne adsec_error.ok then spawn, '../adsec_lib/adsec_wrapper/sysdown'
            err = disable_coils()
            if err ne adsec_error.ok then spawn, '../adsec_lib/adsec_wrapper/sysdown'
            err = clear_dacs(adsec.act_w_cl)
            print, 'Errore comunicazione!'
            spawn, '../adsec_lib/adsec_wrapper/sysdown'
            save, file=filename, local_pos, i, deg, local_curr,time, template, day
            spawn, adsec_path.home+"/sendmail.py"
            free_lun, port
            return
        endif

        deg[i] = gd
        boolean[0] = total(gd.bcu.stratix_temp gt th_bcu) 
        boolean[1] = total(gd.bcu.power_temp gt th_bcu)
        boolean[2] = total(gd.siggen.stratix_temp gt th_sig) 
        boolean[3] = total(gd.siggen.power_temp gt th_sig)
        ;boolean[4] = total(gd.siggen.dsps_temp gt th_sig) 
        boolean[5] = total(gd.dsp.stratix_temp gt th_dsp) 
        boolean[6] = total(gd.dsp.power_temp gt th_dsp)
        boolean[7] = total(gd.dsp.dsps_temp gt th_dsp) 
        boolean[8] = total(gd.dsp.driver_temp gt th_dsp)
        
        print, 'Troppo caldi ', boolean
        
        
        if total(boolean) gt 0 then begin
            err = rip()
            if err ne adsec_error.ok then spawn, '../adsec_lib/adsec_wrapper/sysdown'
            err = disable_coils()
            if err ne adsec_error.ok then spawn, '../adsec_lib/adsec_wrapper/sysdown'
            err = clear_dacs(adsec.act_w_cl)
            spawn, '../adsec_lib/adsec_wrapper/sysdown'
            print, 'Troppo Caldo!'
            save, file=filename, local_pos, i, deg, boolean, local_curr,time,template, day
            spawn, adsec_path.home+"/sendmail.py"
            
            free_lun, port
            return
        endif
        
        if mem_frames then begin
        ;acquisizioni sull'interferometro
            if ((i mod time2acq) eq 0) then begin
               name_radix = radix+strtrim(wcount,1)
               err = save_mwyko(com_wyko, name_radix, n_frames, ROOT_DIR=root, ROOT_WYKO=root_wyko $
                       , TYPE=interf_type, FILE_LIST=inter0)
               if err ne 0 then message, 'Frames not acquired.'
               wcount = wcount+1 
             endif 
        endif
        
        if save_temp then begin 
        mod_temp [*,0] = fp_read(config, 0, indgen(8), port=port)
        mod_temp [*,1] = fp_read(config, 1, indgen(8), port=port)
       ; mod_temp[*,2] = fp_read(config, 2, indgen(8), port=port)

        
        template.h2o_in[i] = mod_temp[0,0]
        template.h2o_out[i] = mod_temp[1,0]
        template.h2o_in_cp[i] = mod_temp[2,0] 
        template.h2o_out_cp[i] = mod_temp[3,0]
        template.rp_32[i] =  mod_temp[4,0]
        template.rp_22[i] =  mod_temp[5,0]
        template.cp_22[i] =  mod_temp[6,0]
        t_env = mod_temp[7,0]
        ;template.pipe_Al[i] =  mod_temp[4,0]
        ;template.cp_22[i] =  mod_temp[0,1]
        ;template.rp_22[i] =  mod_temp[1,1]
        ;template.cp_45[i] =  mod_temp[2,1]
        ;template.rp_45[i] =  mod_temp[3,1]
        ;template.power_bp[i] =  mod_temp[4,1]
        ;t_env = mod_temp[7,1]
        template.t_env[i] =  t_env
        
        ;rh_out =  mod_temp[0,2]
        ;rh_sup =  mod_temp[7,2]
        rh_out =  mod_temp[0,1]
        rh_sup =  mod_temp[7,1]
        rh  = (rh_out/rh_sup - 0.16)/0.0062
        
        template.rh[i] = rh
        template.rh_true[i] = rh/(1.0546-0.00216*t_env)
        endif
    ;scrittura file di testo
       ; openw, unit, path_file, /GET_LUN
;        printf, unit, i
;        free_lun, unit
        writeu, punit, sys_status.position
        flush, punit
        writeu, cunit, sys_status.current
        flush, cunit
        writeu, dunit, gd
        flush, dunit
        writeu, tunit, time[i]
        flush, tunit
        if save_temp then begin
        writeu, au, template.h2o_in[i]
        flush, au
        writeu, bu, template.h2o_out[i]
        flush, bu
        writeu, cu,template.h2o_in_cp[i]
        flush, cu
        writeu, du, template.h2o_out_cp[i]
        flush, du
        ; writeu, eu, template.pipe_Al[i]
        ;flush, eu
        writeu, eu, template.rp_32[i]
        flush, eu
        writeu, fu, template.cp_22[i]
        flush, fu
        writeu, gu, template.rp_22[i]
        flush, gu
        ;writeu  , hu, template.cp_45[i]
;        flush, hu
;        writeu , iu, template.rp_45[i]
;        flush, iu
;        writeu, lu, template.power_bp[i]
;        flush, lu
        writeu, mu, template.t_env[i]
        flush, mu
        writeu, nu, rh_out
        flush, nu
        writeu, ou, rh_sup
        flush, ou
        endif
        ;file di controllo********* 
        day=systime()
        openw, pippo, dir_drift+'controllo_drift.txt', /GET_LUN
        printf, pippo, day
        printf, pippo, "Delay [s]"+strtrim(delay,2)
        printf, pippo, "Meas "+strtrim(i, 2)+" of "+strtrim(nmeas,2)
        printf, pippo, "Positions:"
        printf, pippo, local_pos[*,i]
        printf, pippo, "Currents:"
        printf, pippo, local_curr[*,i]
        printf, pippo, comment_to_add
        
        free_lun, pippo
        ;*************************
        wait, delay
    endfor


    for i = 0, adsec.n_actuators-1 do begin
        means[i] = mean(local_pos[i,*])
        rms[i] = stddev(local_pos[i,*])
    endfor
   ; stop
    err = rip()
    if err ne adsec_error.ok then spawn, '../adsec_lib/adsec_wrapper/sysdown'
    err = disable_coils()
    if err ne adsec_error.ok then spawn, '../adsec_lib/adsec_wrapper/sysdown'
    err = clear_dacs(adsec.act_w_cl)
    spawn, '../adsec_lib/adsec_wrapper/sysdown'

    save, file=filename, local_pos, means, rms,  deg, local_curr,template,time, day

    


    free_lun, punit
    free_lun, cunit
    free_lun, dunit
    free_lun, tunit
    if save_temp then begin
    free_lun, port
    free_lun, au
    free_lun, bu
    free_lun, cu
    free_lun, du
    free_lun, eu
    free_lun, fu
    free_lun, gu
    ;free_lun, hu
    ;free_lun, iu
    ;free_lun, lu
    free_lun, mu
    free_lun, nu
    endif

    window, 0, retain=2
    display, means, title='Medie [m]', /sh
    window, 1, retain=2
    display, rms, title='Rms [m]',/sh

    max_dr=max(local_pos-rebin(local_pos[*,0],adsec.n_actuators,nmeas))
    min_dr=min(local_pos-rebin(local_pos[*,0],adsec.n_actuators,nmeas))
    
    window, 2, retain=2
    plot, [0,nmeas+1],[min_dr,max_dr]*1e9, /nodata
    for i=0, adsec.n_actuators-1 do oplot, (local_pos[i,*]-local_pos[i,0])*1e9 
    window, 3, retain=2
    display, (local_pos[*,nmeas-1]-local_pos[*,0])*1e9, title='delta end-begin[nm]',/sh

    spawn, adsec_path.home+"/endmail.py"
    return

    ;for i=0, adsec.n_actuators-1 do begin & plot, (local_pos[i,*]-local_pos[i,0])*1e6,title="act"+strtrim(i) & wait, 1 & endfor
End
