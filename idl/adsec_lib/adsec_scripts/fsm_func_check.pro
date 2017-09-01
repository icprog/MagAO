Function func_dust_test
    @adsec_common
    default_dir = filepath(ROOT=adsec_path.commons, sub=['ref_wave'], "")
    std_fn   = default_dir+"wave.txt"
    close_fn = default_dir+"wave_1o8.txt"
    log_print, 'Reference wave paramenters initialization...'
    if ~(file_info(std_fn)).exists then begin
        log_print, "Standard capacitive sensor reference wave file not exists."
        return, adsec_error.IDL_INVALID_FILE
    endif
            
    if ~(file_info(close_fn)).exists then begin
        log_print, "Reduced capacitive sensor reference wave file not exists."
        return, adsec_error.IDL_INVALID_FILE
    endif


    ;RELOAD REFERENCE WAVE
    err = disable_watchdog(/DSP)
    if err ne adsec_error.ok then return, err
    err = init_adsec_wave(close_fn)
;    err = init_adsec_wave(std_fn, /VER)
    if err ne adsec_error.ok then return, err
    err = send_wave()
    if err ne adsec_error.ok then return, err
    err =  init_adsec_cals()
    if err ne adsec_error.ok then return, err
    err = send_linearization()
    if err ne adsec_error.ok then return, err
    err = enable_watchdog(/DSP)
    if err ne adsec_error.ok then return, err

   ;FIND DUST GRAINS
    err = rip()
    if err ne adsec_error.ok then return, err
    err = adam_enable_coils()
    if err ne adsec_error.ok then return, err
    err = find_touch(add_timestamp('dust_check'),delta_pos, idx_good, POS0=pos0, POS1=pos1, /SAVE)
    if err ne adsec_error.ok then return, err
    err = adam_disable_coils()
    if err ne adsec_error.ok then return, err
    err = rip()
    if err ne adsec_error.ok then return, err

    ;TODO: ricontrolla bene questo conto perche' non torna....
    act_list = idx_good
    adc_bits = adsec.adc_bits
    vratio = float(adsec.capsens_vmax/adsec.capsens_vref)
    dist_A_coeff = float(adsec.alpha_pos[act_list]) * float(2d0^adc_bits/vratio*adsec.d0[act_list]) ;[m*adcbit]
    dist_B_coeff = float(-(adsec.c_ratio[act_list]*2d0^adc_bits/vratio)) ;[adcbit]
    min_dist = dist_A_coeff/( (2d0^adc_bits-1)+ dist_B_coeff)

    ;RELOAD REFERENCE WAVE
    err = disable_watchdog(/DSP)
    if err ne adsec_error.ok then return, err
    err = init_adsec_wave(std_fn)
    if err ne adsec_error.ok then return, err
    err = send_wave()
    if err ne adsec_error.ok then return, err
    err =  init_adsec_cals()
    if err ne adsec_error.ok then return, err
    err = send_linearization()
    if err ne adsec_error.ok then return, err
    err = enable_watchdog(/DSP)
    if err ne adsec_error.ok then return, err

    not_moved = where(abs(delta_pos[idx_good]) lt 20e-9 and delta_pos[idx_good] gt 0)

    if not_moved[0] ne -1 then begin
        dust_size = max(abs(pos0[idx_good[not_moved]]))
        log_print, "Possible dust contamination of the gap aroud actuators DSP ID: " + strjoin(string(not_moved, format='(" #", I3)'))
        log_print, "Maximum dust size: " + string(dust_size) + ' [m]' 
        if dust_size gt 40e-6 then return, adsec_error.IDL_DUST_CONTAMINATION else return, adsec_error.ok
    endif else begin
        log_print, "No dust contamination of the gap detected."
        return, adsec_error.ok
    endelse

End

Function func_coils_test

    @adsec_common
    delta_curr = adsec.weight_curr 

    err = rip()
    if err ne adsec_error.ok then return, err

    err = update_status()
    if err ne adsec_error.ok then return, err

    err = adam_enable_coils()
    if err ne adsec_error.ok then return, err

    ;CHECK COILS WITH SPI CURRENT
    err = write_bias_curr(sc.all, delta_curr)
    print, delta_curr
    if err ne adsec_error.ok then return, err

    nsample =180
    spi = fltarr(nsample, adsec.n_actuators)    
    time_data = time_estimator(nsample, /OPTIMIZE, /RESET)
    old_cnt = 0
    i = 0
    while i lt nsample do begin
        err= getdiagnvalue(process_list.housekeeper.msgd_name,  'DiagnAppFastVarsCounter', -1, -1, tim=1, cntbb)
        cnt = cntbb.last
        if cnt ne old_cnt then begin 
            err= getdiagnvalue(process_list.housekeeper.msgd_name,  '*coil*', -1, -1, tim=1, tmp) 
            spi[i, *] = tmp.last
            time_str = strtrim(string(time_estimator(i,time_data, unit=unit_str)),2)
            log_print, "Time remaining: "+time_str
            old_cnt = cnt
            i+=1
        endif else begin
            wait, 0.5
        endelse
            
    endwhile

    err = adam_disable_coils()
    if err ne adsec_error.ok then return, err

    err = rip()
    if err ne adsec_error.ok then return, err

    spi_m = (-1) * rebin(spi, 1, adsec.n_actuators)
    spi_sd = spi_m*0
    for i = 0, adsec.n_actuators-1 do spi_sd[i] = stddev(spi[*,i])
    not_work  = where( abs((spi_m)-adsec.weight_curr) gt adsec.weight_curr/3., cc)
    if cc eq 0 then return, adsec_error.ok $
    else begin
        log_print, "Coils current test failed for actuators DSP ID: "+strjoin(string(not_work, format='(" #", I3)'))
        return, adsec_error.ok
    endelse

End

Function func_dist_th_test

    @adsec_common
    
    err = read_seq_dsp(sc.all, dsp_map.pos_check_internal_cnt, 1l, scnt, /UL)
    if err ne adsec_error.ok then return, err
    err = read_seq_dsp(sc.all, dsp_map.pos_check_internal_cnt, 1l, bcnt, /UL)
    if err ne adsec_error.ok then return, err
    id = where(scnt-bcnt eq 0)
    if id[0] ne -1 then begin
        log_print, "Failed on "+strjoin(string(id, format='(" #", I3)'))
    endif else begin
        log_print, "All ok."
    endelse
    return, adsec_error.ok
    

End


Function func_wd_test
    
    @adsec_common
    err = getdiagnvalue(process_list.housekeeper.msgd_name,  '*WatchdogExpired*', -1,-1, wd)
    if err ne adsec_error.ok then return, err
    val = wd.last
    nval = [transpose(val[0:(adsec.n_board_per_bcu*adsec.n_crates*adsec.n_dsp_per_board)/2-1]), $
           transpose(val[(adsec.n_board_per_bcu*adsec.n_crates*adsec.n_dsp_per_board)/2:*])]
    nval = reform(nval, adsec.n_board_per_bcu*adsec.n_crates*adsec.n_dsp_per_board)

    wd_fault = where(nval, cc)
    if cc gt 0 then begin
        log_print, "Watchdog expired for DSP ID: "+strjoin(string(wd_fault , format='(" #", I3)'))
        return, adsec_error.generic_error
    endif else begin
        return, adsec_error.ok
    endelse
    
End

Function func_ps_test       ;-------------------------   TESTED

    @adsec_common
    err = adam_update()
    if err ne adsec_error.ok then return, err
    mp   = adam_out.main_power
    tss0 = adam_in.tss_ac_power_fault_n0
    tss1 = adam_in.tss_ac_power_fault_n1
    tssf = adam_in.tss_fault_n
    tssd = adam_out.tss_disable
    tcs0 = adam_in.tcs_ac_power_fault_n0
    tcs1 = adam_in.tcs_ac_power_fault_n1
    tcs2 = adam_in.tcs_ac_power_fault_n2

    tss = tssd or tssf

    if (mp and tss0 and tss1 and tcs0 and tcs1 and tcs2 and tss) then  return, adsec_error.ok $ 
    else begin

        status = ['Fault', 'Ok']
        log_print, 'Main power '+status[mp]
        log_print, 'TSS AC #0  '+status[tss0]
        log_print, 'TSS AC #1  '+status[tss1]
        log_print, 'TSS FAULT  '+status[tssf]
        log_print, 'TCS AC #0  '+status[tcs0]
        log_print, 'TCS AC #1  '+status[tcs1]
        log_print, 'TCS AC #2  '+status[tcs2]

        return, adsec_error.generic_error
    endelse
End

Function func_fiber_test   ;------------------ TESTED

    @adsec_common
    cnt = 0
    while 1 do begin

        err = fastlink_alignment(/VER, TEST_TIME=30)
        if err eq adsec_error.ok then break 

        cnt +=1
        if cnt eq 5 then begin
            log_print, "Maximum iteration passed."
            return, adsec_error.generic_error
        endif
        err = adam_sys_reset()
        if err ne adsec_error.ok then return, err
        wait, 10.

    endwhile

    return, adsec_error.ok

End

Function func_capsens_test   ; ------ TESTED

    @adsec_common
    err = update_status()
    if err ne adsec_error.ok then return, err
    act_list = adsec.act_w_cl
    adc_bits = adsec.adc_bits
    vratio = adsec.capsens_vmax/adsec.capsens_vref
    dist_A_coeff = float(adsec.alpha_pos[act_list]) * float(2d0^adc_bits/vratio*adsec.d0[act_list]) ;[m*adcbit]
    dist_B_coeff = float(-(adsec.c_ratio[act_list]*2d0^adc_bits/vratio)) ;[adcbit]
    min_dist = dist_A_coeff/( (2d0^adc_bits-1)+ dist_B_coeff)
    bad_act = where(sys_status.position[act_list] gt 2*min_dist[act_list])
    if bad_act[0] eq -1 then begin
        return, adsec_error.ok
    endif else begin
        log_print, "Bad capacitive sensor detected: electronic fault or large dust inside the gap"
        log_print, "Actuators DSP ID:# "+strjoin(string(adsec.act_w_cl[bad_act], format='(" ",I3.3)'))
        return, adsec_error.generic_error
    endelse
    return, adsec_error.ok
 

End


Function func_spi_test, SHOW=show

    @adsec_common
    nsample =60
    spi = fltarr(nsample, adsec.n_actuators)    
    time_data = time_estimator(nsample, /OPTIMIZE, /RESET)
    old_cnt = 0
    i = 0
    while i lt nsample do begin
        err= getdiagnvalue(process_list.housekeeper.msgd_name,  'DiagnAppFastVarsCounter', -1, -1, tim=1, cntbb)
        cnt = cntbb.last
        if cnt ne old_cnt then begin 
            err= getdiagnvalue(process_list.housekeeper.msgd_name,  '*coil*', -1, -1, tim=1, tmp) 
            spi[i, *] = tmp.last
            time_str = strtrim(string(time_estimator(i,time_data, unit=unit_str)),2)
            log_print, "Time remaining: "+time_str
            old_cnt = cnt
            i+=1
        endif else begin
            wait, 0.5
        endelse
            
    endwhile

    spi_m = (-1) * rebin(spi, 1, adsec.n_actuators)
    spi_sd = spi_m*0
    for i = 0, adsec.n_actuators-1 do spi_sd[i] = stddev(spi[*,i])
    not_work  = where( abs((spi_m)) gt adsec.weight_curr/3., cc)
    if keyword_set(SHOW) then begin

        window, 0
        display, spi_m, /sh, /as, TITLE="MEAN of SPI CURRENTS", /NO_N

        window, 1
        display, spi_sd, /sh, /as, TITLE="STD of SPI CURRENTS", /NO_N

    endif
    if cc eq 0 then return, adsec_error.ok $
    else begin
        log_print, "Coils current test failed for actuators DSP ID: "+strjoin(string(not_work, format='(" #", I3)'))
        return, adsec_error.generic_error
    endelse

End



















Function fsm_func_check;, AUTO=auto


    @adsec_common
    if sc.name_comm eq "Dummy" then return, adsec_error.ok
    err_cnt  = 0
    sc.debug = 0
    
    ;config_diagnostics

    catch, Error_status

    ;This statement begins the error handler:  
    if Error_status ne 0 then begin

        err_cnt += 1
        print, err_cnt

        if err_cnt gt 1 then begin
            log_print, "RECURSIVE ERROR."
            log_print, "PANIC: NO ACTION CAN BE DONE!!!!!!"
            return, !AO_ERROR
        endif
        err1 = !AO_ERROR

        log_print, !ERROR_STATE.MSG + " " +strtrim(!AO_ERROR, 2)

        err2 = adam_sys_reset()
        
        print, "system reset"
        err2 = 0
        if err2 ne adsec_error.ok then begin
            log_print, "PANIC: NO ACTION CAN BE DONE!!!!!!"
            return, err2
        endif
        wait, 20.
        log_print, "System reset  after power on failure."
        return, err1
        
    endif

    err = func_ps_test()
    if err ne adsec_error.ok then log_print, "Power supply test failed with error # " + string(err)
    
    err = func_fiber_test()
    if err ne adsec_error.ok then log_print, "Fiber connection test failed with error # "+string(err)

    err = proc_startup(/NO_ASK, /NOFL)
    if err ne adsec_error.ok then begin 
        log_print, "Dsp program download failed with error # "+string(err)
        log_print, "No more tests can be done"
        return, err
    endif else begin

        err = func_wd_test()
        if err ne adsec_error.ok then log_print, "Watchdog DSP expired at least in one board with error # "+string(err)

        err = func_capsens_test()
        if err ne adsec_error.ok then log_print, "Capacitive sensor test failed with error # "+string(err)

        err = func_coils_test()
        if err ne adsec_error.ok then log_print, "Coil test failed with error # "+string(err)

        err = func_dust_test()
        if err ne adsec_error.ok then log_print, "Dust test failed with error # "+string(err)

    endelse

    return, adsec_error.ok
        
    
End

