;$Id: make_io_process_mask.pro,v 1.2 2004/11/02 16:26:40 marco Exp $
;+
; HISTORY
;  13 July 2004 written by A. Riccardi
;  02 Nov 2004, MX
;    Adsec.all changed in sc.all.
;-

function make_IO_process_mask, dsp_list, buf_num

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
process_mask = ulonarr(adsec.n_bcu*adsec.n_board_per_bcu)

for dsp_id = 0,adsec.n_dsp_per_board-1 do begin
    idx=where(dsp_onboard_id eq dsp_id, count)
    if count gt 0 then begin
        sub_dsp_board = dsp_board_list[idx]
        sub_buf_num = the_buf_num+dsp_id*(dsp_const.n_dataIO/adsec.n_dsp_per_board)
        process_mask[sub_dsp_board] = process_mask[sub_dsp_board] or ulong(total(3UL*2UL^(2*sub_buf_num), /DOUBLE))
    endif
endfor

return, process_mask
end
