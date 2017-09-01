function mount_shell, TIME_PRE=time_pre

 
    
    @adsec_common
    set_plot, 'x'
    xadsec
    if rtr.oversampling_time gt 0 then begin
        log_print, "ERROR: the oversampling has to  be disabled in order to mount/unmount the shell."
        log_print, "NOTHING DONE!!"
        return, adsec_error.generic_error
    endif
    
    if !D.name ne 'X' then begin
        log_print, "ERROR: not allowed mount/unmount shell in RPC mode or ZBuffer mode. "
        log_print, "NOTHING DONE!!"
        return, adsec_error.generic_error
    endif
    
    log_print, "Start procedure for install a shell."
    default_file=adsec_path.conf+'configuration.txt'
    def=read_ascii_structure(default_file)
    shell2dism=def.shell
    ans0=dialog_message(['Procedure for the intallation of shell '+shell2dism,'Do you want to continue?'],  /quest)
    if strlowcase(ans0) eq "no"  then return, adsec_error.generic_error

    log_print, "Please verify the checklist:"
    log_print, "LED on ADSEC visible:"
    log_print, "Disable oversampling frames:"
    err = set_diagnostic(OVER=0.0)
    if err ne adsec_error.ok then return, err else log_print, "Ok"

    log_print, "Disable coils:"
    err = adam_disable_coils()
    if err ne adsec_error.ok then return, err else log_print, "Ok"

    log_print, "Change firmware threshold on capsens distance:"
    err = set_dist_threshold(sc.all, 4.,1ul)
    if err ne adsec_error.ok then return, err else log_print, "Ok"
    
    tps = 0.01
    pr_curr = ((round(adsec.preshaper_len*adsec.sampling_time/tps))  > 1)

    err=set_preshaper(sc.all_actuators,pr_curr,/FF)
    if err ne adsec_error.ok then return, err

    time_ps =  step2time_preshaper(pr_curr) ; for DEBUG
    if n_elements(time_ps) gt 1 then time_ps=max(time_ps) > 0.1
    n_act = n_elements(adsec.true_act)
    meanEff=0.576; % N/sqrt(W)
    coilRes=6.7;
    shellWeight=adsec_shell.mass*1e-3*9.81
    actWeight=(adsec_shell.mass*1e-3*9.81)/n_act
    meanBiasForce = mean(adsec.curr4bias_mag[adsec.true_act])*actWeight

    act_true = adsec.true_act
    if keyword_set(TS1) then begin
        cricca = adsec.mir_act2act([221, 222, 223, 224, 276, 277, 278, 279, 337, 338, 339, 340, 404, 405, 406, 407, 477, 478, 479, 480, 556, 557, 558, 559])
        err = complement(cricca, act_true, act_wo_cricca)
        if err ne adsec_error.ok then return, err
    endif else begin
        act_wo_cricca = adsec.true_act
    endelse
 
    ;A2N=-meanEff*sqrt(coilRes);ampere negativi a spingere positivi a tirare
    ;these are the net forces applied to the shell, Fnet=Fcoil+Fweight+Fbias

    startForceG=0.8; % force applied at the beginning of the procedure
    pullForceG=-1.0; % force applied to lift the shell
    finalForceG=-0.6; % final force

    ;% Fcoil=Fnet-Fweight-Fbias
    startForceSet=-(startForceG*actWeight-actWeight-meanBiasForce);  %spingo a 0.6g
    ;startForceSet=-0.11
    pullForceSet=-(pullForceG*actWeight-actWeight-meanBiasForce);    %tiro con -0.6g
    finalForceSet=-(finalForceG*actWeight-actWeight-meanBiasForce);  %tiro con -0.4g

    log_print, 'Clear all forces:'

    err = write_bias_curr(sc.all, 0.0)
    if err ne adsec_error.ok then return, err
    wait, time_ps

    err = clear_dacs(sc.all)
    if err ne adsec_error.ok then return, err
    wait, time_ps
    log_print, "Ok"

    log_print, "Enable coils:"
    err = enable_coils()
    if err ne adsec_error.ok then begin
        print, 'Error in enable coils!!' 
        return, err
    endif
    ; mod RB to allow disabling TSS
    ans=dialog_message('Do you want to enable the coils and disable TSS?', /quest)
    if strlowcase(ans) eq "no"  then return, adsec_error.generic_error

    print, 'Enabling coils and disablig TSS'
    err = adam_enable_coils_dis_tss()
    wait, 0.1
;    err = adam_enable_coils()
    if err ne adsec_error.ok then return, err else log_print, "Ok"

    answ1 = dialog_message(["After checked the LEDS, Do you want to apply the initial force: "+strtrim(startForceSet,2)+"(The shell will be push out of reference body)?"], /QUEST)

    if strlowcase(answ1) eq "no"  then return, adsec_error.generic_error
    startForce=fltarr(adsec.n_actuators)
    startForce[act_wo_cricca]=startForceSet
    err = write_bias_curr(indgen(adsec.n_actuators), startForce)
    if err ne adsec_error.ok then return, err
    wait, time_ps

    err = update_status()
    update_panels, /no_read, /opt

    print, "...applied Initial Forces"
    answ2 = dialog_message(["Do you want to apply the final force(in two step): ("+strtrim(pullForceSet,2)+",1 step) "+strtrim(finalForceSet,2)+"?","Shell will be pull against the reference body"], /QUEST)
    if strlowcase(answ2) eq "no"  then return, adsec_error.generic_error
    print, pullForceSet
    pullForce=fltarr(adsec.n_actuators)
    pullForce[act_true]=pullForceSet

    err = write_bias_curr(indgen(adsec.n_actuators), pullForce)
    if err ne adsec_error.ok then return, err

    wait, time_ps*2.
    print, finalForceSet
    finalForce=fltarr(adsec.n_actuators)
    finalForce[act_true]=finalForceSet

    err = write_bias_curr(indgen(adsec.n_actuators), finalForce)
    if err ne adsec_error.ok then return, err

    print, "...applied Final Forces"

    err = update_status()
    update_panels, /no_read, /opt

    answ4 = dialog_message(["Do you want to apply zero coil force"], /QUEST)
    if strlowcase(answ4) eq "no"  then return, adsec_error.generic_error

    log_print, "Clear all currents:"
    err = write_bias_curr(sc.all, 0.0)
    if err ne adsec_error.ok then return, err

    wait, time_ps

    err = clear_dacs(sc.all)
    if err ne adsec_error.ok then return, err

    err = update_status()
    update_panels, /no_read, /opt
    log_print, "Ok."

    log_print, "Disable coils:"
    err = adam_disable_coils()
    if err ne adsec_error.ok then return, err else log_print, "Ok"

    return, adsec_error.ok

end
