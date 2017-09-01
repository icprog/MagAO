;$Id: save_opt_params.pro,v 1.2 2007/02/07 17:21:42 marco Exp $
;+
; HISTORY
;   07 Feb 2007 MX
;       LUT of variable deleted and name refurbished.
;-

;save status of optical loop
Function save_opt_params, fc, pbs, SEND=send

    @adsec_common
    if keyword_set(SEND) then begin
        
        if (n_elements(fc) eq 0) or (n_elements(fc) ne adsec.n_dsp_per_board*adsec.n_board_per_bcu*adsec.n_bcu) then begin
            message, 'Wrong wfs_frames_counter or param_selector inserted.', CONT=(sc.debug eq 0)
            return, err
        endif
        if n_elements(fc) eq 1 then fc = replicate(fc, adsec.n_dsp_per_board*adsec.n_board_per_bcu*adsec.n_bcu)

        if (n_elements(pbs) eq 0) or (n_elements(pbs) ne adsec.n_dsp_per_board*adsec.n_board_per_bcu*adsec.n_bcu) then begin
            message, 'Wrong wfs_frames_counter or param_selector inserted.', CONT=(sc.debug eq 0)
            return, err
        endif
        if n_elements(pbs) eq 1 then pbs = replicate(pbs, adsec.n_dsp_per_board*adsec.n_board_per_bcu*adsec.nbcu)

        err = write_seq_dsp(sc.all, rtr_map.wfs_frames_counter, fc)
        if err ne adsec_error.ok then return, err

        err = write_seq_dsp(sc.all, rtr_map.param_selector, pbs)
        if err ne adsec_error.ok then return, err

    endif else begin

        err = read_seq_dsp(sc.all, rtr_map.wfs_frames_counter,1l, fc)
        if err ne adsec_error.ok then return, err

        err = read_seq_dsp(sc.all, rtr_map.param_selector, 1l, pbs)
        if err ne adsec_error.ok then return, err

    endelse
    return, adsec_error.ok

End
