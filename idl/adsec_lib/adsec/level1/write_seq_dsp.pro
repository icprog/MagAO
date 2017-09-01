; $Id: write_seq_dsp.pro,v 1.14 2007/09/20 16:08:15 labot Exp $

;+
; WRITE_SEQ_DSP
;
; err = write_seq_dsp(dsp_list, addr, buf [, SET_PM=set_pm] $
;                     [, /CHECK])
;
; dsp_list:   int vector or scalar. Vector of DSPs ID numbers on the
;             DSP boards. (0 <= dsp_list <= adsec.n_dsp-1)
;
; addr:       long scalar. Starting address to write to.
;
; buf:        Float/ulong/long vector[n_words,N], where n_words is the number
;             of word per DSP to write.
; KEYWORDS
;
; SET_PM:     byte scalar. Code identifing the type of memory to
;             write to. Default value is dsp_datasheet.data_mem_id,
;             it is used to read from DSP data memory.
;             Set SET_PM to dsp_datasheet.prog_mem_id to read
;             from program memory (if available)
;
; CHECK:      If it is set, the written data are read back to check
;             if the data have been written correctly
;
; QUAD_WORD:     'as a quad word flags' enable.
; BCU:           If set, the actions are performed on Crate BCUs
; SIGGEN:        If set, the actions are performed on SIGGEN 
; SWITCH BCU:    If set, the actions are performed on SWITCH BCU 
;
; HISTORY
;   Written by A. Riccardi (AR)
;   Osservatorio Asrofisico di Arcetri, ITALY
;   <riccardi@arcetri.astro.it>
;   13 Mar 2002, AR
;     n_dsp > 256 is now handled.
;     Bug fixed in the CONT keyword setting of the MESSAGE call.
;   11 Mar 2004, AR
;       Compatibility to LBT hardware:
;       * dsp_list, addr, n_words NOFLOAT
;       * dsp_list is casted internally to INT (adsec.n_dsp < 2^15)
;       * read data are stored in a float array. Float type is
;         good for LBT (32 bit float DSP) as for MMT (float has 24bit
;         mantissa, that can be used without loss for data memory, 16bit
;         integer data, for program memory, 24 bit integer data)
;       * DSP ranges are forced to split a range if it containes DSPs
;         related to different BCUs (BREAK_MODULUS keyword in list2ranges)
;   18 May 2004, Marco Xompero(MX)
;       QUAD_WORD keyword added.
;   20 May 2004, AR
;       ulong and long buffer are now allowed (peviously only float)
;   02 Nov 2004, MX
;       Adsec.all changed in sc.all.
;   07 Feb 2007 MX
;       LUT of variable deleted and name refurbished.
;       SwitchBCU now managed
;   20 Mar 2007, MX
;     Multicrate management added.
;-
function write_seq_dsp, dsp_list, addr, buf, SET_PM=set_pm, CHECK=check, QUAD_WORD=quad_wor, BCU=is_bcu, SIGGEN=is_siggen, SWITCHB=is_switchb

    @adsec_common

    if n_elements(set_pm) eq 0 then set_pm = 0
    is_dsp = (set_pm eq dsp_datasheet.data_mem_id) or (set_pm eq dsp_datasheet.prog_mem_id)
    if not is_dsp then begin
        message, "Not allowed memory type to access for DSPs.", $
               CONT=(sc.debug eq 0)
        return, adsec_error.IDL_BAD_MEM_REQ
    end


    dsp_per_bcu = adsec.n_dsp_per_board*adsec.n_board_per_bcu
    
    ;Range management

    multi_crate=0B

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

            if check_list(the_dsp_list, adsec.n_dsp) eq adsec_error.ok then begin
                multi_crate = 1B
                endval = adsec.n_dsp-1
            endif

            range = list2ranges(the_dsp_list, range_count, BREAK_MODULUS=dsp_per_bcu)
        end

    endcase

    n_dsp = n_elements(the_dsp_list)

    if test_type(buf, /FLOAT, /ULONG, /LONG, DIM=dim, N_EL=n_el) then begin
        message, "Buf must be float.", CONT=(sc.debug eq 0B)
        return, adsec_error.input_type
    endif

    case dim[0] of
        0: begin
            if n_dsp ne 1 then begin
                message, "Wrong buf size: vector[n_dsp] expected.", CONT=(sc.debug eq 0B)
                return, adsec_error.input_type
            endif
            the_buf = reform([buf], 1, 1)
        end

        1: begin
            if (n_dsp ne n_el) and (n_dsp ne 1) then begin
                message, "Wrong buf size: vector[n_dsp] or array[*,n_dsp] expected.", CONT=(sc.debug eq 0B)
                return, adsec_error.input_type
            endif
            if n_dsp ne 1 then the_buf = reform(buf, 1, n_dsp) else the_buf=buf
        end

        2: begin
            if dim[2] ne n_dsp then begin
                message, "Wrong buf size: array[*,n_dsp] expected.", CONT=(sc.debug eq 0B)
                return, adsec_error.input_type
            endif
            the_buf = buf
        end
    endcase

    sort_idx = sort(the_dsp_list)

    if multi_crate then begin
 
        err = write_seq(0, endval, addr, the_buf, MULTI_CRATE=multi_crate,  $
                        SET_PM=set_pm, QUAD_WORD=quad_word, BCU=is_bcu, SIGGEN=is_siggen, SWITCHB=is_switchb)
        if err ne adsec_error.ok then return, err

        if keyword_set(check) and (sc.host_comm ne "Dummy") then begin
;=============================================================
; INIZIO CODICE ORIGINALE
            err = read_seq(0,endval,  addr, n_elements(the_buf[*,sort_idx[0]]), check_buf,MULTI_CRATE=multi_crate, $
                                SET_PM=set_pm, TYPE=size(buf, /TYPE), BCU=is_bcu, SIGGEN=is_siggen, SWITCHB=is_switchb)
            if err ne adsec_error.ok then return,err
            if total(check_buf ne buf) ne 0 then begin
                print, string(where(check_buf ne buf))
                message," Writing check failed", cont=(sc.debug eq 0B)
                return, adsec_error.write_failed
            endif
; FINE CODICE ORIGINALE
;===========================================================
; INIZIO NUOVO CODICE
;            n_tenta = 10L
;            tenta = 1L
;            repeat begin 
;                err = read_seq(0,endval,  addr, n_elements(the_buf[*,sort_idx[0]]), check_buf,MULTI_CRATE=multi_crate, $
;                                    SET_PM=set_pm, TYPE=size(buf, /TYPE), BCU=is_bcu, SIGGEN=is_siggen, SWITCHB=is_switchb)
;                if err ne adsec_error.ok then begin
;                    message, "ARMATEST: error read_seq in CHECK. Attempt#"+strtrim(tenta,2)+" of "+strtrim(n_tenta,2), cont=(sc.debug eq 0B)
;                    log_print, "ARMATEST: error read_seq in CHECK i. Attempt#"+strtrim(tenta,2)+" of "+strtrim(n_tenta,2), LEVEL=20
;                endif else begin
;                    if total(check_buf ne buf) ne 0 then begin
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

        i = 0L
        for ir=0,range_count-1 do begin
            ii = i+range[1,ir]-range[0,ir]
            idx = sort_idx[i:ii]
            err = write_seq(range[0,ir], range[1,ir], addr, the_buf[*,idx], $
                            SET_PM=set_pm, QUAD_WORD=quad_word, BCU=is_bcu, SIGGEN=is_siggen, SWITCHB=is_switchb)
            if err ne adsec_error.ok then return, err

            if keyword_set(check) and (sc.host_comm ne "Dummy") then begin
;=============================================================
; INIZIO VECCHIO CODICE
;                err = read_seq(range[0,ir], range[1,ir], addr, n_elements(the_buf[*,idx[0]]), check_buf, $
;                                    SET_PM=set_pm, TYPE=size(buf, /TYPE), BCU=is_bcu, SIGGEN=is_siggen, SWITCHB=is_switchb)
;                if err ne adsec_error.ok then return,err
;                if total(check_buf ne the_buf[*,idx]) ne 0 then begin
;                    message," Writing check failed", cont=(sc.debug eq 0B)
;                    return, adsec_error.write_failed
;                endif
; FINE VECCHIO CODICE
;=============================================================
; INIZIO NUOVO CODICE
                n_tenta = 10L
                tenta = 1L
                repeat begin
                    err = read_seq(range[0,ir], range[1,ir], addr, n_elements(the_buf[*,idx[0]]), check_buf, $
                                        SET_PM=set_pm, TYPE=size(buf, /TYPE), BCU=is_bcu, SIGGEN=is_siggen, SWITCHB=is_switchb)
                    if err ne adsec_error.ok then begin
                        message, "ARMATEST: error read_seq in CHECK. Attempt#"+strtrim(tenta,2)+" of "+strtrim(n_tenta,2), cont=(sc.debug eq 0B)
                        log_print, "ARMATEST: error read_seq in CHECK i. Attempt#"+strtrim(tenta,2)+" of "+strtrim(n_tenta,2), LEVEL=20
                    endif else begin
                        if total(check_buf ne the_buf[*,idx]) ne 0 then begin
                            err = adsec_error.write_failed
                            message,"ARMATEST: Writing check failed. Attempt#"+strtrim(tenta,2)+" of "+strtrim(n_tenta,2), cont=(sc.debug eq 0B)
                            log_print, "ARMATEST: Writing check failed. Attempt#"+strtrim(tenta,2)+" of "+strtrim(n_tenta,2), LEVEL=20
                        endif
                    endelse
                    tenta += 1L
                    if err eq adsec_error.ok then break
                endrep until tenta gt n_tenta
                if err ne adsec_error.ok then return, err
; FINE NUOVO CODICE
;=============================================================
            endif
            i = ii+1
        endfor
    endelse

    return, adsec_error.ok

end

