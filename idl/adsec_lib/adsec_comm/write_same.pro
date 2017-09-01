; $Id: write_same.pro,v 1.27 2009/04/10 13:52:15 labot Exp $
;
;+
;
; WRITE_SAME
;
; This function trasmits data from the host to one or more DSPs,
; writing the same data in the same internal addresses of all the
; specified DSPs.
;
; err = write_same(first_dsp, last_dsp, dsp_address, send_data_buffer, $
;                  SET_PM=set_pm, BCU=bcu, SIGGEN=siggen)
;
; INPUTS
;
;  first_dsp:          idl int type scalar.
;                    First DSP to which the data are sent.
;                If the BCU or SIGGEN keyword is set, in this field there is
;                the BCU number to read.
;
;  last_dsp :           idl int type scalar.
;                    Last DSP to which the data are sent.
;                If the BCU or SIGGEN keyword is set, this field is ignored.
;
;  dsp_address:         idl long int type scalar.
;                Internal DSP address from which the data are readed.
;
;  send_data_buffer: idl float/long/ulong vector. Vector of data to send.
;
; OUTPUTS
;
;   err:             long-int scalar. (see adsec_error in init_gvar.pro)
;
; KEYWORDS
;
;   SET_PM:          if set the dsp_address specify an address of the
;                    program memory (PM), instead of the data memory (DM).
;                (must be specified)
;
;                /* Opcode definition
;                       setPM = 0 (OL from IDL) -> DSP MEMORY
;                       setPM = 1 (1L from IDL) -> EMPTY MEMORY
;                       setPM = 2 (2L from IDL) -> SDRAM MEMORY
;                       setPM = 3 (3L from IDL) -> SRAM MEMORY
;                       setPM = 4 (4L from IDL) -> FLASH MEMORY
;                       setPM = 5 (5L from IDL) -> FPGA MEMORY
;
;    BCU:           used to communucate with the bcu board. Cannot be set
;                with SIGGEN keyword.
;
;    SIGGEN:          used to communicate with the Signal Generator board.
;                Cannot be set with the bcu keyword.
;
; HISTORY:
;
;     Jun 1999, written by Armando Riccardi
;       <riccardi@arcetri.astro.it>
;     10 Mar 2003, MX
;      LBT rewriting code.
;     29 Apr 2004, MX
;        get_mem_size function introduced.
;     04 May 2004, MX
;      debugging and variation SIGGEN address: from 0xFE to 0xFC.
;     10 May 2004, MX
;      now manage also the offset in the dsp memory by the xxx_datasheet.data_block_offset.
;     18 May 2004, MX
;      QUAD_WORD keyword to allow the DMA writing in the DSP program inflating.
;     19 May 2004, MX
;      New FPGA mem manage support.
;     20 May 2004, AR
;       ulong and long input data are alowed (not only float).
;       sc.min_timeout variable is now used to set the minimum allowed timeout.
;       help, /TRACEBACK removed in the debug mode
;     28 Jul 2004, MX
;       Minor changes.
;     27 Ago 2004, MX
;       bug in call_external parameter names fixed (new_first, new_last)
;     09 Feb 2004, MX
;       Bug with non-zero bcu number fixed.
;     20 Jul 2006, AR+MX
;       fixed bug when addressing SIGGEN or BCU board 
;     07 Feb 2007, MX
;       New SWITCHB keyword added for managing SWITCH BCU. New mirror controller id
;       now correctly managed.
;-

Function write_same, first_dsp_tmp, last_dsp_tmp, dsp_address_tmp, send_data_buffer $
                   , SET_PM = set_pm, SIGGEN = siggen, BCU = bcu, SWITCHB = switchb, QUAD_WORD=quad_word, MULTI_CRATE=multi_crate

    @adsec_common
    if ~!AO_CONST.side_master then return, adsec_error.IDL_NOT_SIDE_MASTER


    ;=================================================
    ; checks for the input parameters
    ;=================================================

    ;std definitions for testing
    first_dsp = first_dsp_tmp
    last_dsp = last_dsp_tmp
    dsp_address = dsp_address_tmp

    if n_elements(set_pm) eq 0 then set_pm=dsp_datasheet.data_mem_id
    op_sel = byte(set_pm)
    str="short int (IDL int)"
    is_dsp = (op_sel eq dsp_datasheet.data_mem_id) or (op_sel eq dsp_datasheet.prog_mem_id)

    ;check for the QUAD_WORD keyword and set the flags field
    if n_elements(quad_word) eq 0 then flags = 1l else flags = 3l

    ;contitions based on the set_pm parameter: case board based or dsp based
    ;if not is_dsp then begin
    ;   first_dsp = first_dsp * adsec.n_dsp_per_board
    ;   last_dsp = last_dsp * adsec.n_dsp_per_board;
    ;endif

    ;test type of "first_dsp" variable
    ;check int type
    if test_type(first_dsp, /INT, N_EL=n_el) then begin
        message, "first_dsp must be a "+str, CONT=(sc.debug eq 0B)
        return, adsec_error.input_type
    endif
    ;check n elements (must be 1)
    if n_el ne 1 then begin
        message, "first_dsp must be a scalar", CONT=(sc.debug eq 0B)
        return, adsec_error.input_type
    endif
    if keyword_set(bcu) or keyword_set(siggen) then begin
        ;check it's a valid bcu/siggen number
        if first_dsp[0] gt adsec.n_bcu-1 or first_dsp[0] lt 0 then begin
            message, "first_dsp must be a valid bcu or siggen number", CONT=(sc.debug eq 0B)
           return, adsec_error.input_type
        endif
    endif else begin
        ;check it's a valid dsp number
       if (not is_dsp) then first_dsp = first_dsp *adsec.n_dsp_per_board
        if first_dsp[0] gt adsec.last_dsp or first_dsp[0] lt 0 then begin
            message, "first_dsp must be a valid dsp or dsp board number", CONT=(sc.debug eq 0B)
            return, adsec_error.input_type
        endif

        ;test type of "last_dsp" variable
        ;check int type
        if test_type(last_dsp, /INT, N_EL=n_el) then begin
            message, "last_dsp must be a "+str, CONT=(sc.debug eq 0B)
            return, adsec_error.input_type
        endif
        ;check n elements (must be 1)
        if n_el ne 1 then begin
            message, "last_dsp must be a scalar", CONT=(sc.debug eq 0B)
            return, adsec_error.input_type
        endif
        ;check it's a valid dsp number
        if (not is_dsp) then last_dsp = last_dsp *adsec.n_dsp_per_board
        if last_dsp[0] gt adsec.last_dsp then begin
            message, "last_dsp must be a valid dsp or board number", CONT=(sc.debug eq 0B)
            return, adsec_error.input_type
        endif
        ;check if first and last are congruent
        if last_dsp lt first_dsp then begin
            message, "last_dsp cannot be less then the first_one", $
              CONT=(sc.debug eq 0B)
            return, adsec_error.input_type
        endif

        if ~keyword_set(MULTI_CRATE) then begin
            if last_dsp / (adsec.n_board_per_BCU * adsec.n_dsp_per_board) ne $
                first_dsp / (adsec.n_board_per_BCU * adsec.n_dsp_per_board) then begin
                message, "last_dsp and first dsp must be in the same BCU", $
                  CONT=(sc.debug eq 0B)
                return, adsec_error.input_type
            endif
        endif
    endelse

    ;test type of "dsp_address" variable
    if test_type(dsp_address, /LONG, N_EL=n_el) then begin
        message, "dsp_address must be a LONG int", CONT=(sc.debug eq 0B)
        return, adsec_error.input_type
    endif
    ;check n elements (must be 1)
    if n_el ne 1 then begin
        message, "dsp_address must be a scalar", CONT=(sc.debug eq 0B)
        return, adsec_error.input_type
    endif

    ;check it's a valid float array
    if test_type(send_data_buffer, /FLOAT, /LONG, /ULONG, $
                 N_EL=words2send ) then begin
        message, "send_data buffer must be a float/long/ulong vector", $
          CONT=(sc.debug eq 0B)
        return, adsec_error.input_type
    endif


    ;test the keywords
    ;check the bcu and siggen keyword
    if (keyword_set(BCU) and keyword_set(SIGGEN)) or (keyword_set(SWITCHB) and keyword_set(SIGGEN)) then begin
        message, "The BCU/SWITCH keyword and the SIGGEN keyword cannot be both set!", CONT=(sc.debug eq 0B)
        return, adsec_error.input_type;
    endif

    if keyword_set(BCU) xor keyword_set(SIGGEN) then begin

    ;check if in first_dsp there is a correct bcu number
           if (first_dsp lt 0) or (first_dsp gt adsec.n_bcu) then begin
               message, "The number of BCU in first_dsp is not correct!!! ("+strtrim(first_dsp,2)+")", CONT=(sc.debug eq 0B)
               return, adsec_error.input_type;
           endif

;set the mem size for the board mem case
;     mem_size = bcu_datasheet.mem_size;

;set the codes for the bcu case
        if keyword_set(BCU) then begin
            bcu_num = first_dsp
            bcu_last = last_dsp
            new_first = 255;
            new_last = 255;
            mem_size = get_mem_size(bcu_datasheet, op_sel)
            if is_dsp then block_size=bcu_datasheet.data_block_offset  else block_size=0
        endif

        ;set the codes for the siggen case
        if keyword_set(SIGGEN) then begin
            bcu_num = first_dsp 
            bcu_last = last_dsp
            new_first = 252;
            new_last = 252;
            mem_size = get_mem_size(siggen_datasheet, op_sel)
            if is_dsp then block_size=siggen_datasheet.data_block_offset  else block_size=0
        endif

    endif else begin

        if keyword_set(SWITCHB) then begin
            bcu_num = adsec.n_bcu
            bcu_last = adsec.n_bcu
            new_first = 255;
            new_last = 255;
            mem_size = get_mem_size(switch_datasheet, op_sel)
            if is_dsp then block_size=switch_datasheet.data_block_offset  else block_size=0

        endif else begin

            bcu_num = first_dsp / (adsec.n_board_per_BCU * adsec.n_dsp_per_board)
            bcu_last = last_dsp / (adsec.n_board_per_BCU * adsec.n_dsp_per_board)
            new_first = first_dsp mod (adsec.n_board_per_BCU * adsec.n_dsp_per_board)
            new_last = last_dsp mod (adsec.n_board_per_BCU * adsec.n_dsp_per_board)
            mem_size = get_mem_size(dsp_datasheet, op_sel)
            if is_dsp then block_size=dsp_datasheet.data_block_offset  else block_size=0
        endelse

    endelse

    ;check it's a valid mem address
    if is_dsp then block_id_start = dsp_address[0] / block_size else block_id_start=0

    if (block_id_start ge 4) then begin
        message, "dsp_address is out of range", CONT=(sc.debug eq 0B)
        return, adsec_error.input_type
    endif

    if (dsp_address[0] lt 0) or (dsp_address[0]-block_id_start*block_size ge mem_size) then begin
        message, "dsp_address is out of range", CONT=(sc.debug eq 0B)
        return, adsec_error.input_type
    endif

    ;check for sufficient mem size
    if (dsp_address[0]-block_id_start*block_size)+words2send -1 ge mem_size then begin
        message, "requested to write outside the DSP memory limit", CONT=(sc.debug eq 0B)
        return, adsec_error.input_type
    endif

    ;
    ;end of the checks
    ;=============================================================
    ;Begin of the real routine
    ;=============================================================

    ;calculating the number of dsp and board to read
    if is_dsp then begin
        n_dsp = new_last-new_first+1
        n_board = 0
    endif else begin
        n_dsp = 0
        n_board = (new_last-new_first) / adsec.n_dsp_per_board +1
    endelse

    if is_dsp then $
        timeout = long(words2send * 1000d * dsp_const.mean_transfer_time * n_dsp *1.2) $
    else $
        timeout = long(words2send * 1000d * dsp_const.mean_transfer_time * n_board * 1.2)
        timeout = timeout > sc.min_timeout

    if sc.ll_debug then begin
        print, 'Debugging data'
        help, first_dsp
        help, last_dsp
        help, bcu_num
        help, bcu_last
        help, new_first
        help, new_last
        help, words2send
        help, op_sel
        help, block_size
        help, block_id_start
        help, words2send
        help, timeout
        help, flags
        print, 'dsp address ', strtrim(string(dsp_address,format='(Z8.8)'),2)
        print, 'words2send ' ,strtrim(string(words2send,format='(Z8.8)'),2)
        print, 'sc.debug ', sc.debug
        print, 'Size of mem ', strtrim(string(mem_size,format='(Z8.8)'),2)
        print, 'Buffers Dimensions ', size(send_data_buffer, /dim)
    endif

    if sc.host_comm eq "Dummy" then begin
        err = adsec_error.ok
    endif else begin

        if keyword_set(MULTI_CRATE) then begin
        err = write_same_multi_wrap(sc.mirrctrl_id[bcu_num], sc.mirrctrl_id[bcu_last], new_first, new_last, dsp_address, $
                            		words2send, send_data_buffer, timeout, op_sel, flags)
        endif else begin
    ;Natural case and call external to .so
        err = write_same_wrap(sc.mirrctrl_id[bcu_num], new_first, new_last, dsp_address, $
                         	  words2send, send_data_buffer, timeout, op_sel, flags)
        endelse
    endelse
        if err ne adsec_error.ok then begin
            message, "WRITING ERROR BCU", CONT=(sc.debug eq 0B)
            return, err
        endif

    return, err

end
