;dtime: polling period
;iterations: how many iteration to do. 0 means infinite loop

Function comp_offload_modes,  iterations, dtime, OFFLOAD=offload

    @adsec_common

    if n_elements(dtime) eq 0 then dtime = 1.
    if n_elements(iterations) eq 0 then iterations = 1000L

    modes = dblarr(adsec.n_actuators)
    ref_pos = fltarr(adsec.n_actuators)
    ref_m2c = float(adsec.ff_p_svec * 0)
    dposmodes_ref = fltarr(adsec.n_actuators)
    
    ;START CYCLING
    log_print, "Start cycle"
    for i = 0L , iterations -1 do begin

        log_print, "Read reference position"
        restore, filepath(ROOT=adsec_path.meas, SUB=['CMD', 'shape'],'current_ref_pos.sav')
        log_print, "Read M2C matrix"
        restore, filepath(ROOT=adsec_path.meas, SUB=['CMD', 'shape'],'current_ref_m2c.sav')

        if total(abs(ref_pos-pos_vector)) ne 0 then begin
            log_print, "Reference position changed"
            ref_pos = pos_vector
            update_file = 1
        endif

        if total(abs(ref_m2c-m2c)) ne 0 then begin
            log_print, "M2C matrix changed"
            ref_m2c = m2c
            m2c_mat = ref_m2c
            log_print, "Pseudo invert m2c matrix"
            tmp =  total(total(abs(m2c_mat), 2), /CUM)
            nmodes = ((where(deriv(tmp) eq 0))[0] -1) < (adsec.n_actuators - 20)
            m2c_r = m2c_mat[0:nmodes-1,*]
            m2c_inv = pseudo_invert(m2c_r)
            log_print, "Done"
            update_file = 1
        endif

        if update_file then begin
            if i gt 0 then begin
                free_lun, unit
                log_print, current_file + ": closed."
            endif
            ref_pos_file = meas_path('decomposition')+add_timestamp('m2c_ref_pos.sav')
            save, file = ref_pos_file, ref_pos, ref_m2c
            log_print, "New ref_pos saved into " +ref_pos_file
            current_file = ref_pos_file+'.dat'
            log_print, "New decomposition data will be saved into " + current_file
            openw, unit, current_file, /GET
        endif

        err = getdiagnvalue(process_list.fastdiagn.msgd_name, 'ChCurrAverage ChDistAverage mirrorframecounter Modes wfsframecounter ', -1, -1, bb) 
        if err ne adsec_error.ok then return, err

        currave = bb[0:adsec.n_actuators]
        distave = bb[adsec.n_actuators:2*adsec.n_actuators-1]
        wfc = bb[2*adsec.n_actuators]
        modes   = bb[2*adsec.n_actuators+1:3*adsec.n_actuators]
        mfc = bb[3*adsec.n_actuators+1]
        err = intersection(where(finite(modes.average)), where(modes.average), rmodes)
        if err gt 0 then begin
            rmodes = -1
            nrmodes = indgen(adsec.n_actuators)
        endif else begin
            err = complement(rmodes, indgen(adsec.n_actuators), nrmodes)
        endelse
        dpos = dblarr(adsec.n_actuators)
        dpos[adsec.act_w_cl] = (reform(distave.average)-ref_pos)[adsec.act_w_cl]
        dposmodes = dblarr(adsec.n_actuators)
        dposmodes[0:nmodes-1] = (m2c_inv ## transpose(dpos))
        if update_file then dposmodes_ref = dposmodes
        dposmodes2disp = dposmodes
        dposmodes2disp[0:5] = 0
        plot, indgen(adsec.n_actuators)+1, dposmodes2disp, psym=-4. , TITLE='Modes decomposition (current pos - flat pos)', XS=17, YR=[-20e-9, 20e-9]
        if nrmodes[0] ge 0 then oplot, nrmodes+1, dposmodes2disp[nrmodes], psym=4., col=255L
        dummy = [distave.average, currave.average, modes.average, dposmodes, wfc.last, mfc.last]
        writeu, unit, dummy
        print, "Press 's' to Stop the process"
        str = get_kbrd(0)
        if str eq "s" then break
        update_file = 0
        wait, 1
        
    endfor
    free_lun, unit
    log_print, current_file + ": closed."

    if keyword_set(OFFLOAD) then begin
        err  = 1
    endif
        
    return, err

End
