; $Id: disable_io_buffer.pro,v 1.2 2004/11/02 15:14:26 marco Exp $

;+
;
; NAME
;
;   DISABLE_IO_BUFFER
;
;   Disable the start of the specified buffers in the dsp list. The buffer that are not
;   specified are left unchanged.
;   The corresponding dataIO_start flags remain 1 until a trigger event occures. After
;   that, the flags are set to 0 and the buffers start to run. If there is not a trigger
;   enabled the flags are set to 0 and starts immediately.
;   The dataIO_process flags are set to 1 only when the buffers are running
;   (and so when the dataIO_start flags flip from 1 to 0).
;
;   err = disable_IO_buffer(dsp_list, buf_num_list, IO0=io0, IO1=io1, IO2=io2, $
;                                    IO3=io3, IO4=io4, IO5=io5)
;
; INPUTS
;
;   dsp_list:    byte vector. DSP list. sc.all_dsp is allowed.
;                *** BE CAREFULL *** if dsp_list is not a list
;                of sequential DSP numbers, the start disabling
;                is not syncronous for all the DSPs in the list.
;
;   buf_num_list: byte/short/long vector. List of buffer to disable.
;
; OUTPUTS
;
;   err:         long scalar. see adsec_error structure defined in init_gvar.pro
;
; KEYWORDS
;
;   IO0,IO1,IO2,IO3,IO4,IO5:   if set, the corresponding data IO buffer is disabled.
;
; EXAMPLES
;
;   The user can disable the IO buffer using the buf_num_list and/or the IOn keywords.
;   The disabled buffers are those specified in the buf_num_list OR by the IOn keywords.
;
;   err = disable_IO_buffer(sc.all_dsp, [2,5])
;   disables the buffers #2 and #5 in all the DSPs. The previous command line is equivalent to
;   err = disable_IO_buffer(sc.all_dsp, 2, /IO5)
;   or
;   err = disable_IO_buffer(sc.all_dsp, /IO5, /IO2)
;
; HISTORY
;
;   Written by A. Riccardi (AR)
;   Osservatorio Asrofisico di Arcetri, ITALY
;   <riccardi@arcetri.astro.it>
;
;   09 Jan 2004, Marco Xompero
;   Explaining of dataIO_start and dataIO_process behaviour in the help.
;   02 Nov 2004, MX
;     Adsec.all changed in sc.all.
;
;-
;
function disable_IO_buffer, dsp_list,  buf_num

@adsec_common

stop_flags = make_IO_process_mask(dsp_list, buf_num)

err = write_seq_board(sc.all, fpgabuf_map.dataIO_start, stop_flags $
                     , SET_PM=dsp_datasheet.fpgabuf_mem_id)
return, err

end
