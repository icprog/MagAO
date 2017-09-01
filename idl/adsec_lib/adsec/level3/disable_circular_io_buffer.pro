; $Id: disable_circular_io_buffer.pro,v 1.2 2004/11/02 15:12:25 marco Exp $

;+
;
; NAME
;
;   DISABLE_CIRCULAR_IO_BUFFER
;
;   Disable the circularity of the specified buffers in the dsp list. The buffer that are not
;   specified are left unchanged.
;
;   err = disable_circular_IO_buffer(dsp_list, buf_num_list)
;
; INPUTS
;
;   dsp_list:    byte vector. DSP list. sc.all_dsp is allowed.
;                *** BE CAREFULL *** if dsp_list is not a list
;                of sequential DSP numbers, the disabling of circular
;                is not syncronous for all the DSPs in the list. Moreover
;                the syncronization is not possible if more then 1 buffer
;                is specified.
;
;   buf_num_list: byte/short/long vector. List of buffer to disable.
;
; OUTPUTS
;
;   err:         long scalar. see adsec_error structure defined in init_gvar.pro
;
;
;
; HISTORY
;
;   Written by A. Riccardi (AR)
;   Osservatorio Asrofisico di Arcetri, ITALY
;   <riccardi@arcetri.astro.it>
;
;   13 Mar 2002, AR
;     Bug fixed in the CONT keyword setting of the MESSAGE call.
;   02 Nov 2004, MX
;     Adsec.all changed in sc.all.
;-
;
function disable_circular_IO_buffer, dsp_list,  buf_num, WAIT_STOP=wait_stop

@adsec_common

if test_type(buf_num, /NOFLOAT, N_EL=n_buf_num) then begin
    message, "buf_num must be integer.", CONT= (sc.debug eq 0B)
    return, adsec_error.input_type
endif
if n_elements(uniq(buf_num, sort(buf_num))) ne n_buf_num then begin
    message, "there are repeated buffer numbers in buf_num.", CONT= (sc.debug eq 0B)
    return, adsec_error.input_type
endif
if total((buf_num lt 0) + (buf_num gt dsp_const.n_dataIO-1)) ne 0 then begin
    message, "buf_num contents buffer numbers out of range.", CONT=(sc.debug eq 0B)
    return, adsec_error.buff_not_valid
endif

the_buf_num = fix(buf_num)

for i=0,n_buf_num-1 do begin
    addr = dsp_map.dataIO_circul0+the_buf_num[i]*dsp_const.IO_vars_offset
    err = write_same_dsp(dsp_list, addr, 0)
    if err ne adsec_error.ok then return, err
endfor

if keyword_set(wait_stop) then begin
    max_count = 4
    count = 0
    ;; polling
    repeat begin
        time2wait = 0.0
        for i=0,n_buf_num-1 do begin
            addr = dsp_map.dataIO_length0 $
                   + the_buf_num[i]*dsp_const.IO_vars_offset
            err = read_seq_dsp(dsp_list, addr, 1, length)
            if err ne adsec_error.ok then return, err

            addr = dsp_map.dataIO_currdfc0 $
                   + the_buf_num[i]*dsp_const.IO_vars_offset
            err = read_seq_dsp(dsp_list, addr, 1, dec)

            if err ne adsec_error.ok then return, err
            addr = dsp_map.dataIO_curridx0 $
                   + the_buf_num[i]*dsp_const.IO_vars_offset
            err = read_seq_dsp(dsp_list, addr, 1, idx)
            if err ne adsec_error.ok then return, err

            count = count + 1
            ;; compute the max time (in unit of adsec.sampling_time)
            ;; to wait for stop all the given buff.
            time2wait = max((length-idx)*(dec+1.0)) > time2wait
        endfor
        wait, (time2wait*1.1*adsec.sampling_time) > 0.01
    endrep until (time2wait eq 0.0) or (count ge max_count)

    ;; test if exit the polling because of timeout
    if time2wait ne 0.0 then begin
        message, "Timeout: the buffers do not stop", CONT=(sc.debug eq 0B)
        return, adsec_error.buffer_timeout
    endif
endif

return, adsec_error.ok

end
