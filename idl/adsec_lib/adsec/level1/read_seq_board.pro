; $Id: read_seq_board.pro,v 1.12 2009/05/22 15:55:30 labot Exp $

;+
; READ_SEQ_BOARD
;
; err = read_seq_board(board_list, addr, n_words, buf [, SET_PM=set_pm] $
;                    [, /BCU|/SIGGEN][, /LONG|/ULONG|TYPE=data_type])
;
; board_list: int vector or scalar. Vector of DSP/BCU/SIGGEN board ID
;             numbers. If no BCU nor SIGGEN keyword are set, board
;             list refers to DSP boards
;
; addr:       long scalar. Starting address to read from.
;
; n_words:    long scalar. Number of words to read.
;
; buf:        named variable. On output contains a
;             Float/ulong/long vector[n_words,N], where
;             N is the number of DSP to read from.
; KEYWORDS
;
; SET_PM:     byte scalar. Code identifing the type of memory to
;             write to. Default value is dsp_datasheet.sdram_mem_id,
;             it is used to read from SDRAM data memory.
;             See xxxx_mem_id filds in dsp_datasheet structure for the
;             allowed values for SET_PM keyword.
;
; BCU:        if it is set, board_list refers to BCU board IDs.
; SWITCH:     if it is set, the actions are performed on SWITCH bCU
;
; SIGGEN:     if it is set, board_list refers to SIGGEN board IDs.
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
;   24 Mar 2004. Written by A. Riccardi (AR)
;   Osservatorio Asrofisico di Arcetri, ITALY
;   <riccardi@arcetri.astro.it>
;
;   20 May 2004, AR
;       ulong and long buffer are now allowed (peviously only float)
;       if sc.debug is set, then SET_PM has not a default value (procedure stops)
;   04 Aug 2004, Marco Xompero
;       the number of words can be now uint and ulong.
;   02 Nov 2004, MX
;       Adsec.all changed in sc.all.
;   07 Feb 2007 MX
;       LUT of variable deleted and name refurbished.
;       SwitchBCU now managed
;-
function read_seq_board, board_list, addr, n_words, buf, SET_PM=set_pm $
                         ,BCU=is_bcu, SIGGEN=is_siggen, SWITCHB=is_switchb $
                         ,ULONG=set_ulong, LONG=set_long, TYPE=data_type

    @adsec_common

    if test_type(n_words, /INT, /LONG, /UINT, /ULONG,  N_EL=n_el) then begin
        message, "n_words must be an integer", CONT=(sc.debug eq 0B)
        return, adsec_error.input_type
    endif
    if n_el ne 1 then begin
        message, "n_words must be a scalar", CONT=(sc.debug eq 0B)
        return, adsec_error.input_type
    endif

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

;            if check_list(the_dsp_list, adsec.n_crates) eq adsec_error.ok then begin
             if check_list(the_board_list, adsec.n_crates) eq adsec_error.ok then begin
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

    if multi_crate then begin

        err = read_seq(0,endval, addr, n_words, buf, MULTI_CRATE=multi_crate, SET_PM=set_pm, $ 
                        ULONG=set_ulong, LONG=set_long, TYPE=data_type, BCU=is_bcu, SIGGEN=is_siggen, SWITCHB=is_switchb)
        if err ne adsec_error.ok then return, err


    endif else begin

        i = 0L
        for ir=0,range_count-1 do begin
            ii = i+range[1,ir]-range[0,ir]
            idx = sort_idx[i:ii]

            err = read_seq(range[0,ir], range[1,ir], addr, n_words, dummy_buf $
                           ,SET_PM=set_pm, BCU=is_bcu, SIGGEN=is_siggen, SWITCHB=is_switchb $
                           ,ULONG=set_ulong, LONG=set_long, TYPE=data_type)
            if err ne adsec_error.ok then return, err

            if ir eq 0 then buf = make_array(n_words, n_board, TYPE=size(dummy_buf,/TYPE))
            buf[*,idx] = dummy_buf

            i = ii+1
        endfor
    
    endelse

    return, adsec_error.ok

end
