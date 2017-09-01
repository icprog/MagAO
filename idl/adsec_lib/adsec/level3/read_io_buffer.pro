; $Id: read_io_buffer.pro,v 1.3 2004/11/02 15:57:40 marco Exp $

;+
;
; NAME
;
;   READ_IO_BUFFER
;
;   reads the specified buffers in the dsp list.
;
;   err = read_IO_buffer(dsp_list, buf_num_list, buflen, buf $
;                        [, /ULONG|/LONG|TYPE=data_type])
;
; INPUTS
;
;   dsp_list:    byte vector. DSP list. sc.all_dsp is allowed.
;
;   buf_num_list: byte/short/long vector. List of buffer to read from.
;
;   buflen:      short. Buffer length in words to read from SDRAM.
;
; OUTPUTS
;
;   err:         long scalar. see adsec_error structure defined in init_gvar.pro
;
; buf:        named variable. Float/long/ulong vector[buflen,M,N], where
;             M is the number of buffers and N is the number of DSP to read from.
;
; KEYWORDS
;
;    ULONG:      If set the output is translated to unsigned long (same bit sequence)
;     LONG:      If set the output is translated to signed long (same bit sequence)
;    TYPE:       int scalar. If passed the output is translated to the corresponding
;                data type. Allowed values: 3(long), 4(float) and 13(ulong). TYPE setting
;                overrides the ULONG and LONG keyword setting. If no ULONG, LONG or TYPE keyword
;                are set, default data type is float.
;
; EXAMPLES
;
;   None.
;
; HISTORY
;   A. Riccardi (AR): rewritten to match LBT specifications
;
;   04 Aug 2004: AR.
;     SDRAM address computed using sdram_IO_addr
;   02 Nov 2004, MX
;     Adsec.all changed in sc.all.
;-
;
function read_IO_buffer, dsp_list,  buf_num, buflen, buf, ULONG=set_ulong, LONG=set_long, TYPE=data_type
    ;; not yet supported keywords
    ;; WAIT_FOR_STOP=wait4stop, POLLING_TIME=poll_time

@adsec_common

if check_dsp(dsp_list) then begin
    message, "Incorrect list of DSPs.", CONT= (sc.debug eq 0B)
    return, adsec_error.input_type
endif
if dsp_list[0] eq sc.all then $
    the_dsp_list = indgen(adsec.n_dsp) $
else $
    the_dsp_list = fix(dsp_list)

if test_type(buf_num, /NOFLOAT, N_EL=n_buf_num) then begin
    message, "buf_num must be integer.", CONT= (sc.debug eq 0B)
    return, adsec_error.input_type
endif
if n_elements(uniq(buf_num, sort(buf_num))) ne n_buf_num then begin
    message, "there are repeated buffer numbers in buf_num.", CONT= (sc.debug eq 0B)
    return, adsec_error.input_type
endif
if (total(buf_num lt 0) ne 0) or (total(buf_num ge dsp_const.n_dataIO/adsec.n_dsp_per_board) ne 0) then begin
    message, "buf_num contents buffer numbers out of range. 0<=buf_num<=" $
             +strtrim(dsp_const.n_dataIO/adsec.n_dsp_per_board-1,2), CONT=(sc.debug eq 0B)
    return, adsec_error.buff_not_valid
endif
the_buf_num = fix(buf_num)

err = check_working_IO(the_dsp_list, the_buf_num, are_working, WORKING_IO_MAP=working_io_map)
if err ne adsec_error.ok then return, err
if are_working then begin
    message, "WARNING! Some of the specified buffers are currently in use.", /INFO
endif

dsp_onboard_id = the_dsp_list mod adsec.n_dsp_per_board
dsp_board_list = the_dsp_list/adsec.n_dsp_per_board
n_buf_num = n_elements(the_buf_num)
n_dsp_list = n_elements(the_dsp_list)
buf = make_read_array(buflen, n_buf_num, n_dsp_list, ULONG=set_ulong, LONG=set_long, TYPE=data_type)
the_data_type = size(buf, /TYPE)

for dsp_id = 0,adsec.n_dsp_per_board-1 do begin
    idx=where(dsp_onboard_id eq dsp_id, count)
    if count gt 0 then begin
        sub_dsp_board = dsp_board_list[idx]

        for i=0,n_buf_num-1 do begin
            sdram_buf_addr = sdram_IO_addr(dsp_id, the_buf_num[i])
            err = read_seq_board(sub_dsp_board, sdram_buf_addr, buflen, sub_buf $
                                , SET_PM=dsp_datasheet.sdram_mem_id, TYPE=the_data_type)
            if err ne adsec_error.ok then return, err
            buf[*,i,sub_dsp_board*adsec.n_dsp_per_board+dsp_id] = sub_buf
        endfor
    endif
endfor

return,adsec_error.ok

end
