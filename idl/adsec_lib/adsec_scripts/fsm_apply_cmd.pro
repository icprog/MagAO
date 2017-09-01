Function fsm_apply_cmd, dcomm_file, PASSED=passed, DELTA=delta, HOLD=hold, FORCE=force, NOFF=noff, WITHPISTON=withpiston
    
    @adsec_common
    if ~keyword_set(PASSED) then begin
        dc_exists  = (file_info(dcomm_file)).exists     
        if ~dc_exists then begin
            log_message, "The provided file does not exist.", CONT = (sc.debug eq 0), ERR=adsec_error.invalid_file
            return, !AO_ERROR
        endif

        dcomm = readfits(dcomm_file)
    endif else begin
        dcomm = dcomm_file
    endelse

    if n_elements(dcomm) eq n_elements(adsec.act_w_cl) then begin
    endif else begin
        if n_elements(dcomm) ne adsec.n_actuators then begin
            log_message, "The command to apply has wrong format.(must be 1x672, FLOAT)", CONT = (sc.debug eq 0), ERR=adsec_error.IDL_INPUT_TYPE
            return, !AO_ERROR
        endif
        cl_list = where(dcomm ne 0)
        if ~intersection(cl_list, adsec.act_wo_cl, out_list) then begin
            log_print, "WARNING : you should not apply require commands to not working actuators. ", log_lev = !AO_CONST.log_lev_warning
            dcomm[adsec.act_wo_cl] = 0
        endif
    endelse


    if sc.name_comm eq "Dummy" then return, adsec_error.ok
    
    dummy = float(reform(dcomm))
    dcomm = transpose(dummy) 
    
    if ~keyword_set(WITHPISTON) then begin
        dcomm[adsec.act_w_cl] -= mean(dcomm[adsec.act_w_cl])
        log_print, "Piston removed from commands"
    endif else begin
        log_print, "Piston NOT removed from commands"
    endelse
        
    err = update_status()
    if err ne adsec_error.ok then return, err


    if keyword_set(DELTA) then begin

        final_cmd = dcomm + sys_status.command
        dcurr = adsec.ff_matrix ## dcomm 
        final_curr = dcurr + sys_status.current 
        out_stroke  = (max(final_cmd[adsec.act_w_cl]) gt 135e-6)  or (min(final_cmd[adsec.act_w_cl]) lt 33e-6)
        out_force  = max(abs(final_curr[adsec.act_w_cl])) gt 0.78
        
        if keyword_set(FORCE) then begin
            out_stroke =0
            out_force =0
        endif

        if (out_stroke) then begin
            log_message, "Too much stroke required. No command applied.", CONT = (sc.debug eq 0), ERR=adsec_error.IDL_STROKE_REQUIRED
            return, !AO_ERROR
        endif
        
        if (out_force) then begin
            log_message, "Too much force required. No command applied.", CONT = (sc.debug eq 0), ERR=adsec_error.IDL_FORCE_REQUIRED
            return, !AO_ERROR
        endif
        
        

        log_print, "COMM  MAX [m] "+strtrim(max(final_cmd), 2)
        log_print, "FORCE MAX [N] "+strtrim(max(abs(final_curr[adsec.act_w_cl])), 2)

        if keyword_set(NOFF) then begin        
            err = set_offload_cmd(dcomm[adsec.act_w_pos],/DELTA, /NOCHECK, /SW, /START)
            if err ne adsec_error.ok then return, err
        endif else begin        
            err = set_offload_cmd(dcomm[adsec.act_w_pos],/DELTA, /NOCHECK, /SW)
            if err ne adsec_error.ok then return, err
        
            err = set_offload_force(dcurr[adsec.act_w_curr], /DELTA, /NOCHECK, /START, /SW)
            if err ne adsec_error.ok then return, err
        endelse
        
    endif else begin

        final_cmd = dcomm +  sys_status.cflat_offload_cmd + sys_status.bias_command + $
                    (sys_status.full_bias_command - sys_status.bias_command - sys_status.cflat_offload_cmd)
        offload_cmd = dcomm + sys_status.cflat_offload_cmd
        dcurr = adsec.ff_matrix ## dcomm 
        offload_curr = dcurr + sys_status.cflat_offload_curr
        final_curr = dcurr + sys_status.current 
        out_stroke  = (max(final_cmd[adsec.act_w_cl]) gt 135e-6)  or (min(final_cmd[adsec.act_w_cl]) lt 33e-6)
        out_force  = max(abs(final_curr[adsec.act_w_cl])) gt 0.78
        if keyword_set(FORCE) then begin
            out_stroke =0
            out_force =0
        endif
       
        if (out_stroke) then begin
            log_message, "Too much stroke required. No command applied.", CONT = 1, ERRCODE=adsec_error.IDL_STROKE_REQUIRED
            return, !AO_ERROR
        endif
        
        if (out_force) then begin
            log_message, "Too much force required. No command applied.", CONT = 1 , ERRCODE=adsec_error.IDL_FORCE_REQUIRED
            return, !AO_ERROR
        endif
        
        
;        log_print, "COMM  MAX [m] "+strtrim(max(final_cmd), 2)
;        log_print, "FORCE MAX [N] "+strtrim(max(abs(final_curr[adsec.act_w_cl])), 2)
        

        if keyword_set(NOFF) then begin
            err = set_offload_cmd(offload_cmd[adsec.act_w_pos], /NOCHECK, /SW, /START)
            if err ne adsec_error.ok then return, err
        endif else begin
        
            err = set_offload_cmd(offload_cmd[adsec.act_w_pos], /NOCHECK, /SW)
            if err ne adsec_error.ok then return, err
        
            err = set_offload_force(offload_curr[adsec.act_w_curr], /NOCHECK, /START, /SW)
            if err ne adsec_error.ok then return, err

        endelse

    endelse
    
    wait, 2*rtr.oversampling_time > 0.1

    err = update_status()
    if err ne adsec_error.ok then return, err        

    if keyword_set(HOLD) then begin
         sys_status.cflat_offload_cmd = sys_status.offload_cmd
         sys_status.cflat_offload_curr = sys_status.offload_curr

    endif

    if  xregistered('xadsec') gt 0 then  begin
        update_panels, /OPT, /NO_READ
    endif


    return, adsec_error.ok
    
End
