; $Id: write_same_dsp.pro,v 1.15 2007/03/26 17:29:35 labot Exp $

;+
; WRITE_SAME_DSP
;
; err = write_same_dsp(dsp_list, addr, buf [, SET_PM=set_pm])
;
; dsp_list:   int vector or scalar. Vector of DSPs ID numbers on the
;             DSP boards. Use sc.all as shortcut for "all DSPs".
;             (0 <= dsp_list <= adsec.n_dsp-1)
;
; addr:       long scalar. Starting address to write to.
;
; buf:        Float/ulong/long vector[n_words], where n_word the number of words to write
;             per DSP.
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
; QUAD_WORD:  'as a quad word flags' enable.
;
; HISTORY
;
;   Written by A. Riccardi (AR)
;   Osservatorio Asrofisico di Arcetri, ITALY
;   <riccardi@arcetri.astro.it>
;
;   13 Mar 2002, AR
;     n_dsp > 256 is now handled.
;     SET_RESET keyword added.
;     Bug fixed in the CONT keyword setting of the MESSAGE call.
;
;   11 Mar 2004, AR
;     Compatibility to LBT hardware:
;     * dsp_list, addr, n_words NOFLOAT
;     * dsp_list is casted internally to INT (adsec.n_dsp < 2^15)
;     * read data are stored in a float array. Float type is
;       good for LBT (32 bit float DSP) as for MMT (float has 24bit
;       mantissa, that can be used without loss for data memory, 16bit
;       integer data, for program memory, 24 bit integer data)
;     * DSP ranges are forced to split a range if it containes DSPs
;       related to different BCUs (BREAK_MODULUS keyword in list2ranges)
;
;   15 mar 2004, AR
;     SET_RESET keyword no longer supported. See new reset_devices function
;
;   03 May 2004, AR
;     Help updated
;
;    18 May 2004, Marco Xompero(MX)
;     QUAD_WORD keyword added.
;
;   20 May 2004, AR
;     ulong and long buffer are now allowed (peviously only float)
;
;   02 Nov 2004, MX
;     Adsec.all changed in sc.all.
;
;   20 Mar 2007, MX
;     Multicrate management added.
;-
function write_same_dsp, dsp_list, addr, buf, SET_PM=set_pm, CHECK=check, QUAD_WORD=quad_word, BCU=is_bcu, SWITCHB=is_switchb, SIGGEN=is_siggen

    @adsec_common

    ;check for the correct memory addressing
    if n_elements(set_pm) eq 0 then set_pm = 0
    is_dsp = (set_pm eq dsp_datasheet.data_mem_id) or (set_pm eq dsp_datasheet.prog_mem_id)
    if not is_dsp then begin
        message, "Not allowed memory type to access for DSPs.", $
               CONT=(sc.debug eq 0)
       return, adsec_error.IDL_BAD_MEM_REQ
    end

    dsp_per_bcu = adsec.n_dsp_per_board*adsec.n_board_per_bcu
    
    ;Range management

    multi_crate = 0B

    case 1B of
        keyword_set(is_switchb): begin
            range_count=1
            range = [0,0]
            the_dsp_list=0
        end
        keyword_set(is_bcu) or keyword_set(is_siggen): begin 
            if dsp_list[0] eq sc.all then the_dsp_list = indgen(adsec.n_bcu) else the_dsp_list=fix(dsp_list)
            if (total(the_dsp_list gt adsec.n_bcu-1) gt 0) or (total(the_dsp_list lt 0) gt 0) then begin
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

            n_dsp = n_elements(the_dsp_list)
            if check_list(the_dsp_list, adsec.n_dsp) eq adsec_error.ok then begin
                multi_crate = 1B
                endval = adsec.n_dsp-1
            endif
            range = list2ranges(the_dsp_list, range_count, BREAK_MODULUS=dsp_per_bcu)
        end
    endcase

    if multi_crate then begin
        err = write_same(0, endval, addr, buf, MULTI_CRATE=multi_crate,  $
                         SET_PM=set_pm, QUAD_WORD=quad_word, BCU=is_bcu, SIGGEN=is_siggen, SWITCHB=is_switchb)
        if err ne adsec_error.ok then return, err
        if keyword_set(check) and (sc.host_comm ne "Dummy") then begin
;===========================================================
; INIZIO CODICE ORIGINALE
            err = read_seq(0, endval, addr, n_elements(buf), check_buf,MULTI_CRATE=multi_crate, $
                                SET_PM=set_pm, TYPE=size(buf, /TYPE), BCU=is_bcu, SWITCHB=is_switchb)
            if err ne adsec_error.ok then return,err
            n_el = n_elements(buf)
            if total(check_buf ne rebin(reform([buf],n_el,1),n_el,adsec.n_crates)) ne 0 then begin
                message," Writing check failed", cont=(sc.debug eq 0B)
                return, adsec_error.write_failed
            endif
; FINE CODICE ORIGINALE
;===========================================================
; INIZIO NUOVO CODICE
;            n_tenta = 10L
;            tenta = 1L
;            repeat begin
;                err = read_seq(0, endval, addr, n_elements(buf), check_buf,MULTI_CRATE=multi_crate, $
;                                    SET_PM=set_pm, TYPE=size(buf, /TYPE), BCU=is_bcu, SWITCHB=is_switchb)
;                if err ne adsec_error.ok then begin
;                    message, "ARMATEST: error read_seq in CHECK. Attempt#"+strtrim(tenta,2)+" of "+strtrim(n_tenta,2), cont=(sc.debug eq 0B)
;                    log_print, "ARMATEST: error read_seq in CHECK i. Attempt#"+strtrim(tenta,2)+" of "+strtrim(n_tenta,2), LEVEL=20
;                endif else begin
;                    n_el = n_elements(buf)
;                    if total(check_buf ne rebin(reform([buf],n_el,1),n_el,adsec.n_crates)) ne 0 then begin
;                        err = adsec_error.write_failed
;                        message,"ARMATEST: Writing check failed. Attempt#"+strtrim(tenta,2)+" of "+strtrim(n_tenta,2), cont=(sc.debug eq 0B)
;                        log_print, "ARMATEST: Writing check failed. Attempt#"+strtrim(tenta,2)+" of "+strtrim(n_tenta,2), LEVEL=20
;                    endif
;                endelse
;                tenta += 1L
;                if err eq adsec_error.ok then break
;            endrep until tenta gt n_tenta
;            if err ne adsec_error.ok then return, err
; FINE NUOVO CODICE
;===========================================================
        endif


    endif else begin
        
        for ir=0,range_count-1 do begin
            err = write_same(range[0,ir], range[1,ir], addr, buf, $
                             SET_PM=set_pm, QUAD_WORD=quad_word, BCU=is_bcu, SWITCHB=is_switchb, SIGGEN=is_siggen)
            if err ne adsec_error.ok then return, err
            if keyword_set(check) and (sc.host_comm ne "Dummy") then begin
;=============================================================
; INIZIO CODICE ORIGINALE
;                err = read_seq(range[0,ir], range[1,ir], addr, n_elements(buf), check_buf, $
;                                    SET_PM=set_pm, TYPE=size(buf, /TYPE), BCU=is_bcu, SWITCHB=is_switchb, SIGGEN=is_siggen)
;                if err ne adsec_error.ok then return,err
;                n_el = n_elements(buf)
;                n_dsp = range[1,ir]-range[0,ir]+1
;                if total(check_buf ne rebin(reform([buf],n_el,1),n_el,n_dsp)) ne 0 then begin
;                    message," Writing check failed", cont=(sc.debug eq 0B)
;                    return, adsec_error.write_failed
;                endif
; FINE CODICE ORIGINALE
;===========================================================
; INIZIO NUOVO CODICE
                n_tenta = 10L
                tenta = 1L
                repeat begin
                    err = read_seq(range[0,ir], range[1,ir], addr, n_elements(buf), check_buf, $
                                        SET_PM=set_pm, TYPE=size(buf, /TYPE), BCU=is_bcu, SWITCHB=is_switchb, SIGGEN=is_siggen)
                    if err ne adsec_error.ok then begin
                        message, "ARMATEST: error read_seq in CHECK. Attempt#"+strtrim(tenta,2)+" of "+strtrim(n_tenta,2), cont=(sc.debug eq 0B)
                        log_print, "ARMATEST: error read_seq in CHECK i. Attempt#"+strtrim(tenta,2)+" of "+strtrim(n_tenta,2);, LEVEL=20
                    endif else begin
                        n_el = n_elements(buf)
                        n_dsp = range[1,ir]-range[0,ir]+1
                        if total(check_buf ne rebin(reform([buf],n_el,1),n_el,n_dsp)) ne 0 then begin
                            err = adsec_error.write_failed
                            message,"ARMATEST: Writing check failed. Attempt#"+strtrim(tenta,2)+" of "+strtrim(n_tenta,2), cont=(sc.debug eq 0B)
                            log_print, "ARMATEST: Writing check failed. Attempt#"+strtrim(tenta,2)+" of "+strtrim(n_tenta,2);, LEVEL=20
                            log_print, 'Read '+strtrim(string(check_buf[0]), 2) + ' instead of ' + strtrim(string(buf[0]), 2)
                        endif
                    endelse
                    tenta += 1L
                    if err eq adsec_error.ok then break
                endrep until tenta gt n_tenta
                if err ne adsec_error.ok then return, err
; FINE NUOVO CODICE
;===========================================================
            endif
        endfor

    endelse

    return, adsec_error.ok

end

