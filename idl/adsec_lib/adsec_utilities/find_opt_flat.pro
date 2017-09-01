;+
;HISTORY
; Changed some limits for the new electronic.
;
; 14 Aug 2008 A.Riccardi
;    removed redundant code lines
;    fixed
;-
pro find_opt_flat, sys_ini, sys_flat, mode_idx, opt_dcomm, COMPENSATE_FORCE=cf $
	, VERBOSE=verbose, THRESHOLD=thr, MAX_MODE=max_mode, FORCE_MAX_MODE=force_max_mode  $
    , FORCE_THR = force_thr, NOT_MATCH_ACT = not_match_act

    @adsec_common
    
    if n_elements(thr) eq 0 then thr=10d-9
    adsec_used = adsec
    
    n_mode_idx = n_elements(mode_idx)
    
    
    cl_act = adsec_used.act_w_cl
    dcomm = sys_flat.position-sys_ini.position
    if n_elements(not_match_act) gt 0 then dcomm[not_match_act] = 0
    pos_piston = mean(dcomm[cl_act])
    
    ;;; TODO: ADD THE AUTOMATIC PISTON REMOVING IF TOO LARGE
    if abs(pos_piston) gt 3e-6 then $
      message,"Piston to apply too big..."


    if keyword_set(cf) then begin

        dforce = sys_flat.current - sys_ini.current
        dforce_m = dforce - mean(dforce[cl_act])
        inv_ff = pseudo_invert(adsec.ff_matrix, EPS=1e-2, /ver)
        opt_dcomm = dcomm - inv_ff ## dforce_m

    endif else begin

        dcomm_mod = transpose(adsec_used.ff_p_svec[mode_idx,*])##transpose(dcomm)
        
        force_vec = fltarr(adsec_used.n_actuators, n_mode_idx)
        comm_filt_vec = fltarr(adsec_used.n_actuators,n_mode_idx)
        
        for i=0,n_mode_idx-1 do begin
            dummy_p = adsec_used.ff_p_svec[mode_idx[i],*]*dcomm_mod[i]
            dummy_f = adsec_used.ff_sval[mode_idx[i]]*adsec_used.ff_f_svec[mode_idx[i],*]*dcomm_mod[i]
            if i eq 0 then begin
                comm_filt_vec[*,i]=dummy_p
                ;force_vec[*,i]=dummy_f + sys_ini.current
                force_vec[*,i]=dummy_f 
            endif else begin
                comm_filt_vec[*,i]=comm_filt_vec[*,i-1]+dummy_p
                force_vec[*,i]=force_vec[*,i-1]+dummy_f
            endelse
            
        endfor
        
        
        ;NOTE: res_rms and force_rms are computed WITHOUT removing the average value.
        res_filt_vec = comm_filt_vec-rebin(dcomm, adsec_used.n_actuators, n_mode_idx)
        res_rms = sqrt(total(res_filt_vec[cl_act,*]^2,1)/n_elements(cl_act))
        force_rms = sqrt(total(force_vec[cl_act,*]^2,1)/n_elements(cl_act))
        if keyword_set(force_max_mode) then begin
            max_mode = force_max_mode
        endif else begin

            if keyword_set(FORCE_THR) then begin

                max_force = fltarr(n_mode_idx)
                for i=0,n_mode_idx-1 do max_force[i]=max(abs(force_vec[cl_act,i]))
                save,max_force,file='/tmp/max_force.sav'
                idf= where(max_force gt abs(force_thr), cc)
                idf = (idf[0]-1) > 0
                if cc[0] eq 0 then max_mode = n_mode_idx-1 else max_mode = idf

            endif else begin

                dd = min(abs(res_rms-thr),idx)
                max_mode = idx

            endelse
        endelse

        opt_dcomm= comm_filt_vec[*,max_mode]
    
    endelse



    if keyword_set(verbose) then begin
        window,/free
        plot,res_rms,force_rms,psym=-4,/xl,xtit='Residual rms error',ytit='Force rms'
        oplot, thr*[1,1], minmax(force_rms)
        window,/free
        plot,res_rms,max_force,/xl,psym=-4,xtit='Residual rms error',ytit='Peak abs force'
        oplot, thr*[1,1], minmax(max_force)
    endif

end
