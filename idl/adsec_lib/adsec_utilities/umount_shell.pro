function umount_shell

    @adsec_common

    set_plot, 'x'
    xadsec
    default_file=adsec_path.conf+'configuration.txt'
    def=read_ascii_structure(default_file)
    shell2dism=def.shell
    ans0=dialog_message(['Procedure for dismounting shell '+shell2dism,'Do you want to continue?'],  /quest)
    if strlowcase(ans0) eq "no"  then return, adsec_error.generic_error
    print, "Disable coils"
    print, adam_disable_coils()
    print, "Stop oversampling frames diagnostic"
    print, set_diagnostic(OVER=0)
    print, "Erase theshold on act distance"
    print, set_dist_threshold(sc.all, 4.,1ul)
    tps = 0.01 

;    pr_curr = ((round(adsec.preshaper_len*adsec.sampling_time/tps))  > 1)

    ; read preshaper
;    err=get_preshaper(sc.all_actuators,old_pr_curr,/FF)
;    if err ne adsec_error.ok then return, err

    ; set preshaper
;    err=set_preshaper(sc.all_actuators,pr_curr,/FF)
;    if err ne adsec_error.ok then return, err

;'    time_ps =  step2time_preshaper(pr_curr) ; for DEBUG
    time_ps =  1.;step2time_preshaper(pr_curr) ; for DEBUG
;time_ps = time_preshaper() 
;inizializzazione delle variabili

    n_act = n_elements(adsec.true_act)
    meanEff=0.576; % N/sqrt(W)
    coilRes=6.7;
    shellWeight=adsec_shell.mass*1e-3*9.81
    actWeight=(adsec_shell.mass*1e-3*9.81)/n_act
    meanBiasForce = mean(adsec.curr4bias_mag[adsec.true_act])*actWeight

    if keyword_set(TS1) then begin
        act_true = adsec.true_act
        cricca = adsec.mir_act2act([221, 222, 223, 224, 276, 277, 278, 279, 337, 338, 339, 340, 404, 405, 406, 407, 477, 478, 479, 480, 556, 557, 558, 559])
        err = complement(cricca, act_true, act_wo_cricca)
        if err ne adsec_error.ok then return, err
    endif else begin
        act_wo_cricca = adsec.true_act
    endelse
 

;A2N=-meanEff*sqrt(coilRes);ampere negativi a spingere positivi a tirare

;these are the net forces applied to the shell, Fnet=Fcoil+Fweight+Fbias

startForceG=0.6; % force applied at the beginning of the procedure
;pullForceG=-0.6; % force applied to lift the shell

;finalForceG=-0.2; % final force

;% Fcoil=Fnet-Fweight-Fbias
startForceSet=-(startForceG*actWeight-actWeight-meanBiasForce);  %spingo a 0.2g
;startForceSet=-0.11
;pullForceSet=-(pullForceG*actWeight-actWeight-meanBiasForce);    %tiro con -0.6g
;finalForceSet=-(finalForceG*actWeight-actWeight-meanBiasForce);  %tiro con -0.2g

print, startForceSet

print, 'Enable all coils..'
err = write_bias_curr(sc.all, 0.0)
if err ne adsec_error.ok then return, err
wait, time_ps
;mod RB to allow TSS disabling
ans=dialog_message('Do you want to enable the coils and disable TSS?', /quest)
if strlowcase(ans) eq "no"  then return, adsec_error.generic_error

print, 'Enabling coils and disablig TSS'
err = adam_enable_coils_dis_tss()
wait, 0.1
;print, adam_enable_coils()  ; was not commented in version preliminary to src5

err = enable_coils()
if err ne adsec_error.ok then begin
    print, 'Error in enable coils!!' 
    return, err
endif

print, "disable wo_curr actuators"
if adsec.act_wo_curr[0] gt 0 then begin
    err = disable_coils(adsec.act_wo_curr)
    if err ne adsec_error.ok then begin
        disp_mess, '... error on enabling coils.', /APPEND
        return, err
    endif
endif


print, 'coil enabled'
answ1 = dialog_message(["After verifying that LEDS are green, Do you want to apply the initial force: "+strtrim(startForceSet,2)+"?"], /QUEST)

if strlowcase(answ1) eq "no"  then return, adsec_error.generic_error
startForce=fltarr(adsec.n_actuators)
startForce[act_wo_cricca]=startForceSet
err = write_bias_curr(indgen(adsec.n_actuators), startForce)
if err ne adsec_error.ok then return, err
wait, time_ps

err = update_status()
update_panels, /no_read, /opt

print, "...applied Initial Forces"

err = update_status()
update_panels, /no_read, /opt

answ4 = dialog_message(["Do you want to apply zero coil force"], /QUEST)
if strlowcase(answ4) eq "no"  then return, adsec_error.generic_error

err = write_bias_curr(sc.all, 0.0)
if err ne adsec_error.ok then return, err

wait, time_ps

err = clear_dacs(sc.all)
if err ne adsec_error.ok then return, err

err = update_status()
update_panels, /no_read, /opt
print, "coil disabled"
err = adam_disable_coils()


return, adsec_error.ok

end
