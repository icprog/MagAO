; $Id: enable_io_buffer.pro,v 1.5 2004/11/02 15:19:44 marco Exp $

;+
;
; NAME
;
;   ENABLE_IO_BUFFER
;
;   err = enable_IO_buffer(dsp_list, buf_num_list)
;
; INPUTS
;
;   dsp_list:    byte vector. DSP list. sc.all_dsp is allowed.
;
;   buf_num_list: byte/short/long vector. List of buffer to enable.
;
; OUTPUTS
;
;   err:         long scalar. see adsec_error structure defined in init_gvar.pro
;
; EXAMPLES
;
;   err = enable_IO_buffer(sc.all_dsp, [2,5])
;   enables the buffers #2 and #5 in all the DSPs.
;
; HISTORY
;
;   Written by A. Riccardi (AR)
;   Osservatorio Asrofisico di Arcetri, ITALY
;   <riccardi@arcetri.astro.it>
;
;   09 Jan 2004, Marco Xompero
;   Explaining of dataIO_start and dataIO_process behaviour in the help.
;
;   05 May 2004, AR
;   removed IOn keywords.
;   added TRIG_ENABLE keyword
;   rewritten to match LBT buffer definitions
;
;   21 Jul 2004, AR
;   Removed redundant keyword TRIG_ENABLE. Trigger is configured only using set_IO_buffer
;
;   10 Aug 2004, AR
;   Obsolete test on TRIG_ENABLE removed.
;   02 Nov 2004, MX
;     Adsec.all changed in sc.all.
;-
;
function enable_IO_buffer, dsp_list,  buf_num

@adsec_common

err = check_working_IO(dsp_list, buf_num, are_working, WORKING_IO_MAP=working_io_map)
if err ne adsec_error.ok then return, err
if are_working then begin
    message, "Some of the specified buffers are currently in use.", CONT=sc.debug eq 0B
    return, adsec_error.IDL_BUF_NOT_STOP
endif

dataIO_start = make_IO_process_mask(dsp_list, buf_num) $
               and (dsp_const.io_startontrig_mask)

err = write_seq_board(sc.all, fpgabuf_map.dataIO_start, dataIO_start $
                     , SET_PM=dsp_datasheet.fpgabuf_mem_id)
return, err

end
