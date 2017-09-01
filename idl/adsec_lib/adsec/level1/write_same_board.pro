; $Id: write_same_board.pro,v 1.8 2007/03/26 17:26:25 labot Exp $

;+
; WRITE_SAME_BOARD
;
; err = write_same_board(board_list, addr, buf [, SET_PM=set_pm] $
;                        [, /BCU|/SIGGEN][, /CHECK])
;
; board_list: int vector or scalar. Vector of DSP/BCU/SIGGEN board ID
;             numbers. If no BCU nor SIGGEN keyword are set, board
;             list refers to DSP boards
;
; addr:       long scalar. Starting address to write to.
;
; buf:        Float/long/ulong vector[n_words], where n_word the number
;             of words to write per board.
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
; SWITCH:     if set, the actions are performed on switch bcu
;
; CHECK:      If it is set, the written data are read back to check
;             if the data have been written correctly
; QUAD_WORD:  pass quad_word keyword to write same
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
;   27 Feb 2007 MX
;       Added QUAD_WORD keyword passed to write_same function
;   20 Mar 2007, MX
;       Multicrate management added.
;
;-
function write_same_board, board_list, addr, buf, SET_PM=set_pm, CHECK=check, $
                           BCU=is_bcu, SIGGEN=is_siggen, SWITCHB=switchb, QUAD_WORD=quad_word

    @adsec_common

    if sc.debug eq 1B then $
        if n_elements(set_pm) eq 0 then message, "SET_PM not defined." $
    else $
        if n_elements(set_pm) eq 0 then set_pm = dsp_datasheet.sdram_mem_id

    nn = total([keyword_set(is_bcu),keyword_set(is_siggen), keyword_set(SWITCHB)])
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
            keyword_set(switchb)  : the_board_list = 0
        endcase
    endif else the_board_list = fix(board_list)

    ;Range management

    multi_crate = 0B

    case 1B of
        keyword_set(SWITCHB): begin
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

            n_board = n_elements(the_board_list)
            if check_list(the_board_list,adsec.n_dsp/adsec.n_dsp_per_board) eq adsec_error.ok then begin
                multi_crate = 1B
                endval = adsec.n_dsp/adsec.n_dsp_per_board-1
            endif
            range = list2ranges(the_board_list, range_count, BREAK_MODULUS=board_per_sector)
        end
    endcase

    if multi_crate then begin

        err = write_same(0, endval, addr, buf, MULTI_CRATE=multi_crate, $
                         SET_PM=set_pm, BCU=is_bcu, SIGGEN=is_siggen, SWITCHB=switchb, QUAD_WORD=quad_word)
        if err ne adsec_error.ok then return, err

        if keyword_set(check) and (sc.host_comm ne "Dummy") then begin
            err = read_seq(0, endval, addr, n_elements(buf), check_buf, $
                                SET_PM=set_pm, TYPE=size(buf, /TYPE), BCU=bcu, SWITCHB=is_switchb, MULTI_CRATE=multi_crate)
            if err ne adsec_error.ok then return,err
            n_el = n_elements(buf)
            if total(check_buf ne rebin(reform([buf],n_el,1),n_el,adsec.n_crates)) ne 0 then begin
                message," Writing check failed", cont=(sc.debug eq 0B)
                return, adsec_error.write_failed
            endif
        endif



    endif else begin

        for ir=0,range_count-1 do begin
            err = write_same(range[0,ir], range[1,ir], addr, buf, $
                             SET_PM=set_pm, BCU=is_bcu, SIGGEN=is_siggen, SWITCHB=switchb, QUAD_WORD=quad_word)
            if err ne adsec_error.ok then return, err

            if keyword_set(check) and (sc.host_comm ne "Dummy") then begin
                err = read_seq(range[0,ir], range[1,ir], addr, n_elements(buf), check_buf, $
                                    SET_PM=set_pm, BCU=is_bcu, SIGGEN=is_siggen, TYPE=size(buf,/TYPE), SWITCHB=switchb)
                if err ne adsec_error.ok then return,err
                n_el = n_elements(buf)
                n_board = range[1,ir]-range[0,ir]+1
                if total(check_buf ne rebin(reform([buf],n_el,1),n_el,n_board)) ne 0 then begin
                    message," Writing check failed", cont=(sc.debug eq 0B)
                    return, adsec_error.write_failed
                endif
            endif
        endfor
    endelse

return, adsec_error.ok

end

