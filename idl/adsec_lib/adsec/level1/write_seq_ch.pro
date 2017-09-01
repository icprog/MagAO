; $Id: write_seq_ch.pro,v 1.11 2007/04/11 13:05:01 labot Exp $

;+
; WRITE_SEQ_CH
;
; err = write_seq_ch(act_list, addr_ch0, buf [, SET_PM=set_pm][, /CHECK])
;
; act_list:   int vector[N] or scalar. Vector of actuator ID numbers.
;             Use sc.all as shortcut for "all actuators".
;             (0 <= act_list <= adsec.n_actuators-1)
;
; addr_ch0:   long scalar. Address of the channel zero to write to.
;             Even if the target actuator is not the first channel of
;             the DSP, the address must be the one of the channel zero
;             in the DSP. The program computes the correct offset
;             for channels different from the channel zero. That allows
;             to address any channel in act_list using the same address.
;
; buf:        Float/long/ulong vector[n_word,N]. Buffer to write. n_word is the number
;             of words to write per actuator.
;
; KEYWORDS
;
; SET_PM:     byte scalar. Code identifing the type of memory to
;             write to. Default value is dsp_datasheet.data_mem_id,
;             it is used to write to DSP data memory.
;             Set SET_PM to dsp_datasheet.prog_mem_id to write
;             to program memory (if available)
;
; CHECK:      If it is set, the written data are read back to check
;             if the data have been written correctly
;
; HISTORY
;
; Written by A. Riccardi (AR)
; Osservatorio Astrofisico di Arcetri, ITALY
; riccardi@arcetri.astro.it
;
; 13 Mar 2002, AR
;   n_dsp>256 is now handled
;
; 03 May 2004, AR
;   Compatibility to LBT hardware:
;     * dsp_list, addr, n_words NOFLOAT
;     * dsp_list is casted internally to INT (adsec.n_dsp < 2^15)
;     * read data are stored in a float array. Float type is
;       good for LBT (32 bit float DSP) as for MMT (float has 24bit
;       mantissa, that can be used without loss for data memory, 16bit
;       integer data, for program memory, 24 bit integer data)
;
;   20 May 2004, AR
;     ulong and long buffer are now allowed (peviously only float)
;   02 Nov 2004, MX
;     Adsec.all changed in sc.all.
;   07 Feb 2007 MX
;       LUT of variable deleted and name refurbished.
;   03 April 2007 DZ
;       if the buffer to write is a vector of all actuators then it is used only
;       one call to write_seq_dsp in multicrate. 
;-
function write_seq_ch, act_list, addr_ch0, buf, SET_PM=set_pm, CHECK=check

@adsec_common

if(check_channel(act_list)) then begin
    message, "Actuator list not valid.", CONT=(sc.debug eq 0B)
    return, adsec_error.act_not_valid
endif

if act_list[0] eq sc.all_actuators then begin
    the_act_list = indgen(adsec.n_actuators)
endif else begin
    the_act_list = fix(act_list)
endelse

n_act = n_elements(the_act_list)

if test_type(buf, /FLOAT,/ULONG,/LONG, DIM=dim, N_EL=n_el) then begin
    message, "Buf must be float ulong or long.", CONT=(sc.debug eq 0B)
    return, adsec_error.input_type
endif


  
case dim[0] of
        0: begin
            if n_act ne 1 then begin
                message, "Wrong buf format", CONT=(sc.debug eq 0B)
                return, adsec_error.input_type
            endif
            the_buf = reform([buf], 1, 1)
            n_words = 1
        end

        1: begin
            if n_act ne n_el then begin
                message, "Wrong buf format", CONT=(sc.debug eq 0B)
                return, adsec_error.input_type
            endif
            
            if n_act eq adsec.n_actuators then begin
                the_buf = reform(buf, adsec.n_act_per_dsp, adsec.n_dsp)
                ; in multicrate
                err = write_seq_dsp(sc.all, addr_ch0, the_buf, /CHECK)
                if err ne adsec_error.ok then return, err
           
                return, adsec_error.ok
            endif
            
            the_buf = reform(buf, 1, n_act)
            n_words = 1
        end

        2: begin
            if dim[2] ne n_act then begin
                message, "Wrong buf format", CONT=(sc.debug eq 0B)
                return, adsec_error.input_type
            endif
            the_buf = buf
            n_words = dim[1]
            end
    endcase

    the_dsp_list = fix(the_act_list)/adsec.n_act_per_dsp

    for i_ch = 0,adsec.n_act_per_dsp-1 do begin
        act_idx = where(the_act_list mod adsec.n_act_per_dsp eq i_ch, count)

        if count ne 0 then begin
            dsp = the_dsp_list[act_idx]
            err = write_seq_dsp(dsp, addr_ch0+i_ch*n_words, the_buf[*,act_idx], SET_PM=set_pm, CHECK=check)
            if err ne adsec_error.ok then return, err
        endif
    endfor

    return, adsec_error.ok

end

