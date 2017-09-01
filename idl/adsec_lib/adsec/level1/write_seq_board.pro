; $Id: write_seq_board.pro,v 1.7 2007/03/26 17:29:35 labot Exp $

;+
; WRITE_SEQ_BOARD
;
; err = write_seq_board(board_list, addr, buf [, SET_PM=set_pm] $
;                       [, /BCU|/SIGGEN][, /CHECK])
;
; board_list: int vector or scalar. Vector of DSP/BCU/SIGGEN board ID
;             numbers. If no BCU nor SIGGEN keyword are set, board
;             list refers to DSP boards
;
; addr:       long scalar. Starting address to write to.
;
; buf:        Float vector[n_words, N], where N is the number of
;             boards and n_words is the number of word per board to write.
; KEYWORDS
;
; SET_PM:     byte scalar. Code identifing the type of memory to
;             write to. Default value is dsp_datasheet.sdram_mem_id,
;             it is used to read from SDRAM data memory.
;             See xxxx_mem_id filds in dsp_datasheet structure for the
;             allowed values for SET_PM keyword.
;
; BCU:        if it is set, board_list refers to BCU board IDs.
;
; SIGGEN:     if it is set, board_list refers to SIGGEN board IDs.
;
; SWITCH:     if it is set, board_list refers to SWITCH BCU board .
;
; CHECK:      If it is set, the written data are read back to check
;             if the data have been written correctly
;
; HISTORY
;
;   24 Mar 2004. Written by A. Riccardi (AR)
;   Osservatorio Asrofisico di Arcetri, ITALY
;   <riccardi@arcetri.astro.it>
;
;   20 May 2004, AR
;       ulong and long buffer are now allowed (peviously only float)
;       if sc.debug is set, then SET_PM has not a default value (procedure stops)
;   02 Nov 2004, MX
;       Adsec.all changed in sc.all.
;   07 Feb 2007 MX
;       LUT of variable deleted and name refurbished.
;       SwitchBCU now managed
;   20 Mar 2007, MX
;       Multicrate management added.
;   
;-
function write_seq_board, board_list, addr, buf, SET_PM=set_pm, CHECK=check, $
                           BCU=is_bcu, SIGGEN=is_siggen, SWITCHB=is_switchb

    @adsec_common

    if sc.debug eq 1B then $
        if n_elements(set_pm) eq 0 then message, "SET_PM not defined." $
    else $
        if n_elements(set_pm) eq 0 then set_pm = dsp_datasheet.sdram_mem_id

    nn = total([keyword_set(is_bcu),keyword_set(is_siggen), keyword_set(is_switchb)])
    if nn eq 0 then begin
        is_dsp_board=1
    endif else begin
        id_dsp=0
        if nn gt 1 then begin
            message, "Only one keyword between BCU/SIGGEN/SWITCHB can be set.", CONT=(sc.debug eq 0B)
            return, adsec_error.IDL_INPUT_TYPE
        endif
    endelse


    if board_list[0] eq sc.all then begin
        case 1B of
            keyword_set(is_dsp_board): the_board_list = indgen(adsec.n_bcu*adsec.n_board_per_bcu)
            keyword_set(is_bcu) or keyword_set(is_siggen)  : the_board_list = indgen(adsec.n_bcu)
            keyword_set(is_switchb)  : the_board_list = 0
        endcase
    endif else the_board_list = fix(board_list)

    ;Range management

    multi_crate = 0B

    case 1B of
        keyword_set(is_switchb): begin
            range_count=1
            range = [0,0]
        end
        keyword_set(is_bcu) or keyword_set(is_siggen): begin 
            board_per_sector = 1
            if(check_board(the_board_list, DSP=is_dsp_board, BCU=is_bcu, SIGGEN=is_siggen)) then begin
                message, "Board list not valid.", CONT=(sc.debug eq 0B)
                return, adsec_error.dsp_not_valid
            endif
            if check_list(the_dsp_list, adsec.n_crates) eq adsec_error.ok then begin
                multi_crate = 1B
                endval = adsec.n_crates-1
            endif
            range = list2ranges(the_board_list, range_count, BREAK_MODULUS=board_per_sector)
        end
        else:begin
            board_per_sector = adsec.n_board_per_bcu
            if(check_board(the_board_list, DSP=is_dsp_board, BCU=is_bcu, SIGGEN=is_siggen)) then begin
                message, "Board list not valid.", CONT=(sc.debug eq 0B)
                return, adsec_error.dsp_not_valid
            endif
            if check_list(the_board_list,adsec.n_dsp/adsec.n_dsp_per_board) eq adsec_error.ok then begin
                multi_crate = 1B
                endval = adsec.n_dsp/adsec.n_dsp_per_board-1
            endif
            range = list2ranges(the_board_list, range_count, BREAK_MODULUS=board_per_sector)
        end
    endcase
    n_board = n_elements(the_board_list)
    sort_idx = sort(the_board_list)

    if test_type(buf, /FLOAT, /ULONG, /LONG, DIM=dim, N_EL=n_el) then begin
        message, "Buf must be float.", CONT=(sc.debug eq 0B)
        return, adsec_error.input_type
    endif

    case dim[0] of
        0: begin
            if n_board ne 1 then begin
                message, "Wrong buf size: vector[n_board] expected.", CONT=(sc.debug eq 0B)
                return, adsec_error.input_type
            endif
            the_buf = reform([buf], 1, 1)
        end

        1: begin
            if (n_board ne n_el) and (n_board ne 1) then begin
                message, "Wrong buf size: vector[n_board] or array[*,n_board] expected.", CONT=(sc.debug eq 0B)
                return, adsec_error.input_type
            endif
            if n_board ne 1 then the_buf = reform(buf, 1, n_board) else the_buf=buf
        end

        2: begin
            if dim[2] ne n_board then begin
                message, "Wrong buf size: array[*,n_board] expected.", CONT=(sc.debug eq 0B)
                return, adsec_error.input_type
            endif
            the_buf = buf
        end
    endcase


    if multi_crate then begin
        
        err = write_seq(0, endval, addr, the_buf, MULTI_CRATE=multi_crate,  $
                        SET_PM=set_pm, QUAD_WORD=quad_word, BCU=is_bcu, SIGGEN=is_siggen, SWITCHB=is_switchb)
        if err ne adsec_error.ok then return, err

        if keyword_set(check) and (sc.host_comm ne "Dummy") then begin
            err = read_seq(0,adsec.n_dsp/adsec.n_dsp_per_board-1,  addr, n_elements(the_buf[*,sort_idx[0]]), check_buf, $
                                MULTI_CRATE=multi_crate, SET_PM=set_pm, TYPE=size(buf, /TYPE), BCU=is_bcu, SIGGEN=is_siggen, SWITCHB=is_switchb)
            if err ne adsec_error.ok then return,err
            if total(check_buf ne buf) ne 0 then begin
                message," Writing check failed", cont=(sc.debug eq 0B)
                return, adsec_error.write_failed
            endif
        endif


    endif else begin
        i = 0L
        for ir=0,range_count-1 do begin
            ii = i+range[1,ir]-range[0,ir]
            idx = sort_idx[i:ii]
            err = write_seq(range[0,ir], range[1,ir], addr, the_buf[*,idx], $
                            SET_PM=set_pm, BCU=is_bcu, SIGGEN=is_siggen, SWITCHB=is_switchb)
            if err ne adsec_error.ok then return, err

            if keyword_set(check) and (sc.host_comm ne "Dummy") then begin
                err = read_seq(range[0,ir], range[1,ir], addr, n_elements(the_buf[*,idx[0]]), check_buf, $
                                    SET_PM=set_pm, BCU=is_bcu, SIGGEN=is_siggen, TYPE=size(the_buf, /TYPE), SWITCHB=is_switchb)
                if err ne adsec_error.ok then return,err
                if total(check_buf ne buf) ne 0 then begin
                    message," Writing check failed", cont=(sc.debug eq 0B)
                    return, adsec_error.write_failed
                endif
            endif

            i = ii+1
        endfor
    endelse

    return, adsec_error.ok

end

