; $Id: read_seq.pro,v 1.31 2009/05/22 16:34:12 labot Exp $
;
;+
;
; READ_SEQ
;
;   This function trasmits data from one or more DSPs to the host,
;   from the same internal addresses of all the specified DSPs.
;
;   err = read_seq(first_dsp, last_dsp, dsp_address, data_len, receive_data_buffer $
;               [, SET_PM=set_pm][, BCU=bcu][, SIGGEN=siggen][, /ULONG|/LONG])
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
;  data_len:            idl long, short or unsigned short int type scalar.
;                       Number of words per DSP to read.
;
;
; OUTPUTS
;
;  receive_data_buffer: Named variable. Idl float/ulong/long type array output.
;                     N_dsp rows and data_len columns.
;                       N_DSP=last_dsp-first_dsp+1. If TYPE, ULONG, LONG keywords
;                       are set the corresonding data type is output.
;                       By default the output is floating point.
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
;                       setPM = 2 (1L from IDL) -> SDRAM MEMORY
;                       setPM = 3 (1L from IDL) -> SRAM MEMORY
;                       setPM = 4 (1L from IDL) -> FLASH MEMORY
;
;    BCU:           used to communucate with the bcu board. Cannot be set
;                with SIGGEN keyword.
;
;    SIGGEN:          used to communicate with the Signal Generator board.
;                Cannot be set with the bcu keyword.
;
;    ULONG:      If set the output is translated to unsigned long (same bit sequence)
;     LONG:      If set the output is translated to signed long (same bit sequence)
;    TYPE:       int scalar. If passed the output is translated to the corresponding
;                data type. Allowed values: 3(long), 4(float) and 13(ulong). TYPE setting
;                overrides the ULONG and LONG keyword setting. If no ULONG, LONG or TYPE keyword
;                are set, default data type is float.
;
; HISTORY:
;
;   Thu Jun 24 18:07:21 1999, Armando Riccardi (AR)
;  Osservatorio Astrofisico di Arcetri, ITALY
;  <riccardi@riccardi.arcetri.astro.it>
;
;     13 Mar 2002, AR
;         n_dsp>256 is now handled.
;     10 Mar 2003, MX
;      LBT rewriting code.
;     29 Apr 2004, MX
;        get_mem_size function introduced.
;     04 May 2004, MX
;      debugging and variation SIGGEN address: from 0xFE to 0xFC.
;     10 May 2004, MX
;      now manage also the offset in the dsp memory by the xxx_datasheet.data_block_offset.
;     19 May 2004, MX
;      New FPGA mem manage support.
;     20 May 2004, AR
;       TYPE, ULONG and LONG keyword added
;       sc.min_timeout variable is now used to set the minimum allowed timeout.
;       help, /TRACEBACK removed in the debug mode
;     09 Feb 2004, MX
;       Bug with non-zero bcu number fixed.
;     29 Nov 2005, MX
;       Bug in different block manage fixed.
;     20 Jul 2006, AR+MX
;       fixed bug when addressing SIGGEN or BCU board 
;     07 Feb 2007, MX
;       New SWITCHB keyword added for managing SWITCH BCU. New mirror controller id
;       now correctly managed.
;     20 Mar 2007, MX, DZ
;       Multicrate operation added.
;-
;

Function read_seq, first_dsp_tmp, last_dsp_tmp, dsp_address_tmp, data_len_tmp, receive_data_buffer           $
                 , SET_PM = set_pm, SIGGEN = siggen, BCU = bcu, SWITCHB = switchb, MULTI_CRATE=multi_crate   $
                 , ULONG=set_ulong, LONG=set_long, TYPE=data_type

@adsec_common

;=================================================
; checks for the input parameters
;=================================================

;std definitions for testing
first_dsp = first_dsp_tmp
last_dsp = last_dsp_tmp
dsp_address = dsp_address_tmp
data_len = data_len_tmp

if n_elements(data_type) eq 0 then begin
    if keyword_set(set_long)+keyword_set(set_ulong) gt 1 then begin
        message, "ULONG and LONG keyword are exclusive", CONT=(sc.debug eq 0B)
        return, adsec_error.input_type
    endif
    case 1B of
        keyword_set(set_ulong): data_type=13
        keyword_set(set_long): data_type=3
        else: data_type=4
    endcase
endif else begin
    case data_type of
       13: ;ulong
       3:  ;long
       4:  ;float
       else: begin
         message, "Requested data type not valid (TYPE="+strtrim(data_type,2)+").", $
                  CONT=(sc.debug eq 0B)
         return, adsec_error.input_type
       end
    endcase
endelse


if n_elements(set_pm) eq 0 then set_pm=dsp_datasheet.data_mem_id
op_sel = byte(set_pm)
str="short int (IDL int)"
is_dsp = (op_sel eq dsp_datasheet.data_mem_id) or (op_sel eq dsp_datasheet.prog_mem_id)

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
        message, "first_dsp must be a valid dsp or board number", CONT=(sc.debug eq 0B)
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
            message, "last_dsp and first_dsp are not in the same BCU", $
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


;test the keywords
;check the bcu or the siggen keyword
if (keyword_set(BCU) and keyword_set(SIGGEN)) or (keyword_set(SWITCHB) and keyword_set(SIGGEN)) then begin
    message, "The BCU/SWITCH keyword and the SIGGEN keyword cannot be both set!", CONT=(sc.debug eq 0B)
    return, adsec_error.input_type;
endif

if keyword_set(BCU) xor keyword_set(SIGGEN) then begin

;check if in first_dsp there is a correct bcu number
       if (first_dsp lt 0) or (first_dsp gt adsec.n_bcu) then begin
           message, "The number of BCU in first_dsp is not correct!!!", CONT=(sc.debug eq 0B)
         return, adsec_error.input_type;
       endif

;codes definition for the bcu case
    if keyword_set(BCU) then begin
       bcu_num = first_dsp
       bcu_last = last_dsp
       new_first = 255;
       new_last = 255;
       mem_size = get_mem_size(bcu_datasheet, op_sel)
       if is_dsp then block_size=bcu_datasheet.data_block_offset  else block_size=0
    endif

;codes definition for the siggen case
    if keyword_set(SIGGEN) then begin
       bcu_num = first_dsp 
       bcu_last = last_dsp
       new_first = 252;
       new_last = 252;
       mem_size = get_mem_size(siggen_datasheet, op_sel)
       if is_dsp then block_size=siggen_datasheet.data_block_offset else block_size=0
    endif

endif else begin

    if keyword_set(SWITCHB) then begin
       bcu_num =  adsec.n_bcu
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
       if is_dsp then block_size=dsp_datasheet.data_block_offset else block_size=0

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

;test type of "data_len" variable
if test_type(data_len, /LONG, /INT, /UINT, N_EL=n_el) then begin
    message, "data_len must be a short or long-int", CONT=(sc.debug eq 0B)
    return, adsec_error.input_type
endif
;check n elements (must be 1)
if n_el ne 1 then begin
    message, "data_len must be a scalar", CONT=(sc.debug eq 0B)
    return, adsec_error.input_type
endif
data_len = long(data_len[0])
;check it's a valid reasonable length
if data_len[0] le 0 or $
     (dsp_address[0]-block_id_start*block_size)+data_len[0]-1 ge mem_size then begin
    message, "requested to read outside the DSP block memory limit", CONT=(sc.debug eq 0B)
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

;buffer sizing change with memory type
mcf = 1 + (bcu_last-bcu_num) 
case op_sel of
    dsp_datasheet.data_mem_id: receive_data_buffer = make_array(data_len[0], n_dsp*mcf, TYPE=data_type)
    dsp_datasheet.prog_mem_id: receive_data_buffer = make_array(data_len[0], n_dsp*mcf, TYPE=data_type)
    dsp_datasheet.sdram_mem_id: receive_data_buffer = make_array(data_len[0], n_board*mcf, TYPE=data_type)
    dsp_datasheet.sram_mem_id: receive_data_buffer = make_array(data_len[0], n_board*mcf, TYPE=data_type)
    dsp_datasheet.flash_mem_id: receive_data_buffer = make_array(data_len[0], n_board*mcf, TYPE=data_type)
    dsp_datasheet.fpgabuf_mem_id: receive_data_buffer = make_array(data_len[0], n_board*mcf, TYPE=data_type)
    else: begin
             message, "SET_PM keyword must be specified, byte and not equal to 1!!!", CONT=(sc.debug eq 0B)
             return, adsec_error.input_type
         end
endcase

if is_dsp then $
    timeout = long(data_len * 1000d * dsp_const.mean_transfer_time * n_dsp * 1.2) $
else $
    timeout = long(data_len * 1000d * dsp_const.mean_transfer_time * n_board * 1.2)
timeout = timeout > sc.min_timeout

if sc.ll_debug then begin
    print, 'Debugging data'
    help, first_dsp
    help, last_dsp
    help, bcu_num
    help, bcu_last
    help, new_first
    help, new_last
    help, n_dsp
    help, n_board
    help, op_sel
    help, block_size
    help, block_id_start
    help, timeout
    help, multi_crate
    print, 'dsp address ', strtrim(string(dsp_address,format='(Z8.8)'),2)
    print, 'data len ' ,strtrim(string(data_len,format='(Z8.8)'),2)
    print, 'sc.debug ', sc.debug
    print, 'Size of mem ', strtrim(string(mem_size,format='(Z8.8)'),2)
    print, 'Buffers Dimensions ', size(receive_data_buffer, /dim)
endif

;Dummy case
if sc.host_comm eq "Dummy" then begin
    common dummy_comm_block, dummy_comm_seed
    receive_data_buffer[*] = 10*randomu(dummy_comm_seed, size(receive_data_buffer,/DIM))
    err = adsec_error.ok
endif else begin

    if keyword_set(MULTI_CRATE) then begin
        err = read_seq_multi_wrap(sc.mirrctrl_id[bcu_num], sc.mirrctrl_id[bcu_last], new_first, new_last, dsp_address, data_len, $
                             	  receive_data_buffer, timeout*mcf, op_sel)
    endif else begin
        ;Natural case and call dlm
        err = read_seq_wrap(sc.mirrctrl_id[bcu_last], new_first, new_last, dsp_address, data_len,                          $
                       	    receive_data_buffer, timeout, op_sel)
    endelse
endelse

    if err ne adsec_error.ok then begin
        message, "READING ERROR BCU", CONT=(sc.debug eq 0B)
        return, err
    endif

return, err

end
