; $Id: force_dsp_idle.pro,v 1.4 2008/04/08 09:29:41 labot Exp $
;+
;   NAME:
;    FORCE_DSP_IDLE
;
;   PURPOSE:
;    This function forces selected DSPs of the DSP/BCU/SIGGEN boards
;    to stop the DSP program, set in idle and re-initialize the DMA
;    communication channels.
;
;   USAGE:
;    err = force_dsp_idle(id_list [,/BCU|/SIGGEN|/SWITCH])
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
;    Created by Marco Xompero on 26 Jan 2007
;    marco@arcetri.astro.it
;    07 Feb 2007 MX
;       LUT of variable deleted and name refurbished. SWITCH BCU now managed.
;       Multiple crate sync not implemented yet.
;    27 Frb 2007 MX
;       Fixed bug managing SWITCH and CRATE BCUs.
;-

Function force_dsp_idle, id_list, BCU=is_bcu, SIGGEN=is_siggen, SWITCHB=switchb

    @adsec_common

    case fix(keyword_set(is_bcu)) or 2*keyword_set(is_siggen) or 4*keyword_set(SWITCHB) of
        0: begin
         dsp_list=id_list
         is_dsp_board = 1B
         set_pm = dsp_datasheet.data_mem_id
       end

       1: begin
         board_list=id_list
         is_dsp_board = 0B
         set_pm = bcu_datasheet.data_mem_id
       end

       2: begin
         board_list=id_list
         is_dsp_board = 0B
         board_list = [1,3,5] ;;;;;;;;;;;;;;;;;;;;;;;;cosi' fa schifo , bisogna sistemarlo bene per le accelerometer
         set_pm = siggen_datasheet.data_mem_id
       end

       4: begin
         board_list=id_list
         is_dsp_board = 0B
         set_pm = switch_datasheet.data_mem_id
       end

       else: begin
         message, "BCU and SIGGEN and SWITCH keywords are exclusive.", CONT=(sc.debug eq 0B)
         return, adsec_error.input_type
       end
    endcase

    ;
    ; stop the DSP program
    ;
    stop_prog_addr='00180359'XL
    buf = '00000020'XUL
    if is_dsp_board then $
        err = write_same_dsp(dsp_list, stop_prog_addr, buf) $
    else $
        err = write_same_board(board_list, stop_prog_addr, buf, SET_PM=set_pm, BCU=is_bcu, SIGGEN=is_siggen, SWITCHB=switchb)
    if err ne adsec_error.ok then return, err
    ;
    ; DMA initialization
    ;
    dma_init_addr='00180448'XL
    buf = ['00000000'XUL,'00000000'XUL,'00000000'XUL,'00000000'XUL]
    if is_dsp_board then $
        err = write_same_dsp(dsp_list, dma_init_addr, buf, /QUAD_WORD) $
    else $
        err = write_same_board(board_list, dma_init_addr, buf, BCU=is_bcu, SIGGEN=is_siggen, SET_PM=set_pm, SWITCHB=switchb, /QUAD_WORD)
    if err ne adsec_error.ok then return, err

    dma_init_addr='0018044c'XL
    buf = ['00000000'XUL,'00000000'XUL,'00000000'XUL,'00000000'XUL]
    if is_dsp_board then $
        err = write_same_dsp(dsp_list, dma_init_addr, buf, /QUAD_WORD) $
    else $
        err = write_same_board(board_list, dma_init_addr, buf, BCU=is_bcu, SIGGEN=is_siggen, SET_PM=set_pm, SWITCHB=switchb, /QUAD_WORD)
    if err ne adsec_error.ok then return, err

    dma_init_addr='001804e0'XL
    buf = ['00000000'XUL,'00000000'XUL,'000004c8'XUL,'000004c8'XUL]
    if is_dsp_board then $
        err = write_same_dsp(dsp_list, dma_init_addr, buf, /QUAD_WORD) $
    else $
        err = write_same_board(board_list, dma_init_addr, buf, BCU=is_bcu, SIGGEN=is_siggen, SET_PM=set_pm, SWITCHB=switchb, /QUAD_WORD)
    if err ne adsec_error.ok then return, err

    return, err
end
