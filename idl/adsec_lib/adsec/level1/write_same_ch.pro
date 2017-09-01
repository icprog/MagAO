; $Id: write_same_ch.pro,v 1.9 2007/04/11 13:01:50 labot Exp $

;+
; WRITE_SAME_CH
;
; err = write_same_ch(act_list, addr_ch0, buf [, SET_PM=set_pm][, /CHECK])
;
; act_list:   int vector or scalar. Vector of actuator ID numbers.
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
; buf:        Float/long/ulong vector[n_word]. Buffer to write (same for all the actuators).
;             n_word is the number of words to write per actuator.
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
;
;   02 Nov 2004, MX
;     Adsec.all changed in sc.all.
;   07 Feb 2007 MX
;       LUT of variable deleted and name refurbished.
;   03 April 2007 DZ
;       Only one call if the act_list is all the actuators. 
;-
function write_same_ch, act_list, addr_ch0, buf, SET_PM=set_pm, CHECK=check

@adsec_common

if(check_channel(act_list)) then begin
    message, "Actuator list not valid", CONT=(sc.debug eq 0B)
    return, adsec_error.act_not_valid
endif
if act_list[0] eq sc.all_actuators then begin
    the_act_list = indgen(adsec.n_actuators)
endif else begin
    the_act_list = fix(act_list)
endelse



if (n_elements(buf) eq 1) and (act_list[0] eq sc.all_actuators) then begin

    buf2write = replicate(buf, adsec.n_act_per_dsp)

    err = write_same_dsp(sc.all, addr_ch0, buf2write, SET_PM=set_pm, CHECK=check)
        if err ne adsec_error.ok then return, err

    return, adsec_error.ok

endif else begin 

    the_dsp_list = fix(the_act_list)/adsec.n_act_per_dsp
    for i_ch = 0,adsec.n_act_per_dsp-1 do begin
        act_idx = where(the_act_list mod adsec.n_act_per_dsp eq i_ch, count)

        if count ne 0 then begin
            dsp = the_dsp_list[act_idx]
            err = write_same_dsp(dsp, addr_ch0+i_ch, buf, SET_PM=set_pm, CHECK=check)
            if err ne adsec_error.ok then return, err
        endif
    endfor

    return, adsec_error.ok
endelse

end

