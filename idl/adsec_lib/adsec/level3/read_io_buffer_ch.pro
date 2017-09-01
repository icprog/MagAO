; $Id: read_io_buffer_ch.pro,v 1.3 2005/08/03 16:56:54 labot Exp $
;
;+
;    READ_IO_BUFFER_CH
;
;    read a buffer supposing that actuator-based data have been saved.
;    (buffers with n_samp records with adsec.n_act_per_dsp record size)
;
;    err = read_io_buffer_ch(dsp_list, n_buf_list, n_samp, buf, $
;                            [/ULONG|/LONG|TYPE=type])
;
;    dsp_list, n_buf_list and KEYWORDS same as read_io_buffer.
;
;    N = n_elements(n_buf_list)
;    n_dsp_list = n_elements(dsp_list)
;    buf = buf[n_samp, N, n_dsp_list*adsec.n_act_per_dsp]
; HISTORY
;   Casted to long the n_samp.
;   Bug on one single dsp removed.
;-

function read_IO_buffer_ch, dsp_list, n_buf_list, n_samp, buf, ULONG=ulong, LONG=long, TYPE=type

@adsec_common

err=read_IO_buffer(dsp_list, n_buf_list, long(n_samp)*adsec.n_act_per_dsp, buf $
                   , ULONG=ulong, LONG=long, TYPE=type)
if err ne adsec_error.ok then return, err

s = size(buf, /DIM)
if n_elements(s) gt 1 then begin
    nn = s[1]
    n_dsp = s[2]
    buf = reform(buf, adsec.n_act_per_dsp, n_samp, nn, n_dsp, /OVER)
    buf = transpose(temporary(buf), [1,2,0,3])
    buf = reform(buf, n_samp, nn, adsec.n_act_per_dsp*n_dsp, /OVER)
endif else begin

    n_dsp = 1
    buf = reform(buf, adsec.n_act_per_dsp, n_samp, n_dsp, /OVER)
    buf = transpose(temporary(buf), [1,0,2])
    buf = reform(buf, n_samp,adsec.n_act_per_dsp*n_dsp, /OVER)

endelse

return, adsec_error.ok
end
