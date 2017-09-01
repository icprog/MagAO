; $Id: start_program.pro,v 1.4 2007/02/20 15:30:38 labot Exp $
;+
;   NAME:
;    START_PROGRAM
;
;   PURPOSE:
;    This function forces selected DSPs of the DSP/BCU/SIGGEN boards
;    to start the DSP program.
;
;   USAGE:
;    err = start_program(id_list [,/BCU|/SIGGEN|/SWITCH])
;
;   INPUT:
;    id_list:  int vector/scalar. DSP list in case BCU and SIGGEN and SWITCH are
;              not set. In case BCU is set id_list corresponds to
;              the list of BCUs for which the corresponding DSP has
;              to be set in idle. The same in case SIGGEN is set. The SWITCH bcu is only one
;   OUTPUT:
;    Error code.
;
;   KEYWORDS:
;    BCU:    keyword to addres the CrateBCU.
;    SIGGEN: keyword to addres the Siggen/Accelerometers Boards.
;    SWITCH: keyword to addres the SwitchBCU.
;
;   HISTORY:
;   Written by Armando Riccardi on 27 May 2004
;   Updated by Marco Xompero on 26 Jan 2007
;   marco@arcetri.astro.it
;-

Function start_program, id_list, BCU=is_bcu, SIGGEN=is_siggen, SWITCHB=is_switch

    @adsec_common

    case fix(keyword_set(is_bcu)) or 2*keyword_set(is_siggen) or 4*keyword_set(is_switch) of
        0: begin
         dsp_list=id_list
         is_dsp_board = 1B
       end

       1: begin
         board_list=id_list
         is_dsp_board = 0B
       end

       2: begin
         board_list=id_list
         is_dsp_board = 0B
       end

       4: begin
         board_list=id_list
         is_dsp_board = 0B
       end

       else: begin
         message, "BCU and SIGGEN and SWITCH keywords are exclusive.", CONT=(sc.debug eq 0B)
         return, adsec_error.input_type
       end
    endcase


    ;
    ; start the DSP program
    ;
    startprog_addr='00180730'XL
    if is_dsp_board then $
        err = write_same_dsp(dsp_list, startprog_addr, 0UL) $
    else $
      err = write_same_board(board_list, startprog_addr, 0UL, BCU=is_bcu, SIGGEN=is_siggen, SET_PM=0, SWITCHB=is_switch)
    if err ne adsec_error.ok then return, err
    return, err

end
