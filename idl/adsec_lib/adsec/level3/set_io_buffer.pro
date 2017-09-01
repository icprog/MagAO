; $Id: set_io_buffer.pro,v 1.13 2006/11/14 11:20:14 labot Exp $

;+
;
; NAME
;
;   SET_IO_BUFFER
;
;   sets the parameters for the future use of a IO buffer
;   if the specified buffer is currently in use in some
;   passed dsp (see dsp_map.data_IO_start and dsp_map.data_IO_process),
;   the setting is not performed and an error is returned.
;
;   err = set_IO_buffer(dsp_list, buf_num, dm_addr, length, $
;                       trig_dm_addr, trig_val, $
;                       WRITE_TO_MEM=write, DECIMATION=dec, $
;                       CIRCULAR=circular, TRIG_ENABLE=trig_enable, $
;                       START_ENABLE=start_enable, RECORD_SIZE=rec_size)
;
; INPUTS
;
;   dsp_list:    byte vector. DSP list. sc.all_dsp is allowed.
;
;   buf_num:     byte/short/long vector. Buffer numbers to set.
;                The allowed values are 0,1,2,3,4,5,6,7.
;
;   dm_addr:     short vector. Data memory addresses linked to the corresponding
;                buffers.
;
;   length:      short or unsigned short scalar. Length of the buffer in records.
;                Maximum value is 'FFFF'XU.
;                The amount of used buffer words are given by length*RECORD_SIZE
;
;   trig_dm_addr: short scalar. Data memory address of the variable used
;                 as trigger.
;
;   trig_val:    short scalar. Trigger value. When the value in the memory
;                location trig_dm_addr is equal to trig_val and the buffer is
;                enabled (see the enable_IO_burrer function), the IO buffer
;                starts to operate.
;
; OUTPUTS
;
;   err:         long scalar. see adsec_error structure defined in init_gvar.pro
;
; KEYWORDS
;
;   WRITE2MEM:   if set, the content of the buffer is written (one word a time)
;                in the memory location specified by dm_addr. If it is not set,
;                the values in the memory location dm_addr are saved (one word a time)
;                in the buffer.
;
;   DECIMATION:  short scalar. Decimation factor. The buffer operates using a temporal
;                sampling of (DECIMATION+1)*adsec.sampling_time.
;
;   CIRCULAR:    if set, the buffer is used as a circular buffer. Otherwise
;                the buffer stop to operate after it is scanned once.
;
;   TRIG_ENABLE: if set, the buffer trigger is enabled.
;
;   START_ENABLE: if set, the data buffer scan is enabled to start.
;
;   RECORD_SIZE: byte scalar. number of consecutive words to transfer
;                per buffer iteration. If it is not set, 1 is assumed.
;
;  HISTORY
;    ?? ???? ????: written by A. Riccardi (AR)
;    13 July 2004: AR
;      SYNC_ENABLED keyword removed
;      RECORD_SIZE keyword added
;      completely re-written to match LBT specifications
;
;    21 Jul 2004: AR
;      Removed keyword TRIG_ENABLE in the enable_IO_buffer call.
;
;    30 Jul 2004: AR
;      Rewritten checks on length. Record_size value is now considered for the checks.
;
;    04 Aug 2004: AR
;      check on buffer length takes in account the record length now.
;      SDRAM address computed using sdram_IO_addr.
;   02 Nov 2004, MX
;     Adsec.all changed in sc.all.
;
;    30 Nov 2004: AR
;      help modification
;     March 2005, MX
;     Added the CIRCULAR keyword.
;    01 Aug 2005, MX
;     Removed the patch for the circular buffers.
;-
;
function set_IO_buffer, dsp_list, buf_num, dm_addr, length, $
                        trig_dm_addr, trig_val, $
                        WRITE_TO_MEM=write, DECIMATION=dec, $
                        CIRCULAR=circular, TRIG_ENABLE=trig_enable, $
                        START_ENABLE=buffer_enable, $
                        NO_CHECK_LENGHT=no_check_lenght, RECORD_SIZE=rec_size

@adsec_common

if check_dsp(dsp_list) then begin
    message, "Incorrect list of DSPs.", CONT= (sc.debug eq 0B)
    return, adsec_error.input_type
endif
if dsp_list[0] eq sc.all then $
    the_dsp_list = indgen(adsec.n_dsp) $
else $
    the_dsp_list = fix(dsp_list)

if n_elements(rec_size) eq 0 then begin
    the_rec_size=1B
endif else begin
    if test_type(rec_size, /INT, /BYTE, N_EL=n_rec_size) then begin
        message, "record_size be integer.", CONT= (sc.debug eq 0B)
        return, adsec_error.input_type
    endif
    if n_rec_size ne 1 then begin
        message, "record_size must be a scalar.", CONT= (sc.debug eq 0B)
        return, adsec_error.input_type
    endif
    if (rec_size le 0) or (rec_size gt 'FF'X) then begin
        message, "Must be 0<rec_size<=255", CONT=(sc.debug eq 0B)
        return, adsec_error.buff_not_valid
    endif
    the_rec_size = byte(rec_size)
endelse

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

if test_type(dm_addr, /NOFLOAT, N_EL=n_el) then begin
    message, "dm_addr must be an integer", CONT=(sc.debug eq 0B)
    return, adsec_error.input_type
endif
if n_el ne n_buf_num then begin
    message, "dm_addr must have the same number of elements of buf_num", CONT=(sc.debug eq 0B)
    return, adsec_error.input_type
endif
if not is_valid_addr(dm_addr, dsp_datasheet, dsp_datasheet.data_mem_id) then begin
    message, "dsp_address is out of range", CONT=(sc.debug eq 0B)
    return, adsec_error.out_range_addr
endif
the_dm_addr = ulong(dm_addr)

if test_type(length, /INT, /UINT, N_EL=n_el) then begin
    message, "length must be integer.", CONT= (sc.debug eq 0B)
    return, adsec_error.input_type
endif
if n_el ne 1 then begin
    message, "length must be a scalar.", CONT=(sc.debug eq 0B)
    return, adsec_error.input_type
endif
if not keyword_set(no_check_lenght) then begin
    length_in_words = ulong(length[0])*the_rec_size
    if (length_in_words lt 1) || (length_in_words gt dsp_const.max_data_IO_len) then begin
        message, "length in words is out of range.", CONT=(sc.debug eq 0B)
        return, adsec_error.IO_len_not_valid
    endif
endif
the_length = uint(length[0])


if (n_elements(trig_dm_addr) ne 0) && (n_elements(trig_val) ne 0) then begin
    if keyword_set(trig_enable) then begin
        if test_type(trig_dm_addr, /NOFLOAT, N_EL=n_el) then begin
            message, "trig_dm_addr must be a short-int", CONT=(sc.debug eq 0B)
            return, adsec_error.input_type
        endif
        if n_el ne 1 then begin
            message, "trig_dm_addr must be a scalar", CONT=(sc.debug eq 0B)
            return, adsec_error.input_type
        endif
        if ~ is_valid_addr(trig_dm_addr, dsp_datasheet, dsp_datasheet.data_mem_id) then begin
            message, "trig_dm_addr is out of range", CONT=(sc.debug eq 0B)
            return, adsec_error.out_range_addr
        endif
        the_trig_dm_addr = long(trig_dm_addr[0])

        if test_type(trig_val, /LONG, /ULONG, /FLOAT, N_EL=n_el) then begin
            message, "trig_val must be long integer or float.", CONT=(sc.debug eq 0B)
            return, adsec_error.input_type
        endif
        if n_el ne 1 then begin
            message, "trig_val must be a scalar.", CONT=(sc.debug eq 0B)
            return, adsec_error.input_type
        endif
        the_trig_val = trig_val[0]
    endif else begin
        message, "WARNING: passed trigger variable without setting TRIG_ENABLE keyword,", /INFO
        message, "trigger will not configured.", /INFO
    endelse
endif else begin
    if keyword_set(trig_enable) then begin
        message, "The trig_dm_addr and trig_val must be defined when the trigger is enabled.", CONT=(sc.debug eq 0B)
        return, adsec_error.input_type
    endif
endelse

if n_elements(dec) eq 0 then begin
    the_dec = 0
endif else begin
    if test_type(dec, /INT, /UINT, N_EL=n_el) then begin
        message, "DECIMATION must be integer.", CONT=(sc.debug eq 0B)
        return, adsec_error.input_type
    endif
    if n_el ne 1 then begin
        message, "DECIMATION must be a scalar.", CONT=(sc.debug eq 0B)
        return, adsec_error.input_type
    endif
    if dec[0] lt 0 or dec[0] gt adsec.max_dec_val then begin
        message, "DECIMATION is out of range.", CONT=(sc.debug eq 0B)
        return, adsec_error.dec_not_valid
    endif
    the_dec = uint(dec[0])
endelse

err = check_working_IO(the_dsp_list, the_buf_num, are_working, WORKING_IO_MAP=working_io_map)
if err ne adsec_error.ok then return, err
if are_working then begin
    message, "Some of the specified buffers are currently in use.", CONT=sc.debug eq 0B
    return, adsec_error.IDL_BUF_NOT_STOP
endif

dsp_onboard_id = the_dsp_list mod adsec.n_dsp_per_board
dsp_board_list = the_dsp_list/adsec.n_dsp_per_board

for dsp_id = 0,adsec.n_dsp_per_board-1 do begin
    idx=where(dsp_onboard_id eq dsp_id, count)
    if count gt 0 then begin
        sub_dsp_board = dsp_board_list(idx)
        sub_buf_num = the_buf_num+dsp_id*(dsp_const.n_dataIO/adsec.n_dsp_per_board)
        n_sub_buf_num = n_elements(sub_buf_num)

        for i=0,n_sub_buf_num-1 do begin
            IDlink_addr    = fpgabuf_map.dataIO_IDlink0    + sub_buf_num[i]*dsp_const.IO_vars_offset
            trig_mask_addr = fpgabuf_map.dataIO_trig_mask0 + sub_buf_num[i]*dsp_const.IO_vars_offset
            Vallink_addr   = fpgabuf_map.dataIO_Vallink0   + sub_buf_num[i]*dsp_const.IO_vars_offset
            decfact_addr   = fpgabuf_map.dataIO_decfact0   + sub_buf_num[i]*dsp_const.IO_vars_offset
            length_addr    = fpgabuf_map.dataIO_length0    + sub_buf_num[i]*dsp_const.IO_vars_offset
            curridx_addr   = fpgabuf_map.dataIO_curridx0   + sub_buf_num[i]*dsp_const.IO_vars_offset
            DMlink_addr    = fpgabuf_map.dataIO_DMlink0    + sub_buf_num[i]*dsp_const.IO_vars_offset
            SDRAMlink_addr = fpgabuf_map.dataIO_SDRAMlink0 + sub_buf_num[i]*dsp_const.IO_vars_offset

            if keyword_set(trig_enable) then begin
                IDlink = the_trig_dm_addr and '00FFFFFF'XUL
                IDlink = IDlink or ishft(1UL,30) or ishft(1UL,31)
                if dsp_id ne 0 then IDlink = IDlink or ishft(1UL,22)
                Vallink= the_trig_val
            endif else begin
                IDlink = 0UL
                Vallink= 0UL
            endelse
            trig_mask = 'FFFFFFFF'XUL
            decfact   = ulong(the_dec) or ishft(ulong(the_dec), 16)
            buflen = ulong(the_length) or ishft(ulong(the_length), 16)
            DMlink    = the_dm_addr[i] and '003FFFFF'XUL
            if dsp_id ne 0 then DMlink = DMlink or ishft(1UL,22)
            DMlink = DMlink or ishft(ulong(the_rec_size), 24)
            SDRAMlink = sdram_IO_addr(dsp_id, the_buf_num[i]) and '01FFFFFF'XUL
            curridx   = SDRAMlink
            SDRAMlink = SDRAMlink or ishft(1UL,30)*keyword_set(write)
            SDRAMlink = SDRAMlink or ishft(1UL,31)*keyword_set(circular)

            err = write_same_board(sub_dsp_board, IDlink_addr, IDlink, SET_PM=dsp_datasheet.fpgabuf_mem_id)
            if err ne adsec_error.ok then return, err
            err = write_same_board(sub_dsp_board, trig_mask_addr, trig_mask, SET_PM=dsp_datasheet.fpgabuf_mem_id)
            if err ne adsec_error.ok then return, err
            err = write_same_board(sub_dsp_board, Vallink_addr, Vallink, SET_PM=dsp_datasheet.fpgabuf_mem_id)
            if err ne adsec_error.ok then return, err
            err = write_same_board(sub_dsp_board, decfact_addr, decfact, SET_PM=dsp_datasheet.fpgabuf_mem_id)
            if err ne adsec_error.ok then return, err
            err = write_same_board(sub_dsp_board, length_addr, buflen, SET_PM=dsp_datasheet.fpgabuf_mem_id)
            if err ne adsec_error.ok then return, err
            err = write_same_board(sub_dsp_board, curridx_addr, curridx, SET_PM=dsp_datasheet.fpgabuf_mem_id)
            if err ne adsec_error.ok then return, err
            err = write_same_board(sub_dsp_board, DMlink_addr, DMlink, SET_PM=dsp_datasheet.fpgabuf_mem_id)
            if err ne adsec_error.ok then return, err
            err = write_same_board(sub_dsp_board, SDRAMlink_addr, SDRAMlink, SET_PM=dsp_datasheet.fpgabuf_mem_id)
            if err ne adsec_error.ok then return, err
        endfor
    endif
endfor

if keyword_set(buffer_enable) then begin
    err = enable_IO_buffer(the_dsp_list, the_buf_num)
    if err ne adsec_error.ok then return, err
endif

return, adsec_error.ok

end
