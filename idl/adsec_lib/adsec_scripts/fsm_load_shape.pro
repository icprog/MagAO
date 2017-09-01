Function fsm_load_shape, file2restore, NOPIST=nopist, NOTILT=notilt, DISPL=displ, DISABLE_ICC=disable_icc, ACT_LIST=act_list_tmp, FORCE_MAX_MODE=force_max_mode, NOSKIPRECOVERY=noskiprecovery, REMOVETILT=removetilt

    @adsec_common
    if n_elements(act_list_tmp) eq 0 then act_list= -2 else act_list = act_list_tmp
    if n_elements(file2restore) eq 0 then file2restore = ""

    info = file_info(file2restore)
    if not (info.exists and info.read and info.regular)  then begin
        log_message, 'Flat not found ' + file2restore, CONT = (sc.debug eq 0), ERR=adsec_error.flat_not_found
        return, adsec_error.flat_not_found
    endif
    
    if ~keyword_set(noskiprecovery) then begin
        err = fsm_skip_recovery(/FORCE, /NOLOADSHAPE)
        if err ne adsec_error.ok then return, err
    endif

    if sc.name_comm eq "Dummy" then return, adsec_error.ok
    err = update_status()
    if err ne adsec_error.ok then return, err

    ;CLEAR DISTURBANCE
    err = set_disturb(fltarr(rtr.disturb_len, adsec.n_actuators))
    if err ne adsec_error.ok then return, err
    
    err = set_disturb(/DIS, /RESET, /COUNT)
    if err ne adsec_error.ok then return, err

    wait, 0.1
    err = update_status()
    if err ne adsec_error.ok then return, err

;    if n_elements(file2restore) eq 0 then file2restore = file_search(filepath(ROOT=filepath(ROOT=adsec_path.conf, SUB='data', ''), SUB='flat', '*.sav'))
;    if n_elements(file2restore) eq 1 and file_basename(file2restore[0]) eq "default.sav" then begin
;      file2restore = file_search(filepath(ROOT=filepath(ROOT=adsec_path.conf, SUB='data', ''), SUB='flat', '*.sav'))
;    endif else begin
;        selected_flat = file2restore
;    endelse
    
    for i=0, n_elements(file2restore)-1 do begin

        if ~(file_test(file2restore[i], /READ, /REGULAR)) then begin
            log_message, 'Wrong filename provided '+file2restore[i], CONT = (sc.debug eq 0), ERR=adsec_error.flat_not_found
            return, !AO_ERROR
        endif

        log_print, 'Loading shape: '+ file2restore[i]
        undefine, flattened_status
        restore,file2restore[i], /VERB
        if n_elements(status_save) gt 0 and n_elements(flattened_status) eq 0 then flattened_status = status_save.sys_status
        if i eq 0 then begin
            sys_status_flat = flattened_status 
            maxf = max(abs(adsec.ff_matrix ## (sys_status_flat.position - sys_status.position)))
            current_flat = 0
            selected_flat = file2restore[0]
        endif else begin
            tmp = max(abs(adsec.ff_matrix ## (flattened_status.position - sys_status.position)))
            log_print, string(tmp, format = '(F5.3)')
            if tmp lt maxf then begin 
                maxf = tmp
                sys_status_flat = flattened_status
                selected_flat = file2restore[i]
                current_flat = i
            endif
        endelse
        
    
    endfor
    log_print, "Selected flat: "+selected_flat

    mode_idx = indgen(n_elements(adsec.act_w_cl))
    ; checks the mirror status
    if check_channel(adsec.act_w_cl,/noall) then begin 
        log_message,"No actuator valid for cl operation!",  CONT = (sc.debug eq 0), ERR=adsec_error.IDL_CLOOP_FLAT
        return, !AO_ERROR
    endif

    sys_act_w_cl = where(sys_status.closed_loop, count)
    flat_cl_act = where(sys_status_flat.closed_loop, count)
    err = intersection(flat_cl_act, sys_act_w_cl, inter)
    if err then begin
        log_message, "The actuators in cl are not all the ones available for cl operation or no closed loop actuators.",  CONT = (sc.debug eq 0), ERR=adsec_error.IDL_CLOOP_FLAT
        return, !AO_ERROR
    endif


;    null_dcomm = fltarr(adsec.n_actuators)
;    err = fsm_apply_cmd(null_dcomm, /PASSED)
;    if err ne adsec_error.ok then begin
;        log_message, "Error applying old flat command before mirror reflattening.",  CONT = (sc.debug eq 0), ERR=err
;        return, !AO_ERROR
;    endif

    ;RESTORE ORIGINAL OFFLOAD COMMANDS
    err = set_offload_cmd(sys_status.flat_offload_cmd[adsec.act_w_pos], /NOCHECK, /SW)
    if err ne adsec_error.ok then return, err

    err = set_offload_force(sys_status.flat_offload_curr[adsec.act_w_curr], /NOCHECK, /START, /SW)
    if err ne adsec_error.ok then return, err


    old_ovs = rtr.oversampling_time
    ;qui il catch e' costretto a disabilitare i coils perche' non si sa in che stato e' il sistema
    err_cnt = 0
    catch, Error_status
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
        err2 = adam_disable_coils()
        if err2 ne adsec_error.ok then begin
            log_print, "PANIC: NO ACTION CAN BE DONE!!!!!!"
            return, err2
        endif
        log_print, "Error catching procedure: coils succeffully disabled."
        return, err1
        
    endif

    ;here we could read the ff_ud_current after the ICC integration, fit low order modes
    ; and put them in the offload for having info for the weight changed for gravity (wind??)
    ; before reset ICC

    err = set_icc(/RESET)
    if err ne adsec_error.ok then log_message, "Error in low level procedure CMD #0a", ERR=err
    
    wait, 2*old_ovs > 0.1 ;to allow ICC to clean itself with zero gain

    ;this clean maybe can be avoided (fsm_skip_recovery did it?)
;;    err = set_diagnostic(OVER=0.0, TIMEOUT=1000.)
;;    if err ne adsec_error.ok then log_message, "Error in low level procedure CMD #0", ERR=err
;;    wait, 3*old_ovs 
;;;    ;CLEAN THE RECONSTRUCTOR DELAY LINES and the integrated FF CURRENTs
;;;    err = clear_dl()
;;;    if err ne adsec_error.ok then log_message, "Error in low level procedure CMD #0b", ERR=err
;;;    err = read_seq_ch(sc.all, dsp_map.ff_pure_current, 1L,ff_pure)
;;;    if err ne adsec_error.ok then log_message, "Error in low level procedure CMD #1a", ERR=err
;;;    err = write_same_chisc.all, dsp_map.ff_pure_current, 0., /CHECK)
;;;    if err ne adsec_error.ok then log_message, "Error in low level procedure CMD #1b", ERR=err
    
;;    ;cleaning the integrated control current contribution on feed-forward
;;    err = write_same_ch(sc.all_actuators, dsp_map.int_control_current, 0.0, /CHECK)
;;    if err ne adsec_error.ok then return, err

;;    err = set_diagnostic(OVER=old_ovs)
;;    if err ne adsec_error.ok then log_message, "Error in low level procedure CMD #3", ERR=err

;    log_print, "step # 0"
;;    wait, 2*old_ovs > 0.01
    err = zero_ctrl_current2()
    if err ne adsec_error.ok then log_message, "Error in low level procedure CMD #3a", ERR=err

    if ~keyword_set(disable_icc) then begin
        err = update_ovs(rtr.oversampling_time)
        if err ne adsec_error.ok then log_message, "Error in low level procedure CMD #3b", ERR=err
    endif
    
    err = zero_ctrl_current2()
    if err ne adsec_error.ok then log_message, "Error in low level procedure CMD #3c", ERR=err
;    log_print, "step # 1"
;    wait, (2*rtr.oversampling_time) > 0.1
    wait, 0.1

    catch, Error_status
    if Error_status ne 0 then begin

        log_print, !ERROR_STATE.MSG + " " +strtrim(!AO_ERROR, 2)
        return, !AO_ERROR
        
    endif

;    log_print, "step # 2"
    ;qui in poi sono errori vari ma non gravi
    err = update_status()
    if err ne adsec_error.ok then log_message,  "Error in low level procedure CMD #4", ERR=err


    sys_status_ini = sys_status

;;============================================================================
;; DA USARE SOLO IN CASO SI VOGLIA RIUTILIZZARE I VECCHI FLATTENING PRE-CALIBRAZIONE
;; INIZIO MODIFICA
;restore, adsec_path.data+"/calib_divisor.sav"
;sys_status_flat.position /= calib_divisor
;sys_status_flat.command /= calib_divisor
;; FINE MODIFICA
;;===========================================================================

    ; computes the commands to apply
;    filt_dcomm = filt_flat2(sys_status_flat, sys_status_ini, ACT_FILTERED=act_filtered)
    if act_list[0] eq -2 then undefine, act_list
    filt_dcomm = filt_flat7(sys_status_flat, sys_status_ini, ACT_FILTERED=act_filtered, ACT_LIST=act_list, /SKIP)
    ;filt_dcomm = filt_flat6(sys_status_flat, sys_status_ini, ACT_FILTERED=act_filtered, ACT_LIST=act_list)

    new_sys_status_flat = sys_status_flat
    new_sys_status_flat.position = sys_status_ini.position+filt_dcomm

    new_sys_status_flat_nopist = new_sys_status_flat
    aa=replicate(1.0,1,adsec.n_actuators)
    mm=transpose(adsec.ff_p_svec)##aa
    newpist=adsec.ff_p_svec[0:8,*]##mm[0,0:8]
    deltapist = mean(new_sys_status_flat.position[adsec.act_w_cl]) - mean(sys_status_ini.position[adsec.act_w_cl]) 
    if keyword_set(NOPIST) then begin
        new_sys_status_flat_nopist.position[adsec.act_w_cl] -= (deltapist*newpist[adsec.act_w_cl])
    endif 
    dcomm = new_sys_status_flat_nopist.position - sys_status_ini.position
    

    ;particular case: no data of flat on actual CL act
    ;maybe can be done before but you ha ve to take in account the error catching....
    if n_elements(inter) ne n_elements(adsec.act_w_cl) then begin
        err = complement(inter, adsec.act_w_cl, cl_not_match)
        if err gt 0 then log_message, "Error wrong list closed loop actuators",  ERR=adsec_error.IDL_CLOOP_FLAT
        print, cl_not_match
        dcomm[cl_not_match] = 0.
    endif



    ; checks for too large piston to apply
;    pos_piston = mean((dcomm*adsec.m_per_pcount)[adsec.act_w_cl])
;    piston = mean(replicate(pos_piston,adsec.n_actuators)/adsec.m_per_pcount)
    piston= mean(dcomm[adsec.act_w_cl])
    if abs(piston[0]) gt 2.8e-6 then log_message,"Piston requested to be applied is too apply too big returning", ERR=adsec_error.IDL_SAFE_PARAMS_OOB
 
    is_ok=0
    ;position threshold on find_opt_flat (delta position error wrt requested position)
    thr = 0.1e-9
    ;force threshold on find_opt_flat (maximum delta force applicable)
    fth = 0.25
    ;fth = 0.15
    ;maximum absolute force applicable on command application
    fth1 = (max(abs(sys_status_flat.current)) + 0.1) > 0.74

    max_mode=10
    log_print, "Delta command modal filtering for reducing peak force to "+string(fth, FORMAT='(F4.2)')+ " [N]"
    find_opt_flat, sys_status_ini, new_sys_status_flat_nopist, mode_idx, dcomm_fo, FORCE_MAX_MODE=force_max_mode $
                     , THR=thr, MAX_MODE=max_mode, NOT_MATCH_ACT=cl_not_match, FORCE_THR = fth  ; max mode has been set to 10
    log_print,"Maximum mode index used to apply flat -> # "+strtrim(max_mode,2)

    dforce = adsec.ff_matrix ## reform(dcomm_fo,1,adsec.n_actuators)
    tot_force = sys_status_ini.current+dforce
    if (max(abs(tot_force), idx_peak) lt fth1) then is_ok=1
    log_print,"Min/Max delta command to apply for flattening -> # "+string(min(dcomm_fo)) + ' /  ' +string(max(dcomm_fo)) + ' [m]'

    if keyword_set(REMOVETILT) then begin
        tmp = dcomm_fo
        coeff= pm_offload.pmpos ## transpose(tmp)
        tilt = pm_offload.pmz[1:2, *] ## coeff[1:2]
;        dcomm_fo = tmp - tilt
        new_sys_status_flat_nopist.position -= tilt
        find_opt_flat, sys_status_ini, new_sys_status_flat_nopist, mode_idx, dcomm_fo, FORCE_MAX_MODE=force_max_mode $
                         , THR=thr, MAX_MODE=max_mode, NOT_MATCH_ACT=cl_not_match, FORCE_THR = fth  ; max mode has been set to 10
        log_print,"Maximum mode index used to apply flat -> # "+strtrim(max_mode,2)
        log_print,"Min/Max delta command to apply for flattening after tilt removal-> # "+string(min(dcomm_fo)) + ' /  ' +string(max(dcomm_fo)) + ' [m]'

        dforce = adsec.ff_matrix ## reform(dcomm_fo,1,adsec.n_actuators)
        tot_force = sys_status_ini.current+dforce
        if (max(abs(tot_force), idx_peak) lt fth1) then is_ok=1
    endif


    if is_ok then begin

        ;err = complement(adsec.dummy_act, adsec.act_wo_pos, wo_pos_not_dummy)
        ;if err ne 1 then begin
        ;     err = intersection(wo_pos_not_dummy, adsec.act_w_curr, wo_pos_w_curr)
        ;     if err ne 1 then begin
         ;       log_print, "Set flat bias current to wo_pos actuators"
        ;        vv = fltarr(672)
        ;        vv[wo_pos_w_curr] = -0.25 > (new_sys_status_flat_nopist.current[wo_pos_w_curr] < 0.25)
        ;        err = set_offload_force(vv[adsec.act_w_curr], /DELTA, /SWITCH, /START)
        ;        if err ne adsec_error.ok then return, err
        ;     endif
        ;endif

        err = fsm_apply_cmd(dcomm_fo, /PASSED, /DELTA)
        if err ne adsec_error.ok then log_message, "Error in low level procedure CMD #5", ERR = err
        !AO_STATUS.shape = file2restore[current_flat]
        err = update_rtdb()
        if err ne adsec_error.ok then log_message, "Error in low level procedure CMD #6", ERR = err

    endif else begin

        log_message, "Out of threshold request. Not possible to apply a flat command", ERR=adsec_error.IDL_SAFE_PARAMS_OOB

    endelse

;    log_print, "step # 4"
    wait, (2*rtr.oversampling_time) > 0.1
;    log_print, "step # 5"
    err = update_status()
    if err ne adsec_error.ok then log_message, "Error in low level procedure CMD #7", ERR = err

    if  xregistered('xadsec') gt 0 then  begin
        update_panels, /OPT, /NO_READ
    endif

    sys_status.cflat_offload_cmd = sys_status.offload_cmd
    sys_status.cflat_offload_curr = sys_status.offload_curr
    pos_vector = sys_status.position
    cur_vector = sys_status.current
    
    ;updating RTDB variables
    !AO_STATUS.REF_POS = pos_vector
    !AO_STATUS.REF_FOR = cur_vector
    !AO_STATUS.LAST_FLAT_LOADED = retrieve_shortname(!AO_STATUS.shape)
    !AO_STATUS.zern_applied = fltarr(22)
    
    flattened_status = sys_status
    file2save = file_basename(file2restore[current_flat])
    save, file=add_timestamp(filepath(ROOT=adsec_path.meas, SUB=['CMD', 'shape'],file2save), 'stability'), flattened_status, sys_status_ini, act_filtered
    save, file=filepath(ROOT=adsec_path.meas, SUB=['CMD', 'shape'],'current_ref_pos.sav'), pos_vector, cur_vector
    return, update_rtdb()

end
