; $Id: get_ch_data_buf.pro,v 1.11 2006/11/14 11:06:19 labot Exp $

;+
;   NAME:
;    GET_CH_DATA_BUF
;
;   PURPOSE:
;    It retrieves the data from a channel-base address by using the buffers.
;
;   USAGE:
;    err=get_ch_data_buf(addr, buflen, buf, $
;           DECIMATION=dec, ULONG=set_ulong, LONG=set_long, TYPE=data_type)
;
;   INPUT:
;    addr:          address from which to retrieve data.
;    buflen:        length of buffered data.
;
;   OUTPUT:
;    err:           error code.
;    buf:           data acquired.
;
;   KEYWORD:
;    DECIMATION:    decimation parameter.
;    ULONG:         data read as ulong.
;    LONG:          data read as long.
;    DATA_TYPE:     data read as data_type.
;
; HISTORY
;
;   Written by G. Brusa and A. Riccardi (AR)
;   Osservatorio Asrofisico di Arcetri, ITALY
;   <riccardi@arcetri.astro.it>
;
;   13 Mar 2002, AR
;     Bug fixed in the CONT keyword setting of the MESSAGE call.
;
;   05 June 2004, AR
;     Rewritten to match the LBT specifications.
;   02 Nov 2004, MX
;     Adsec.all changed in sc.all.
;   01 Dec 2004, AR
;     DECIMATION keyword added
;   08 Feb 2005, MX
;    Help written.
;    Commented some rows.
;   14 Nov 2006, MX
;    Adjusted some casting from int o long
;-
function get_ch_data_buf, addr, buflen, buf, DECIMATION=dec, ULONG=set_ulong, LONG=set_long, TYPE=data_type
@adsec_common

if n_elements(dec) eq 0 then dec=0

naddr = n_elements(addr)
if (naddr gt 3) or (naddr lt 1)  then begin
    message, "Wrong number of addresses (1-3)",CONT=(sc.debug eq 0B)
    return,adsec_error.input_type
endif
;if total((addr lt dsp_const.first_ch_addr) or (addr gt dsp_const.last_ch_addr)) gt 0 then begin
;    message,"Addresses must be within the "+string(dsp_const.first_ch_addr,FORMAT="(Z8.8)")+"-" $
;        +string(dsp_const.last_ch_addr,FORMAT="(Z8.8)")+" range",CONT=(sc.debug eq 0B)
;    return,adsec_error.input_type
;endif
if buflen lt 1 or buflen gt dsp_const.MAX_DATA_IO_LEN then begin
    message,"Wrong buffer length (1-"+strtrim(dsp_const.MAX_DATA_IO_LEN,2),CONT=(sc.debug eq 0B)
    return,adsec_error.input_type
endif

buff2use = indgen(naddr)

err = set_IO_buffer(sc.all_dsp, buff2use, addr, buflen $
                   , RECORD_SIZE=adsec.n_act_per_dsp, DECIMATION=dec, /START_ENABLE)
if err ne adsec_error.ok then return, err

wait, (buflen*(dec+1)*adsec.sampling_time)*1.2 > 0.01

counter = 3
repeat begin
    err = check_working_io(sc.all_dsp, buff2use, are_working)
    if err ne adsec_error.ok then return, err
    counter -= 1
    wait, (buflen*long(dec+1)*adsec.sampling_time)*1.2 > 0.01
endrep until counter eq 0 or are_working eq 0
if are_working ne 0 then begin
    message, "Some buffer did not stop!", CONT=sc.debug eq 0B
    return, adsec_error.IDL_BUF_NOT_STOP
endif

err = read_IO_buffer(sc.all_dsp,  buff2use, long(buflen)*adsec.n_act_per_dsp, buf $
                     , ULONG=set_ulong, LONG=set_long, TYPE=data_type)
if err ne adsec_error.ok then return, err

buf = reform(buf, adsec.n_act_per_dsp, buflen, naddr, adsec.n_dsp, /OVER)
buf = transpose(temporary(buf), [0,3,1,2])
buf = reform(buf, adsec.n_act_per_dsp*adsec.n_dsp, buflen, naddr, /OVER)
buf = transpose(temporary(buf), [1,2,0])

return, adsec_error.ok
end
