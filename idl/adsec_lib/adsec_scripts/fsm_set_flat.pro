;$Id: fsm_set_flat.pro,v 1.19 2009/11/25 08:48:47 marco Exp $
;+
; NAME:
;   FSM_SET_FLAT
;
; PURPOSE:
;   Function for the passage from the Operating state to an operative state (SL, AO, Chop)
;   In the Operanting state the LBT672 crates have power.The BCU, SIGGEN, DSP 
;   boards and Power Backplane firmware has correctly bootstrapped.Housekeeping 
;   diagnostic is running. The shell is pushed against the reference plate by 
;   bias magnets.
;
;   Events in Set_Flat event:
;       -   Mirror Setting to configured gap
;       -   Mirror Flattened to configured shape
;       -   Control loop gain rising
;       -   If needed, load the optical loop reconstructor matrices.
;
; CATEGORY:
;   Supervisor IDL function.
;
; CALLING SEQUENCE:
;   err = FSM_SET_FLAT()
;
; INPUTS:
;   None.
;
; KEYWORD PARAMETERS:
;   MS_CONF_FILE = mscf : mirror set basic parameter file setting (gap, basic p_gain...). If no file provided, default SL configuration will be used.
;   FS_CONF_FILE = fscf : flat shape delta command file (got from calibration routines). If omitted, no shell deformation will be provided as default.
;   OL_CONF_FILE = olcf : reconstructor matrices and optical loop filter parameters file. If not, no reconstructor matrix or filter will be loaded into the system.
;   ELEVATION    = elev : current telescope elevation, in degrees.
;
;
; OUTPUTS:
;   err : Error code.
;
; RESTRICTIONS:
;   None.
;
; MODIFICATION HISTORY:
;   Created by Marco Xompero on Jan 2009
;   01/10 Added elevation keyword.
;-
;-

function fsm_set_flat, MS_CONF_FILE=mscf, FS_SAV_FILE=fscf, OL_CONF_FILE=olcf, FFCALIBRATION=ff_calibration, DISABLE_ICC=disable_icc, DISP=disp, ELEVATION=elev, NO_FLAT_ON_FF=no_flat_on_ff, CHOP=chop, PROGRESS=progress

    @adsec_common
    if sc.name_comm eq "Dummy" then return, adsec_error.ok
    if n_elements(progress) eq 0 then progress=[0,100]

    log_print, "fsm_set_flat() procedure started."
    err = update_msg('Adsec '+!AO_CONST.unit+'-'+ !AO_CONST.telescope_side+' shell set started.')
    if err ne adsec_error.ok then return, err
    cpr = progress[0]
    err = update_progress(cpr)
    if err ne adsec_error.ok then return, err
    
    err_cnt = 0
    catch, Error_status
   
;error_status=0
    ;This statement begins the error handler:  
    if Error_status ne 0 then begin

        log_print, "Error catching service procedure of fsm_set_flat() was called."
        err_cnt += 1
        ;print, err_cnt

        if err_cnt gt 1 then begin
            log_print, "RECURSIVE ERROR."
            log_print, "PANIC: NO ACTION CAN BE DONE!!!!!!"
            return, !AO_ERROR
        endif
        err1 = !AO_ERROR

        log_print, !ERROR_STATE.MSG + " " +strtrim(!AO_ERROR, 2)

        err2 = adam_disable_coils()
        if err2 ne adsec_error.ok then begin
            log_print, "PANIC: NO ACTION CAN BE DONE!!!!!!"
            return, err2
        endif
        log_print, "Coils succeffully disabled."
        log_print, "Ripping the shell..."
        err2 = fsm_rip()
        if err2 ne adsec_error.ok then begin
            log_print, "PANIC: NO ACTION CAN BE DONE!!!!!!"
            return, err2
        endif
        if err1 ne 0 then return, err1 else begin
            log_print, "Error catched but unable to identify the source of error. Returning generic fault."
            return, adsec_error.generic_error
        endelse
        
    endif

    ;PATCH requested by RB, 16/11/2011
    filename=filepath(ROOT=adsec_path.commons, sub=['ref_wave'], 'wave.txt')
    err = update_wave(filename)
    if err ne adsec_error.ok then return, err
    log_print, 'Reference wave increased'
    ;END PATCH requested by RB, 16/11/2011

    log_print, "Restore operative thresholds for RMS"
    changes = setdiagnparam(process_list.fastdiagn.msgd_name,  'ChDistRMS', -1, -1, WARNING_MAX=1e-7, ALARM_MAX=5e-7, TIM=1)
    for i=0, n_elements(adsec.act_wo_icc)-1 do begin
        changes = setdiagnparam(process_list.fastdiagn.msgd_name,  'ChDistRMS', adsec.act_wo_icc[i], adsec.act_wo_icc[i], WARNING_MAX=1e-6, ALARM_MAX=1, TIM=1)
        if changes lt 0 then log_print, "A problem occurred restoring RMS thresholds for act: DSP#"+string(adsec.act_wo_icc[i], FORMAT='(I3.3)')+".Continue anyway", LOG_LEV=!AO_CONST.LOG_LEV_WARNING
        wait, 0.1
    endfor
    log_print, "Done"


    ;MIRROR SET PROCEDURE: CONFIGURATION AND CALL
    ;common temporary_block, comm0, c_step, n_step, cl_list
    if n_elements(mscf) eq 0 then begin
        if keyword_set(CHOP) then mscf = "mirror_set_conf_offload_chop.txt" $
        else mscf = "mirror_set_conf_offload.txt"
    endif
    def_val=read_ascii_structure(filepath(ROOT=adsec_path.conf,SUB=!AO_CONST.SHELL,mscf))

    final_pos = def_val.final_pos
    start_gain = def_val.start_gain/mean(adsec.gain_to_bbgain[adsec.act_w_cl])
    final_gain	= def_val.final_gain/mean(adsec.gain_to_bbgain[adsec.act_w_cl])
    shell_floating_pos = def_val.shell_floating_pos
    shell_set_pos_file = def_val.shell_set_pos
    max_amp_pos = def_val.max_amp_pos
    ovs = def_val.oversampling_time
    use_ctrl = def_val.use_int_ctrl_curr
    final_pr_cmd = ((round(adsec.preshaper_len*adsec.sampling_time/def_val.final_cmd_preshaper)  > 1) $
               < dsp_const.fastest_preshaper_step)
    final_pr_curr = ((round(adsec.preshaper_len*adsec.sampling_time/def_val.final_ff_preshaper)  > 1) $
               < dsp_const.fastest_preshaper_step)

    if shell_set_pos_file eq "" then begin
        undefine, shell_set_pos
    endif else begin
        fullfile = adsec_path.data+shell_set_pos_file
        info = file_info(fullfile)
        if ~(file_test(full_file, /READ, /REGULAR)) then begin
            log_message, "WRONG CONFIGURATION FILE", CONT = (sc.debug eq 0), ERR =  adsec_error.invalid_file
            return, adsec_error.invalid_file
        endif
            
        restore, fullfile, /ver
        restored = status_save.adsec.act_wo_pos
        actual = adsec.act_wo_pos
        if intersection(actual, restored, out_aa)        $
           or n_elements(out_aa) ne n_elements(actual)   $
           or n_elements(out_aa) ne n_elements(restored) $
           then begin
            log_message, "POSITION SAVED NOT COMPATIBLE WITH ACTUAL ACTUATOR ACT_W_POS PATTERN", CONT = (sc.debug eq 0), ERR=adsec_error.generic_error
        endif

        shell_set_pos = status_save.sys_status.position
    endelse
    if max_amp_pos gt 5e-6 then begin
        log_message, "ERROR: MAX AMP POS REQUESTED TOO LARGE", CONT = (sc.debug eq 0), ERR=adsec_error.IDL_SAFE_PARAMS_OOB
        return, !AO_ERROR
    endif


    if start_gain gt 2e4 then begin
        log_message, "ERROR: START GAIN REQUESTED TOO LARGE", CONT = (sc.debug eq 0), ERR=adsec_error.IDL_RAMP_GAIN_ERROR
        return, !AO_ERROR
    endif

    if final_gain gt 2e4 then begin
        log_message, "ERROR: FINAL GAIN REQUESTED TOO LARGE", CONT = (sc.debug eq 0), ERR=adsec_error.IDL_RAMP_GAIN_ERROR
        return, !AO_ERROR
    endif

    if ovs gt 0.03 or ovs lt 0.0015 then begin
        log_message, "ERROR: OVS REQUESTED HAS WRONG VALUES", CONT = (sc.debug eq 0), ERR=adsec_error.IDL_OVS_NOT_VALID
        return, !AO_ERROR
    endif



    ;STOP OVERSAMPLING IF ANY
    err = set_diagnostic(OVER=0.)
    if err ne adsec_error.ok then begin
        log_message, "Error stoppig oversampling.", CONT = (sc.debug eq 0), ERR=err
        return, err
    endif

    wait, 0.5
    err = update_status() 
    if err ne adsec_error.ok then begin
        log_message, "Error updating status.", CONT = (sc.debug eq 0), ERR=err
        return, err
    endif


    ;PUSH THE SHELL AGAINST THE RB AND CLEAN CURRENTS
    err = rip() 
    if err ne adsec_error.ok then begin
        log_message, "Error ripping the shell.", CONT = (sc.debug eq 0), ERR=err
        return, err    
    endif


    err = update_status() 
    if err ne adsec_error.ok then begin
        log_message, "Error updating status", CONT = (sc.debug eq 0), ERR=err
        return, err
    endif

    err = update_msg('Adsec '+!AO_CONST.unit+'-'+ !AO_CONST.telescope_side+' enabling coils.')
    if err ne adsec_error.ok then return, err

    cpr = progress[0]+fix(0.1*(progress[1]-progress[0]))
    err = update_progress(cpr)
    if err ne adsec_error.ok then return, err


    if total(sys_status.current) eq 0 then begin
        log_print, "Enabling coils and update TSS status..."
        err = adam_update()
        if err ne adsec_error.ok then log_message, "Error updating adam status ",ERR=err
        tss_n_status = adam_out.tss_disable

        err = adam_enable_coils_dis_tss()
        if err ne adsec_error.ok then log_message, "Error enabling coils ",ERR=err
;        err = adam_enable_coils()
;        if err ne adsec_error.ok then return, err

        for i=0, 10 do begin
            log_print, "Coil testing after TSS disabling..."
            wait, 0.5
            err = test_coils()
            if err eq adsec_error.ok then begin
;                log_print, "Coils successfully enabled."
                break
            endif

        endfor
        if err ne adsec_error.ok then log_message, "Error enabling coils ",ERR=err 


        log_print, "Restoring TSS previous status...."
        if tss_n_status then err = adam_disable_tss() else err=adam_enable_tss()
        if err ne adsec_error.ok then log_message, "Error enabling coils ",ERR=err
        log_print, "Coil test after the TSS status restoring..."
        err = test_coils()
        if err ne adsec_error.ok then log_message, "Error enabling coils ",ERR=err
           
        log_print, "Coils successfully enabled."

;        err = adam_enable_coils()
;        if err ne adsec_error.ok then return, err

    endif else begin
        log_print, "NOT NULL CURRENTS: coils forced disabled"
        err = adam_disable_coils()
        return, err
    endelse

    err = update_status() 
    if err ne adsec_error.ok then log_message, "Error updating status", CONT = (sc.debug eq 0), ERR=err


    ;COMPUTE THE FIRST COMMAND
    thinit_force = 0.34 ;initial force ratio (weight_curr unit) threshold
    mf = adsec.weight_curr*thinit_force
;    shell_floating_pos =  median(((mf+sys_status.position*start_gain*mean(adsec.gain_to_bbgain[adsec.act_w_cl]))/start_gain/mean(adsec.gain_to_bbgain[adsec.act_w_cl]))[adsec.act_w_cl])
    if shell_floating_pos lt min(sys_status.position[adsec.act_w_cl]) then log_message, "Error!!! The shell has too wrong capacitive sensor readings for setting", ERR=adsec_error.IDL_SAFE_PARAMS_OOB
    ff = (shell_floating_pos-sys_status.position[adsec.act_w_cl])*start_gain*mean(adsec.gain_to_bbgain[adsec.act_w_cl])
    log_print, "mean ff "+strtrim(mean(ff[adsec.act_w_cl]),2) + "  " +  strtrim(0.8*adsec.weight_curr, 2)
    if abs(mean(ff[adsec.act_w_cl])) gt 0.8*adsec.weight_curr then log_message, "Error!!! The requested mean initial force to set shell is exceeding limits.", ERR=adsec_error.IDL_SAFE_PARAMS_OOB
    if max(abs(ff[adsec.act_w_cl])) gt 3*adsec.weight_curr then log_message, "Error!!! The max peak force requested to set the shell is exceeding limits.", ERR=adsec_error.IDL_SAFE_PARAMS_OOB

    ;start lines to create a new CL_INIT_DCMD
 ;   make_chopping_tilt, adsec.act_w_cl, tilt, force, mc, /pure, angle=!pi/8+!pi/12.-!pi
  ;  cl_init_dcmd = tilt*3e-6 + 6e-6

    err = mirror_set_offload2(FINAL_POS=final_pos, S_GAIN=start_gain, F_GAIN=final_gain $
                            , OVERSAMPLING_TIME=ovs, SHELL_FLOATING_POS=shell_floating_pos, CL_INIT_DCMD=cl_init_dcmd    $
                            , SHELL_SET_POS=shell_set_pos, MAX_AMP_POS=max_amp_pos, CTRL = use_ctrl $
                        , FINAL_PR_CMD=final_pr_cmd,  FINAL_PR_CURR=final_pr_curr, RIP_MIRROR=rip_mirror, /SKIP , PROGRESS=[20,80])
    if err ne adsec_error.ok then begin
        log_message, "ERROR ON SETTING PROCEDURE!!!", ERR=err
;        return, err
    endif



;    If n_elements(olcf) gt 0 then begin
;
;        ;configure RTR
;        err = update_rtr(olcf)
;        if err ne adsec_error.ok then return, err
;
;    endIf


    ;print, "RAMP GAIN TO 0.03 N/um for calibration purpose"
    ;err = ramp_gain(adsec.act_w_cl, 0.03)
    ;if err ne adsec_error.ok then begin
    ;    log_message, "ERROR RAMPING THE CONTROL LOOP GAIN!!"  , CONT = (sc.debug eq 0), ERR=err
;        return, err
    ;endif

;    print, "Reduce diagnostic frequency to 50Hz..."
;    err = set_diagnostic(over=ovs_final)
;    if err ne adsec_error.ok then message, "ERROR INCREASING
;    DIAGNOSTIC FREQUENCY"   

    ;return, adsec_error.ok
    
    if ~keyword_set(FF_CALIBRATION) then begin

        err = update_msg('Adsec '+!AO_CONST.unit+'-'+ !AO_CONST.telescope_side+' set PD gains.')
        if err ne adsec_error.ok then return, err
        cpr = progress[0]+fix(0.8*(progress[1]-progress[0]))
        err = update_progress(cpr)
        if err ne adsec_error.ok then return, err

        if !AO_CONST.unit eq '585m' then begin
            err = set_icc(/reset)
            if err ne adsec_error.ok then return, err

            err = ramp_gain(adsec.act_w_cl, 0.01, /NOCH)
            if err ne adsec_error.ok then return, err
            err = zero_ctrl_current2()
            if err ne adsec_error.ok then return, err

            err = ramp_gain(adsec.act_w_cl, 0.03, /NOCH)
            if err ne adsec_error.ok then return, err
            err = zero_ctrl_current2()
            if err ne adsec_error.ok then return, err
            wait, 0.1
        endif

        err = complement(adsec.dummy_act,adsec.act_wo_pos, wo_pos_not_dummy) 
        if err ne 1 then begin
             err = intersection(wo_pos_not_dummy, adsec.act_w_curr, wo_pos_w_curr)
             if err ne 1 then begin
                log_print, "Set new bias current to wo_pos actuators"
                vv = fltarr(672)
                vv[wo_pos_w_curr] = mean(sys_status.current[adsec.act_w_cl])
                err = set_offload_force(vv[adsec.act_w_curr], /DELTA, /SWITCH, /START)    
                if err ne adsec_error.ok then return, err
             endif
        endif

        pos_um=round(mean(final_pos[adsec.act_w_cl]*1e6))
        gfile=filepath(ROOT=adsec_path.conf,SUB=!AO_CONST.shell, 'gain_'+string(pos_um, format='(I2.2)')+'um.txt')
        err = ramp_gain_script(gfile, adsec.act_w_cl)
;        case pos_um of
    ;        80 : err = ramp_gain_script80()
;            60 : err = ramp_gain_script(gfile, adsec.act_w_cl)
    ;        70 : err = ramp_gain_script70()
;            else: print, 'NO CONFIGURATION WAS SET, THE CONTROL LOOP GAINS IN USE ARE ONLY PROPORTIONAL GAINS,' ,final_gain, 'N/um'       
;        endcase
        if err ne adsec_error.ok then begin
            log_message, "ERROR RAMPING THE CONTROL LOOP GAIN!!" , CONT=(sc.debug eq 0), ERR=err
 ;           return, err
        endif
        if !AO_CONST.unit eq '585m' then begin
            err = update_ovs(rtr.oversampling_time)
            if err ne adsec_error.ok then log_message, "ERROR RAMPING RESTORING ICC!!" , CONT=(sc.debug eq 0), ERR=err
        endif


    endif


    err = update_msg('Adsec '+!AO_CONST.unit+'-'+ !AO_CONST.telescope_side+' apply flat shape.')
    if err ne adsec_error.ok then return, err
    cpr = progress[0]+fix(0.9*(progress[1]-progress[0]))
    err = update_progress(cpr)
    if err ne adsec_error.ok then return, err



    ;FLAT SHAPE: CONFIGURATION AND PROCEDURE
 ;   err = rec_apply_flat(FLATTENING_FILE=fscf)
    wait, 2*rtr.oversampling_time > 0.3
    err = update_status()
    if err ne adsec_error.ok then begin
        log_message, "ERROR UPDATING THE STATUS!!" , CONT=(sc.debug eq 0), ERR=err
 ;       return, err
    endif

    sys_status.flat_offload_cmd = sys_status.offload_cmd
    sys_status.flat_offload_curr = sys_status.offload_curr
    sys_status.cflat_offload_cmd = sys_status.offload_cmd
    sys_status.cflat_offload_curr = sys_status.offload_curr

    flattened_status = sys_status
    ;save, file = '/towerdata/adsec_calib/CMD/shape/last_mirror_set.sav', flattened_status

    ;fscf = '/home/aoacct/AO/current/conf/adsec/672a/data/flat/20100316_185538_close_loop_200modes_a_mean.sav'
    ;fscf = '/home/aoacct/meas/adsec_calib/flat/20100514_201113/IRTC.20100514_cloop200modes.sav'
    ;fscf = '/home/aoacct/meas/adsec_calib/flat/20100515_184859/IRTC.20100515_moccolo3.sav.sav'
;    fscf = '/home/aoacct/meas/adsec_calib/flat/20100516_000923/IRTC.20100515_moccolo5.sav.sav'
    ;fscf = '/home/aoacct/meas/adsec_calib/flat/IRTC.flat_data_120modes_91nm.sav'
;fscf = '/home/aoacct/meas/adsec_calib/flat/IRTC_flat_400modes.sav'
    fscf = filepath(ROOT=adsec_path.meas, sub=['flat', 'Applicable'], 'default.sav')
    if (file_info(fscf)).exists eq 0 then begin
        file_mkdir,  filepath(ROOT=adsec_path.meas, sub=['flat', 'Applicable'],'tmp')
        file_delete, filepath(ROOT=adsec_path.meas, sub=['flat', 'Applicable'],'tmp')
        save, file=filepath(ROOT=adsec_path.meas, sub=['flat', 'Applicable'], 'shell_set.sav'), flattened_status 
        file_link, filepath(ROOT=adsec_path.meas, sub=['flat', 'Applicable'], 'shell_set.sav'), filepath(ROOT=adsec_path.meas, sub=['flat', 'Applicable'], 'default.sav')
    endif
    if ~keyword_set(NO_FLAT_ON_FF) then begin
        err = fsm_load_shape(fscf, /NOPIST, DISABLE_ICC=disable_icc, DISP=disp, act_list= adsec.act_w_ffb_on_flat)  ;mod at LBTO to keep the shell floating but not flattened
        ;err = fsm_load_shape(fscf, /NOPIST, DISABLE_ICC=disable_icc, DISP=disp)
        if err ne adsec_error.ok then return, err
    ;    sys_status.flat_offload_cmd = sys_status.offload_cmd
    ;    sys_status.flat_offload_curr = sys_status.offload_curr
    endif

    if ~keyword_set(FF_CALIBRATION) then begin
        ovs_final = 0.002
        if keyword_set(disable_icc) then begin
            err = set_diagnostic(OVER=ovs_final)
            if err ne adsec_error.ok then return, err
        endif else begin
            err = update_ovs(ovs_final)
            if err ne adsec_error.ok then return, err
        endelse
        wait, 1
;        err = enable_ho_offload()
;        if err ne adsec_error.ok then return, err
        ; JA: commented at LBT 2011-06-16
        ;err = enable_ho_offload()
        ;if err ne adsec_error.ok then return, err
    
    endif else begin

            err = set_icc(/reset)
            log_print, "ICC reset for FF calibration"
            if err ne adsec_error.ok then return, err

	
            if !AO_CONST.unit eq '585m' then begin
		
		
                err = ramp_gain(adsec.act_w_cl, 0.01)
                if err ne adsec_error.ok then return, err
                err = zero_ctrl_current2()
                if err ne adsec_error.ok then return, err

                err = ramp_gain(adsec.act_w_cl, 0.03)
                if err ne adsec_error.ok then return, err
                err = zero_ctrl_current2()
                if err ne adsec_error.ok then return, err
            endif

    endelse


    if  xregistered('xadsec') gt 0 then  begin
        update_panels, /OPT, /NO_READ
    endif


    ;============================================================================================================  
    ; update state current
;    adsec_state.state_current = adsec_state.operating 
;    !AO_STATUS.fsm_state = "AOSet"
    ;============================================================================================================  

    err = update_msg('Adsec '+!AO_CONST.unit+'-'+ !AO_CONST.telescope_side+' shell set done.')
    if err ne adsec_error.ok then return, err
    cpr = progress[1]
    err = update_progress(cpr)
    if err ne adsec_error.ok then return, err

	;ENABLE FOCAL STATION
;    log_print, "Focal station enabled"
;err = select_wfs_port(1)	
;   if err ne adsec_error.ok then return, err

;   subs = (strsplit(process_list.adsecarb.msgd_name, '.', /EX))[0]
;   err = write_var('FOCAL_STATION','port1', subs=subs)
;   if err ne adsec_error.ok then return, err

    log_print, "fsm_set_flat() procedure done."
    return, set_fsm_state('AOSet')

end
