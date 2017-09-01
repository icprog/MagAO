; $Id: read_seq_ch.pro,v 1.11 2007/04/11 13:00:35 labot Exp $

;+
; READ_SEQ_CH
;
; err = read_seq_ch(act_list, addr_ch0, n_words, buf [, SET_PM=set_pm] $
;                   [, /LONG|/ULONG|TYPE=data_type])
;
; act_list:   int vector[N] or scalar. Vector of actuator ID numbers.
;             Use sc.all as shortcut for "all actuators".
;             (0 <= act_list <= adsec.n_actuators-1)
;
; addr_ch0:   long scalar. Address of the channel zero to read from.
;             Even if the target actuator is not the first channel of
;             the DSP, the address must be the one of the channel zero
;             in the DSP. The program computes the correct offset
;             for channels different from the channel zero. That allows
;             to address any channel in act_list using the same address.
;
; n_words:    long scalar. Number of words to read per actuator.
;
; buf:        named variable in input. Float/ulong/long vector[n_word,N] on output.
;             Buffer filled by values read from actuators.
;
; KEYWORDS
;
; SET_PM:     byte scalar. Code identifing the type of memory to
;             read from. Default value is dsp_datasheet.data_mem_id,
;             it is used to write to DSP data memory.
;             Set SET_PM to dsp_datasheet.prog_mem_id to read from
;             program memory (if available)
;
;    ULONG:      If set the output is translated to unsigned long (same bit sequence)
;     LONG:      If set the output is translated to signed long (same bit sequence)
;    TYPE:       int scalar. If passed the output is translated to the corresponding
;                data type. Allowed values: 3(long), 4(float) and 13(ulong). TYPE setting
;                overrides the ULONG and LONG keyword setting. If no ULONG, LONG or TYPE keyword
;                are set, default data type is float.
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
;     TYPE, ULONG and LONG keyword added
;   02 Nov 2004, MX
;     Adsec.all changed in sc.all.
;   20 May 2005, MX
;     Routine managed to get multi-dimensional data in ch mode.
;   07 Feb 2007 MX
;       LUT of variable deleted and name refurbished.
;   03 April 2007 DZ
;       Read in one call to read_seq_dsp in the case of all actuators and one word.
;-
function read_seq_ch, act_list, addr_ch0, n_words, buf, SET_PM=set_pm, $
         ULONG=set_ulong, LONG=set_long, TYPE=data_type

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
the_dsp_list = fix(the_act_list)/adsec.n_act_per_dsp

multi_crate = 0b
if check_list(the_dsp_list, adsec.n_dsp) eq adsec_error.ok then begin
    multi_crate = 1B
    endval = adsec.n_dsp-1
endif

;if multi_crate then begin
;; err = read_seq_dsp(dsp, addr_ch0+i_ch*n_words, n_words, dummy_buf, SET_PM=set_pm, $
;                               ULONG=set_ulong, LONG=set_long, TYPE=data_type)


if (n_act eq adsec.n_actuators) and (n_words eq 1) then begin

    err = read_seq_dsp(sc.all, addr_ch0, n_words*adsec.n_act_per_dsp, dummy_buf, SET_PM=set_pm, $
                       ULONG=set_ulong, LONG=set_long, TYPE=data_type)
    if err ne adsec_error.ok then return, err
    buf = transpose(reform(dummy_buf, adsec.n_actuators))
    
    return, adsec_error.ok
 

endif else begin

    allocate_buf=1B
    for i_ch = 0,adsec.n_act_per_dsp-1 do begin
        act_idx = where(the_act_list mod adsec.n_act_per_dsp eq i_ch, count)
        
        if count ne 0 then begin
            dsp = the_dsp_list[act_idx]
            err = read_seq_dsp(dsp, addr_ch0+i_ch*n_words, n_words, dummy_buf, SET_PM=set_pm, $
                               ULONG=set_ulong, LONG=set_long, TYPE=data_type)
            if err ne adsec_error.ok then return, err
            if allocate_buf then begin
                buf = make_array(n_words, n_act, TYPE=size(dummy_buf,/TYPE))
                allocate_buf = 0B
            endif
            buf[*,act_idx] = dummy_buf
        endif
    endfor
endelse

return, adsec_error.ok

end

