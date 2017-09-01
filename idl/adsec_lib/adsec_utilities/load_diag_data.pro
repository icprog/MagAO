Function load_diag_data, numframes, data_reduced, SHARED_MEM=shared_mem, SWITCHB=switchb, BCU=bcu, READFC=readfc, SKIP=skip, NOFAST=nofastcheck
;default SDRAM in DSP
    @adsec_common
    if keyword_set(SHARED_MEM) then begin
        case 1 of
            keyword_set(SWITCHB) : begin
                shm_name = "switch_shm"
                sized = (rtr.n_slope+16) *4
                type = 1
            end
            keyword_set(BCU) : begin
                shm_name = "bcu_shm"
                sized = (8+68*8) *4
                type = 1
            end
            else: begin
                log_print, "You must specify the kind of diagnostic frame"
                return, adsec_error.input_type
            endelse

        endcase
    
        err = smread(shm_name, sized, type, data, /ulong)
        if err ne adsec_error.ok then return, err

        buf = gen_fastdiag_template(BCU=bcu, SWITCHB=switchb)
    
        case 1 of
            keyword_set(BCU): begin
                ;BODY
                For i=0, adsec.n_bcu-1 do begin
                    fill_fastdiag_dsp, data, data_reduced,1, /NOHEAD, BCU=i
                endFor
                ;HEADER
                data_reduced.wfs_frames_counter = data[0]
                data_reduced.param_selector = data[1]
                data_reduced.mirror_frames_counter = data[2]
                ;FOOTER
                data_reduced.wfs_frames_counter_check = data[n_elements(data)-3]
                data_reduced.param_selector_check = data[n_elements(data)-2]
                data_reduced.mirror_frames_counter_check = data[n_elements(data)-1]
            end
            keyword_set(SWITCHB): begin
                ;HEADER, BODY, FOOTER
                fill_fastdiag, data, data_reduced,1, /NOHEAD, /SWITCHB
            end
        
        endcase
    

        return, adec_error.ok

    endif else begin

        case 1 of
            keyword_set(BCU): begin
                err = load_diag_data_bcu(numframes, data_reduced, READFC=readfc, SKIP = skip, NOFASTCHECK=nofastcheck)
                if err ne adsec_error.ok then return,err
            end
            keyword_set(SWITCHB): begin
                err = load_diag_data_switch(numframes, data_reduced, READFC=readfc, SKIP = skip, NOFASTCHECK=nofastcheck)
                if err ne adsec_error.ok then return,err
            end
            else: begin
                err = load_diag_data_dsp(numframes, data_reduced, READFC=readfc, SKIP = skip, NOFASTCHECK=nofastcheck)
                if err ne adsec_error.ok then return,err
            end
        endcase

    endelse

    return, adsec_error.ok
 

End
