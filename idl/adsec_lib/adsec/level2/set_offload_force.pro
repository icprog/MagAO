;$Id: set_offload_force.pro,v 1.4 2008/08/25 14:16:48 labot Exp $$
;+
;   NAME:
;    SET_OFFLOAD_FORCE
;
;   PURPOSE:;    write in a memory block the force vector to offload.
;
;   USAGE:
;    err = set_offload_force(force, [/RESET_ALL, /NOCHECK, /DSP,$
;                                    /SWITCHB, /START, /DELTA])
;
;   INPUT:
;    force: force vector to apply (its length must be equal to adsec.n_act_w_curr)
;    
;   OUTPUT:
;    Error code.
;
;   KEYWORD:
;    RESET_ALL: if set clear all memory location of cmd offload.
;    SWITCHB:   read current block from SwitchBCU
;    NOCHECK:   no check for delta command
;    DSP:       read current block from DSP boards
;    START:     start the offload application in the next step
;    DELTA:     the passed command is an increment of the currently
;               applied offload command
;
;   NOTE:
;    None.
;   
; HISTORY
;   Created on 24 Jan 2007 by Marco Xompero
;   marco@arcetri.astro.it
;   3 April 2007, MX
;   Added the reading of block in use,DSP  keyword, and the Keyword Start to start the exact offloading command
;       block.
;   4 Nov 2007, MX
;       START, SWITCHB, NOCHECK keywords added.
;
;   14 Aug 2008, A. Riccardi (AR)
;       DELTA keyword added
;-

Function set_offload_force, force_tmp, RESET_ALL=reset_all, NOCHECK=nocheck, START=start, DSP=is_dsp, SWITCHB=is_switch, DELTA=is_delta


    ;Input parameters checks
    @adsec_common
    if keyword_set(reset_all) then force=fltarr(n_elements(adsec.act_w_curr)) else force=reform(force_tmp)

    ;mode list data type verification
    if test_type(force, /FLOAT, N_EL=nel) then begin
        message, "Wrong format of forces vector. It should be a float array. No force to offload written.", CONT = (sc.debug eq 0)
        return, adsec_error.input_type
    endif 

    ;mode list format verification
    if n_elements(force) ne n_elements(adsec.act_w_curr) then begin
        message, "Wrong format of forces vector. The length of the passed array must be equal to adsec.n_act_w_curr. No force to offload written.", CONT = (sc.debug eq 0)
        return, adsec_error.input_type
    endif 


    ;reset all loop gains (A and B block)
    f2w = fltarr(adsec.n_actuators)
    f2w[adsec.act_w_curr] = force
    if keyword_set(reset_all) then begin 
        
        err = write_seq_ch(sc.all, dsp_map.fmol_a, f2w, /CHECK) 
        if err ne adsec_error.ok then return, err
        err = write_seq_ch(sc.all, dsp_map.fmol_b, f2w, /CHECK) 
        return, err

    endif else begin 

        ;read the current block used
        if keyword_set(is_dsp) then begin
            err = read_seq_dsp(sc.all, rtr_map.param_selector, 1l, buf, /ULONG) 
            if err ne adsec_error.ok then return, err
                                ;extract block bit
            block_of = (buf[0] && 2^13)        
            if total(buf - buf[0]) ne 0 then begin
                message, "Param selector mismatch. No actions done.", CONT = (sc.debug eq 0)
                return, adsec_error.idl_pbs_mismatch
            endif

        endif else begin
            if keyword_set(is_switch) then begin

                err = read_seq_dsp(0,switch_map.OffloadSelector, 1L, block_of, /SWITCH, /UL)
                if err ne adsec_error.ok then return, err

            endif else begin
            
                err = getdiagnvalue(process_list.fastdiagn.msgd_name, 'ParamBlockOffloadBlock', 0, 0, block_of_tmp, tim=0.5)
                if err ne adsec_error.ok then return, err
                block_of = block_of_tmp.last
                
            endelse
            

        endelse

        
                                ;extract block bit
        
        if ~keyword_set(NOCHECK) then begin

            if (block_of) then begin
                err = read_seq_ch(sc.all, dsp_map.fmol_b, 1l, old_f2w)
                if err ne adsec_error.ok then return, err
            endif else begin
                err = read_seq_ch(sc.all, dsp_map.fmol_a, 1l, old_f2w)
                if err ne adsec_error.ok then return, err
            endelse
            
            err=update_status()
            if err ne adsec_error.ok then return,err
            
            if total(abs(sys_status.current[adsec.act_w_curr] + force - old_f2w[adsec.act_w_curr]) gt max(abs([adsec.pos_sat_force, adsec.neg_sat_force]))) gt 1 then begin
                message, "Wrong format of forces vector. The final force is too high. No force to offload written.", CONT = (sc.debug eq 0)
                return, adsec_error.input_type
                
            endif
        endif
        
        if keyword_set(is_delta) then begin
            if (block_of) then begin
                err = read_seq_ch(sc.all, dsp_map.fmol_b, 1l, old_f2w)
                if err ne adsec_error.ok then return, err
            endif else begin
                err = read_seq_ch(sc.all, dsp_map.fmol_a, 1l, old_f2w)
                if err ne adsec_error.ok then return, err
            endelse
        endif else begin
            old_f2w = fltarr(adsec.n_actuators)
        endelse

        if (block_of) then begin
            err = write_seq_ch(sc.all, dsp_map.fmol_a, f2w+old_f2w, /CHECK)
            if err ne adsec_error.ok then return, err
        endif else begin
            err = write_seq_ch(sc.all, dsp_map.fmol_b, f2w+old_f2w, /CHECK)
            if err ne adsec_error.ok then return, err
        endelse
        
    endelse    

    if keyword_set(START) then begin

        if block_of eq 1 then selector = 0UL else selector = 1UL
        err = write_same_dsp(0,switch_map.OffloadSelector, selector, /SWITCH, /CHECK)
        if err ne adsec_error.ok then return, err

    endif


        
    return,adsec_error.ok

end
