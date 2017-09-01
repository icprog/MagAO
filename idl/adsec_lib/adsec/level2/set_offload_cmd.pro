;$Id: set_offload_cmd.pro,v 1.6 2008/10/02 13:49:05 labot Exp $$
;+
;   NAME:
;    SET_OFFLOAD_CMD
;
;   PURPOSE:;    write in a memory block the command vector to offload as subtraction.
;
;   USAGE:
;    err = set_offload_cmd(cmd, [/RESET_ALL, /NOCHECK, /DSP, /SWITCHB,$
;                                /START, /DELTA])
;
;   INPUT:
;    cmd: cmd vector to apply (its length must be equal to adsec.n_act_w_pos)
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
;       Added the reading of block in use, DSP keyword, and the Keyword Start to start the exact offloading command
;       block.
;   4 Nov 2007, MX
;       START, SWITCHB, NOCHECK keywords added.
;
;   14 Aug 2008, A. Riccardi (AR)
;       DELTA keyword added
;-

Function set_offload_cmd, cmd_tmp, RESET_ALL=reset_all, NOCHECK=nocheck, DSP=is_dsp, START=start $
  , SWITCHB=is_switch, DELTA=is_delta


    ;Input parameters checks
    @adsec_common
    if keyword_set(reset_all) then cmd=fltarr(n_elements(adsec.act_w_pos)) else cmd=reform(cmd_tmp)

    ;mode list data type verification
    if test_type(cmd, /FLOAT, N_EL=nel) then begin
        message, "Wrong format of cmds vector. It should be a float array. No cmd to offload written.", CONT = (sc.debug eq 0)
        return, adsec_error.input_type
    endif 

    ;mode list format verification
    if n_elements(cmd) ne n_elements(adsec.act_w_pos) then begin
        message, "Wrong format of cmds vector. The length of the passed array must be equal to adsec.n_act_w_pos. No cmd to offload written.", CONT = (sc.debug eq 0)
        return, adsec_error.input_type
    endif 

    if ~keyword_set(NOCHECK) then begin

        err=update_status()
        if err ne adsec_error.ok then return,err

        cond_a = total(((sys_status.position[adsec.act_w_pos] + cmd) lt (adsec.nominal_gap*0.5))) gt 0 
        cond_b = total(((sys_status.position[adsec.act_w_pos] + cmd) gt (adsec.nominal_gap*2.0))) gt 0
        if cond_a or cond_b then begin
            message, "Wrong format of cmds vector. The final cmd is too high. No cmd to offload written.", CONT = (sc.debug eq 0)
        return, adsec_error.input_type

        endif
    endif

    ;reset all loop gains (A and B block)
    c2w = fltarr(adsec.n_actuators)
    c2w[adsec.act_w_pos] = cmd
    if keyword_set(reset_all) then begin 
        
        err = write_seq_ch(sc.all, dsp_map.cmol_a, c2w, /CHECK) 
        if err ne adsec_error.ok then return, err
        err = write_seq_ch(sc.all, dsp_map.cmol_b, c2w, /CHECK) 
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

                err = read_seq_dsp(0,switch_map.OffloadSelector, 1l, block_of, /SWITCH, /UL)
                if err ne adsec_error.ok then return, err
                

            endif else begin


                err = getdiagnvalue(process_list.fastdiagn.msgd_name, 'ParamBlockOffloadBlock', 0, 0, block_of_tmp, tim=0.1)
                if err ne adsec_error.ok then return, err
                block_of = block_of_tmp.last

            endelse

        endelse

        if keyword_set(is_delta) then begin
            if (block_of) then begin
                err = read_seq_ch(sc.all, dsp_map.cmol_b, 1l, c2w0)
                if err ne adsec_error.ok then return, err
            endif else begin
                err = read_seq_ch(sc.all, dsp_map.cmol_a, 1l, c2w0)
                if err ne adsec_error.ok then return, err
            endelse
        endif else begin
            c2w0 = fltarr(adsec.n_actuators)
        endelse
        
        if ~keyword_set(NOCHECK) then begin
            err=update_status()
            if err ne adsec_error.ok then return,err
            final_pos = (sys_status.position+c2w+c2w0)[adsec.act_w_pos]
            cond_a = total(final_pos lt (adsec.nominal_gap*0.5)) gt 0 
            cond_b = total(final_pos gt (adsec.nominal_gap*2.0)) gt 0
            if cond_a or cond_b then begin
                message, "Wrong format of cmds vector. The final cmd is too high. " $
                         +"No cmd to offload written.", CONT = (sc.debug eq 0)
                return, adsec_error.input_type
            endif
        endif

        if (block_of) then begin
            err = write_seq_ch(sc.all, dsp_map.cmol_a, c2w+c2w0, /CHECK)
            if err ne adsec_error.ok then return, err
        endif else begin
            err = write_seq_ch(sc.all, dsp_map.cmol_b, c2w+c2w0, /CHECK)
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
