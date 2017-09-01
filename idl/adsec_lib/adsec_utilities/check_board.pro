; $Id: check_board.pro,v 1.3 2007/03/26 17:43:38 labot Exp $
;+
; HYSTORY
;   13 July 2004 written by A. Riccardi
;-

function check_board, board_list, NOALL=no_all_boards $
  , DSP=is_dsp, BCU=is_bcu, SIGGEN=is_siggen

    @adsec_common 

    nn = total([keyword_set(is_dsp),keyword_set(is_bcu),keyword_set(is_siggen)])
    if nn eq 0 then begin
        is_dsp=1
    endif else begin
        if nn gt 1 then begin
            message, "Only one keyword among DSP, BCU and SIGGEN can be set.", CONT=(sc.debug eq 0B)
            return, adsec_error.IDL_INPUT_TYPE
        endif
    endelse

    case 1B of
        keyword_set(is_dsp): n_max_board = adsec.n_bcu*adsec.n_board_per_bcu
        keyword_set(is_bcu): n_max_board = adsec.n_bcu
        keyword_set(is_siggen): n_max_board = adsec.n_bcu
    endcase
    
    return, check_list(board_list, n_max_board, NOALL=no_all_boards, /GREAT)
end
