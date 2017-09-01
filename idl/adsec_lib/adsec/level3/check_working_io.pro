;$Id: check_working_io.pro,v 1.3 2004/11/02 15:00:22 marco Exp $
;+
; HISTORY
;   written by A. Riccardi (AR)
;
;   13 Aug 2004, AR
;     process status is read only from dsp_list (before was read from all DSPs)
;   02 Nov 2004, MX
;     Adsec.all changed in sc.all.
;-
function check_working_IO, dsp_list, buf_num, are_working, WORKING_IO_MAP=working_io_mask

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

process_mask = make_IO_process_mask(the_dsp_list, the_buf_num)

dsp_board_list = the_dsp_list/adsec.n_dsp_per_board
uniq_dsp_board_list = dsp_board_list(uniq(dsp_board_list,sort(dsp_board_list)))
err = read_seq_board(uniq_dsp_board_list, fpgabuf_map.dataIO_process, 1L, sub_dataIO_process $
                    , SET_PM=dsp_datasheet.fpgabuf_mem_id, /ULONG)
if err ne adsec_error.ok then return, err
dataIO_process = ulonarr(adsec.n_bcu*adsec.n_board_per_bcu)
dataIO_process[uniq_dsp_board_list] = sub_dataIO_process

working_io_mask = process_mask and dataIO_process
are_working = (total(working_io_mask, /DOUBLE) ne 0) && (sc.host_comm ne "Dummy")

return, adsec_error.ok
end
