;ALL THE RECONSTRUCTOR PARAMETER HAS TO BE SPECIFIED (REC_MAT, A_DELAY< B_DELAY< M2C)
function fsm_load_rec, REC_MAT_F=rec_mat_file                  $
                     , A_DELAY_F=a_delay_file                  $
                     , B_DELAY_F=b_delay_file                  $
                     , M2C_F=m2c_file                          $
                     , G_GAIN=gain_file                        $
                     , NO_ACC_CORRECTION=no_acc_correction     $
                     , BLOCK=block, NOBLOCKCHECK=noblockcheck  $
                     , ONLY_ACC=only_acc, PROGRESS=progress

    @adsec_common
    if sc.name_comm eq "Dummy" then return, adsec_error.ok
    if n_elements(progress) eq 0 then progress=[0,100]

    err = update_msg('Adsec '+!AO_CONST.unit+'-'+ !AO_CONST.telescope_side+' loading reconstructor.')
    if err ne adsec_error.ok then return, err
    cpr = progress[0]+fix(0.*(progress[1]-progress[0]))
    err = update_progress(cpr)
    if err ne adsec_error.ok then return, err

    ask_b0      =  n_elements(rec_mat_file) gt 0 
    ask_a_delay =  n_elements(a_delay_file) gt 0 
    ask_b_delay =  n_elements(b_delay_file) gt 0 
    ask_m2c     =  n_elements(m2c_file)     gt 0 



    if ask_b0 then begin
        log_print, "Loading: "+rec_mat_file
        b0_exists  = (file_info(rec_mat_file)).exists
        if ~(b0_exists) then begin
            log_message, "ERROR: reconstruction matrix (B0) file not exists.", CONT = (sc.debug eq 0), ERR=adsec_error.invalid_file
            return, adsec_error.invalid_file
        endif
        b0  = readfits(rec_mat_file)
        sb0  = size(b0, /DIM)
        if total(sb0 eq [rtr.n_slope,adsec.n_actuators]) ne 2 then begin
            log_message, "Wrong dimensions of reconstruction matrix (B0) matrix in input", CONT = (sc.debug eq 0), ERR=adsec_error.input_type
            return, adsec_error.input_type
        endif
        update_b0 = 1
    endif

    if ask_a_delay then begin
        ad_exists  = (file_info(a_delay_file)).exists
        if ~(ad_exists) then begin
            log_message, "ERROR: modes delay matrix (A_DELAY) file not exists.", CONT = (sc.debug eq 0), ERR=adsec_error.invalid_file
            return, adsec_error.invalid_file
        endif
        a_delay = readfits(a_delay_file)
        sad  = size(a_delay, /DIM)
        if total(sad eq [adsec.n_actuators,adsec.n_actuators, rtr.n_modes_delay]) ne 3 then begin
            log_message, "Wrong dimensions of mode matrix (A_DELAY) in input", CONT = (sc.debug eq 0), ERR=adsec_error.input_type
            return, adsec_error.input_type
        endif
        update_ad = 1
    endif

    if ask_b_delay then begin
        bd_exists  = (file_info(b_delay_file)).exists 
        if ~(bd_exists) then begin
            log_message, "ERROR: slope delay matrix (B_DELAY) file not exists.", CONT = (sc.debug eq 0), ERR=adsec_error.invalid_file
            return, adsec_error.invalid_file
        endif
        b_delay  = readfits(b_delay_file)
        sbd  = size(b_delay, /DIM)
        if total(sbd eq [rtr.n_slope,adsec.n_actuators, rtr.n_slope_delay]) ne 2 then begin
            log_message, "Wrong dimensions of slope delay matrix (B_DELAY) in input", CONT = (sc.debug eq 0), ERR=adsec_error.input_type
            return, adsec_error.input_type
        endif
        update_bd = 1
    endif

    if ask_m2c     then begin
        md_exists  = (file_info(m2c_file)).exists     
        if ~(md_exists) then begin
            log_message, "ERROR: modes-to-command matrix (M2C) file not exists.", CONT = (sc.debug eq 0), ERR=adsec_error.invalid_file
            return, adsec_error.invalid_file
        endif
        m2c = readfits(m2c_file)
        if ~keyword_set(NO_ACC_CORRECTION) then begin
            ;PUT IN THE LAST 3 COLUMNS PISTON/TIP/TILT correction
            ;TEMPORARY AVOID PISTON
            ;COMMENTED TEMPORARY TO AVOID REC PROBLEMS
            m2c[adsec.n_actuators-3, *] = pm_offload.pmz[1,*]*0
            m2c[adsec.n_actuators-2, *] = pm_offload.pmz[2,*]*0
        endif
        sm2c = size(m2c, /DIM)
        if total(sm2c eq [adsec.n_actuators, adsec.n_actuators]) ne 2 then begin
            log_message, "Wrong dimension of mode-to-command matrix (M2C) in input", CONT = (sc.debug eq 0), ERR=adsec_error.input_type
            return, adsec_error.input_type
        endif
        save, file=filepath(ROOT=adsec_path.meas, SUB=['CMD', 'shape'],'current_ref_m2c.sav'), m2c
        update_m2c = 1
    endif

    change_block = keyword_set(block)

    if keyword_set(ONLY_ACC) then begin
        update_b0=1
        update_bd=1
        update_ad=1
        update_m2c=1
        m2c = fltarr(adsec.n_actuators, adsec.n_actuators)
        b0  = fltarr(rtr.n_slope, adsec.n_actuators)
        b_delay = fltarr(rtr.n_slope,adsec.n_actuators, rtr.n_slope_delay)
        a_delay = fltarr(adsec.n_actuators,adsec.n_actuators, rtr.n_modes_delay)
        m2c[adsec.n_actuators-3, *] = pm_offload.pmz[1,*]
        m2c[adsec.n_actuators-2, *] = pm_offload.pmz[2,*]
    endif



    ;;TEST FOR NO SKIP FRAME ACTIVITIES
    err = test_skip_frame(/NOVERB, WFS_CNT = cnt0, PENDING=psf0, SAFE=ssf0)
    if err ne adsec_error.ok then return, err
    wait, 0.2
    err = test_skip_frame(/NOVERB, WFS_CNT = cnt1, PENDING=psf1, SAFE=ssf1)
    if err ne adsec_error.ok then return, err


    if (psf1 - psf0) ne 0 then begin
        log_message, "Pending skip frames counter is increasing, It is not possible to update reconstructor. Please stop the Slope Computer and retry.", CONT = (sc.debug eq 0), ERR=adsec_error.IDL_SAFE_SKIP_ACTIVE
        return, adsec_error.IDL_SAFE_SKIP_ACTIVE
    endif

    if (ssf1 - ssf0) ne 0 then begin
        log_message, "Safe skip frame counter is increasing. Slope computer is now running sending slopes. Please recovey from skip frame and retry.", CONT = (sc.debug eq 0), ERR=adsec_error.IDL_SAFE_SKIP_ACTIVE
        return, adsec_error.IDL_SAFE_SKIP_ACTIVE
    endif

    if  (update_b0 or update_bd) and (~update_ad) and ~(update_m2c) and  keyword_set(BLOCK) then begin

        err_cnt = 0
;'        catch, Error_status
         Error_status=0
        ;This statement begins the error handler:  
        if Error_status ne 0 then begin

            err_cnt += 1

            if err_cnt gt 1 then begin
                log_print, "RECURSIVE ERROR."
                log_print, "PANIC: NO ACTION CAN BE DONE!!!!!!"
                return, !AO_ERROR
            endif
            err1 = !AO_ERROR

            log_print, !ERROR_STATE.MSG_PREFIX+!ERROR_STATE.MSG + " " +strtrim(!AO_ERROR, 2)
            return, err1
            
        endif

        
        ;TEST CURRENT RECONSTRUCTOR BLOCK 
        bloc4yk_read = read_seq_dsp(0, rtr_map.switch_SCStartRTR, 2L, buf, /SWITCH, /UL)
        pbs = buf[1]
        curr_rtr_block = (pbs and 2^0) gt 0
        block2w = ~(curr_rtr_block)

        if update_b0 then begin
            err = set_b0_matrix(b0, BLOCK=block2w, FITS=rec_mat_file)
            if err ne adsec_error.ok then log_message, "Error applying low level settings CMD #a", ERR=err
        endif

        cpr = progress[0]+fix(0.5*(progress[1]-progress[0]))
        err = update_progress(cpr)
        if err ne adsec_error.ok then return, err

        if update_b_delay then begin
            err = set_b_delay_matrix(b_delay, BLOCK=block2w, FITS=b_delay_file)
            if err ne adsec_error.ok then log_message, "Error applying low level settings CMD #b", ERR=err
        endif

        cpr = progress[1]
        err = update_progress(cpr)
        if err ne adsec_error.ok then return, err

        return, update_rtdb()


    endif else begin

        if (cnt1 - cnt0) ne 0 then begin
            log_message, "Slope Computer is now routing slopes do the Adaptive Secondary. Please stop it and retry.", CONT = (sc.debug eq 0), ERR = adsec_error.IDL_SC_SENDING_SLOPES
            return, adsec_error.IDL_SC_SENDING_SLOPES
        endif

        if keyword_set(BLOCK) then begin
            ;TEST CURRENT RECONSTRUCTOR BLOCK 
            bloc4yk_read = read_seq_dsp(0, rtr_map.switch_SCStartRTR, 2L, buf, /SWITCH, /UL)
            pbs = buf[1]
            curr_rtr_block = (pbs and 2^0) gt 0
            block2w = ~(curr_rtr_block)
        endif else begin
            block2w = 0
        endelse

     
        ;CLEAR SLOPES 
;        pend_skip_frame = 1
;        pend_cnt = 0
;        max_pend = 10
;        while pend_skip_frame gt 0 do begin
;                
;            err = test_skip_frame(/NOVERB, WFS_CNT = cnt0, PENDING=psf0, SAFE=ssf0)
;            if err ne adsec_error.ok then return, err
;            err = wfs_switch_step(fltarr(rtr.n_slope), WAIT=0.1)
;            if err ne adsec_error.ok then return, err
;            err = test_skip_frame(/NOVERB, WFS_CNT = cnt1, PENDING=psf1, SAFE=ssf1)
;            if err ne adsec_error.ok then return, err
;            if psf1-psf0 eq 0 then pend_skip_frame = 0 else pend_cnt += 1
;            if pend_cnt eq max_pend then begin
;                log_message, "Unable to load reconstructor because not able to clean slope", CONT = (sc.debug eq 0), ERR=adsec_error.IDL_SC_SENDING_SLOPES
;                return, adsec_error.IDL_SC_SENDING_SLOPES
;            endif
;
;        endwhile


        err_cnt = 0
        old_ovs = rtr.oversampling_time
        ;catch, Error_status
        Error_status=0
        ;This statement begins the error handler:  
        if Error_status ne 0 then begin

            err_cnt += 1

            if err_cnt gt 1 then begin
                log_print, "RECURSIVE ERROR."
                log_print, "PANIC: NO ACTION CAN BE DONE!!!!!!"
                return, !AO_ERROR
            endif
            err1 = !AO_ERROR

            log_print, !ERROR_STATE.MSG_PREFIX+!ERROR_STATE.MSG + " " +strtrim(!AO_ERROR, 2)

; AP removed
;            err2 = adam_disable_coils()
;           ; err2 = 0
;            if err2 ne adsec_error.ok then begin
;                log_print, "PANIC: NO ACTION CAN BE DONE!!!!!!"
;                return, err2
;            endif
;            log_print, "Coils succeffully disabled."
            err1 = set_diagnostic(over=old_ovs)
            if err1 ne adsec_error.ok then log_message, "Error applying low level settings on catch routine", ERR=err
            return, err
            
        endif

        err = read_seq_dsp(0, rtr_map.modes_vector, long(adsec.n_actuators), modes1)   
        if err ne adsec_error.ok then log_message, "Error applying low level settings CMD #c", ERR=err
 
        ;only the command because the force are just integrated in the ff_pure_current
        err = get_commands(sc.all, ol, /OFFLOAD)
        if err ne adsec_error.ok then log_message, "Error applying low level settings CMD #d", ERR=err

        err = get_commands(sc.all, olf, /CURR)
        if err ne adsec_error.ok then log_message, "Error applying low level settings CMD #e", ERR=err

        current_m2c_mat = *rtr.m2c_matrix
        cmd  = float(current_m2c_mat ## transpose(modes1))
        curr = float(adsec.ff_matrix ## cmd)
        offload_cmd  = float(ol+cmd)  
        offload_curr = float(olf+curr) 

        cpr = progress[0]+fix(0.1*(progress[1]-progress[0]))
        err = update_progress(cpr)
        if err ne adsec_error.ok then return, err

        t0 = systime(/sec)
        err = set_diagnostic(OVER=0.0, TIMEOUT=10000.)
        if err ne adsec_error.ok then log_message, "Error applying low level settings CMD #f", ERR=err

        err = set_b0_matrix(b0, BLOCK=block, FITS=rec_mat_file)
        if err ne adsec_error.ok then log_message, "Error applying low level settings CMD #f", ERR=err

        cpr = progress[0]+fix(0.3*(progress[1]-progress[0]))
        err = update_progress(cpr)
        if err ne adsec_error.ok then return, err

        err = set_b_delay_matrix(b_delay, BLOCK=block, FITS=b_delay_file)
        if err ne adsec_error.ok then log_message, "Error applying low level settings CMD #g", ERR=err

        cpr = progress[0]+fix(0.45*(progress[1]-progress[0]))
        err = update_progress(cpr)
        if err ne adsec_error.ok then return, err

        ;RTR parameter cleaning and offload current position lines cleaning funziona
        err = clear_dl()
        if err ne adsec_error.ok then log_message, "Error applying low level settings CMD #i", ERR=err

        cpr = progress[0]+fix(0.60*(progress[1]-progress[0]))
        err = update_progress(cpr)
        if err ne adsec_error.ok then return, err

       if (block2w) then rec = *rtr.b0_b_matrix else rec=*rtr.b0_a_matrix
       if update_m2c then begin
           m2c_used = m2c 
           ;!AO_STATUS.pmhofor = add_timestamp(getenv('$ADOPT_LOG')+'ho_matrix_'+m2c_file), comp_ho_offload_matrix(m2c_used, /AO)
           ;writefits, !AO_STATUS.pmhofor, comp_ho_offload_matrix(m2c_used, /AO)
       endif else begin
           m2c_used = current_m2c_mat
       endelse


        if ~keyword_set(ONLY_ACC) then begin
           dmodes_th = comp_modal_th(rec)
           tmp =  total(total(abs(m2c_used), 2), /CUM)
           nmodes = ((where(deriv(tmp) eq 0))[0] -1) < (adsec.n_actuators - 20) 
           if nmodes eq 1 then log_message, "Error applying low level settings CMD #nn", ERR=err
           err = set_safe_skip(ACT=adsec.act_w_cl, MODLIST=indgen(nmodes), DELTAMODE=dmode_th)
           if err ne adsec_error.ok then log_message, "Error applying low level settings CMD #n", ERR=err


            ;set up the modes/commands/forces thresholds
            err = set_safe_skip(ACT=adsec.act_w_cl, MODLIST=indgen(n_elements(adsec.act_w_cl)))
            if err ne adsec_error.ok then log_message, "Error applying low level settings CMD #n", ERR=err
        endif

        ;set up the recostruction/delay/m2c matrices
        err = set_m2c_matrix(m2c, FITS=m2c_file)
        if err ne adsec_error.ok then log_message, "Error applying low level settings CMD #o", ERR=err

        cpr = progress[0]+fix(0.75*(progress[1]-progress[0]))
        err = update_progress(cpr)
        if err ne adsec_error.ok then return, err

        err = set_a_delay_matrix(a_delay, FITS=a_delay_file)
        if err ne adsec_error.ok then log_message, "Error applying low level settings CMD #p", ERR=err

        cpr = progress[0]+fix(0.9*(progress[1]-progress[0]))
        err = update_progress(cpr)
        if err ne adsec_error.ok then return, err

        err = set_offload_cmd(cmd[adsec.act_w_pos], /NOCHECK, /SW, /DELTA)
        if err ne adsec_error.ok then log_message, "Error applying low level settings CMD #q", ERR=err

        err = set_offload_force(curr[adsec.act_w_curr], /NOCHECK, /START, /SW, /DELTA)
        if err ne adsec_error.ok then log_message, "Error applying low level settings CMD #q", ERR=err

        cpr = progress[0]+fix(0.95*(progress[1]-progress[0]))
        err = update_progress(cpr)
        if err ne adsec_error.ok then return, err

;        err = fsm_apply_cmd(cmd, /PASS, /DELTA)
;        if err ne adsec_error.ok then return, err


        err = set_diagnostic(OVER = old_ovs)
        if err ne adsec_error.ok then log_message, "Error applying low level settings CMD #r", ERR=err

;        pmhofor = comp_ho_offload_matrix(/AO)
;        cfile = getenv('ADOPT_LOG')+'/current_pmfor.fits',
;        writefits, cfile, pmhofor
;        !AO_STATUS.PMHOFOR = cfile
;        pm_offload.pmhofor =  pmhofor
        cpr = progress[1]
        err = update_progress(cpr)
        if err ne adsec_error.ok then return, err

        return, update_rtdb()

    endelse



end

