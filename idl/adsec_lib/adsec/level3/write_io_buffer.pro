; $Id: write_io_buffer.pro,v 1.4 2005/03/31 13:12:51 labot Exp $

;+
;
; NAME
;
;   WRITE_IO_BUFFER
;
;   writes in SDRAM the contents of the specified buffers related to
;   the dsp list. This function is used to zero the SDRAM memory or
;   fill it with desired values for write_to_mem buffer oprations. 
;
;   err = write_IO_buffer(dsp_list, buf_num_list, buf)
;
; INPUTS
;
;   dsp_list:    byte vector. DSP list. sc.all_dsp is allowed.
;
;   buf_num_list: byte/short/long vector. List of buffer to read from.
;
;   buf:        Float/long/ulong vector[buflen,M,N], where
;               M is the number of buffers and N is the number of
;               DSPs. If buf is vector[buflen,M] then same buffers are
;               written on all specified DSPs. If buf is
;               vector[buflen] then same buffers are written on all
;               the specified DSPs and IO buffers.
;
; OUTPUTS
;
;   err:         long scalar. see adsec_error structure defined in init_gvar.pro
;
; EXAMPLES
;
;   None.
;
; HISTORY
;   06 Aug 2004: written by A. Riccardi (AR)
;
;   10 Aug 2004: AR
;     buflen input parameter removed. That info can be extracted from
;     buf parameter.
;     Tests on input buf implemented.
;     buf[buflen, n_buf] -> write same buffers on all the specified DSPs
;     buf[buflen] -> write same buffers on all the specified DSPs and
;                    IO buffers
;   02 Nov 2004, MX
;     Adsec.all changed in sc.all.
;   March 2005
;     Added CHECK keyword.
;-
;
function write_IO_buffer, dsp_list, buf_num, buf, CHECK=check

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

dsp_onboard_id = the_dsp_list mod adsec.n_dsp_per_board
dsp_board_list = the_dsp_list/adsec.n_dsp_per_board
n_buf_num = n_elements(the_buf_num)
n_dsp_list = n_elements(the_dsp_list)

dim = size(buf, /DIM)
case n_elements(dim) of
    1: begin
        do_write_same_dsp = 1B
        do_write_same_buf = 1B
    end
    
    2: begin
        if dim[1] ne n_buf_num then begin
            message, '2-D buf must have buf[buflen,n_buffer] size.', CONT=sc.debug eq 0B
            return, adsec_error.input_type
        endif
        do_write_same_dsp = 1B
        do_write_same_buf = 0B
    end

    3: begin
        if dim[2] ne n_dsp_list then begin
            message, '3-D buf must have buf[buflen,n_buffer,n_dsp] size.' $
            , CONT=sc.debug eq 0B
            return, adsec_error.input_type
        endif
        if dim[1] ne n_buf_num then begin
            message, '2-D buf must have buf[buflen,n_buffer] size.', CONT=sc.debug eq 0B
            return, adsec_error.input_type
        endif
        do_write_same_dsp = 0B
        do_write_same_buf = 0B
    end

    else: begin
        message, "Unexpected number of dimensions for buf", CONT=sc.debug eq 0B
        return, adsec_error.input_type
    end
end

err = check_working_IO(the_dsp_list, the_buf_num, are_working, WORKING_IO_MAP=working_io_map)
if err ne adsec_error.ok then return, err
if are_working then begin
    message, "WARNING! Some of the specified buffers are currently in use.", /INFO
endif

for dsp_id = 0,adsec.n_dsp_per_board-1 do begin
    idx=where(dsp_onboard_id eq dsp_id, count)
    if count gt 0 then begin
        sub_dsp_board = dsp_board_list[idx]

        for i=0,n_buf_num-1 do begin
            sdram_buf_addr = sdram_IO_addr(dsp_id, the_buf_num[i])
            if do_write_same_dsp then begin
                if do_write_same_buf then begin
                    tmp_buf = buf
                endif else begin
                    tmp_buf = buf[*,i]
                endelse
            endif else begin
                tmp_buf = buf[*,i,sub_dsp_board*adsec.n_dsp_per_board+dsp_id]
            endelse
            err = write_same_board(sub_dsp_board, sdram_buf_addr $
                                   , tmp_buf, SET_PM=dsp_datasheet.sdram_mem_id, CHECK=check)
            if err ne adsec_error.ok then return, err
        endfor
    endif
endfor

return,adsec_error.ok

end
