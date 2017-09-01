; $Id: read_seq_dsp.pro,v 1.13 2007/03/26 17:29:35 labot Exp $

;+
; READ_SEQ_DSP
;
; err = read_seq_dsp(dsp_list, addr, n_words, buf [, SET_PM=set_pm] $
;                    [, /ULONG|/LONG|TYPE=data_type])
;
; dsp_list:   int vector or scalar. Vector of DSPs ID numbers on the
;             DSP boards. (0 <= dsp_list <= adsec.n_dsp-1)
;
; addr:       long scalar. Starting address to read from.
;
; n_words:    long scalar. Number of words to read.
;
; buf:        named variable. Float/long/ulong vector[n_words,N], where
;             N is the number of DSP to read from.
; KEYWORDS
;
; SET_PM:     byte scalar. Code identifing the type of memory to
;             read from. Default value is dsp_datasheet.data_mem_id,
;             it is used to read from DSP data memory.
;             Set SET_PM to dsp_datasheet.prog_mem_id to read
;             from DSP program memory (if available)
;
;
;    ULONG:      If set the output is translated to unsigned long (same bit sequence)
;     LONG:      If set the output is translated to signed long (same bit sequence)
;    TYPE:       int scalar. If passed the output is translated to the corresponding
;                data type. Allowed values: 3(long), 4(float) and 13(ulong). TYPE setting
;                overrides the ULONG and LONG keyword setting. If no ULONG, LONG or TYPE keyword
;                are set, default data type is float.
;   SWITCH:     If set, the actions refers to SWITCH BCU
;
; HISTORY
;
;   Written by A. Riccardi (AR)
;   Osservatorio Asrofisico di Arcetri, ITALY
;   <riccardi@arcetri.astro.it>
;
;   13 Mar 2002, AR
;       n_dsp > 256 is now handled
;   26 Feb 2004, AR
;       Compatibility to LBT hardware:
;       * dsp_list, addr, n_words NOFLOAT
;       * dsp_list is casted internally to INT (adsec.n_dsp < 2^15)
;       * read data are stored in a float array. Float type is
;         good for LBT (32 bit float DSP) as for MMT (float has 24bit
;         mantissa, that can be used without loss for data memory, 16bit
;         integer data, for program memory, 24 bit integer data)
;       * CHECK_PM_16BIT and WPHL_RESULT keywords no longer supported
;       * DSP ranges are forced to split a range if it containes DSPs
;         related to different BCUs (BREAK_MODULUS keyword in list2ranges)
;   20 May 2004, AR
;       TYPE, ULONG and LONG keyword added
;   02 Nov 2004, MX
;       Adsec.all changed in sc.all.
;   07 Feb 2007 MX
;       LUT of variable deleted and name refurbished.
;       SwitchBCU now managed
;-
function read_seq_dsp, dsp_list, addr, n_words, buf, SET_PM=set_pm, $
         ULONG=set_ulong, LONG=set_long, TYPE=data_type, BCU=is_bcu, SIGGEN=is_siggen, SWITCHB=is_switchb

    @adsec_common

    if n_elements(set_pm) eq 0 then set_pm = 0
    is_dsp = (set_pm eq dsp_datasheet.data_mem_id) or (set_pm eq dsp_datasheet.prog_mem_id)
    if not is_dsp then begin
        message, "Not allowed memory type to access for DSPs.", $
               CONT=(sc.debug eq 0)
       return, adsec_error.IDL_BAD_MEM_REQ
    end

    if test_type(n_words, /INT, /LONG, N_EL=n_el) then begin
        message, "n_words must be an integer", CONT=(sc.debug eq 0B)
        return, adsec_error.input_type
    endif
    if n_el ne 1 then begin
        message, "n_words must be a scalar", CONT=(sc.debug eq 0B)
        return, adsec_error.input_type
    endif

    
    ;Range management

    multi_crate = 0B

    case 1B of

        keyword_set(is_switchb): begin
            range_count=1
            range = [0,0]
            the_dsp_list=0
        end

        keyword_set(is_bcu) or keyword_set(is_siggen): begin 
            if dsp_list[0] eq sc.all then the_dsp_list = indgen(adsec.n_crates) else the_dsp_list=fix(dsp_list)
            if (total(the_dsp_list gt adsec.n_crates-1) gt 0) or (total(the_dsp_list lt 0) gt 0) then begin
                message, "BCU/SIGGEN list not valid.", CONT=(sc.debug eq 0B)
                return, adsec_error.dsp_not_valid
            endif
            
            if check_list(the_dsp_list, adsec.n_crates) eq adsec_error.ok then begin
                multi_crate = 1B
                endval = adsec.n_crates-1
            endif
            range = list2ranges(the_dsp_list, range_count, BREAK_MODULUS=1)
        end

        else:begin
            if(check_dsp(dsp_list)) then begin
                message, "DSP list not valid.", CONT=(sc.debug eq 0B)
                return, adsec_error.dsp_not_valid
            endif

            if dsp_list[0] eq sc.all_dsp then begin
                the_dsp_list = indgen(adsec.n_dsp)
            endif else begin
                the_dsp_list = fix(dsp_list)
            endelse
            dsp_per_bcu = adsec.n_dsp_per_board*adsec.n_board_per_bcu
            
            if check_list(the_dsp_list, adsec.n_dsp) eq adsec_error.ok then begin
                multi_crate = 1B
                endval = adsec.n_dsp-1
            endif
            range = list2ranges(the_dsp_list, range_count, BREAK_MODULUS=dsp_per_bcu)
        end

    endcase

    n_dsp = n_elements(the_dsp_list)
    sort_idx = sort(the_dsp_list)

    if multi_crate then begin
        
        err = read_seq(0,endval, addr, n_words, buf, MULTI_CRATE=multi_crate, SET_PM=set_pm, $ 
                        ULONG=set_ulong, LONG=set_long, TYPE=data_type, BCU=is_bcu, SIGGEN=is_siggen, SWITCHB=is_switchb)
        if err ne adsec_error.ok then return, err

    endif else begin

        i = 0L
        for ir=0,range_count-1 do begin

            ii = i+range[1,ir]-range[0,ir]
            idx = sort_idx[i:ii]

            err = read_seq(range[0,ir], range[1,ir], addr, n_words, dummy_buf, $
                           SET_PM=set_pm, ULONG=set_ulong, LONG=set_long, TYPE=data_type, BCU=is_bcu, SIGGEN=is_siggen, SWITCHB=is_switchb)
            if err ne adsec_error.ok then return, err

            if ir eq 0 then buf = make_array(n_words, n_dsp, TYPE=size(dummy_buf,/TYPE))
            buf[*,idx] = dummy_buf

            i = ii+1

        endfor
    endelse

    return, adsec_error.ok

end
