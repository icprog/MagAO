; $Id: init_cl_buffer.pro,v 1.2 2007/02/07 17:21:41 marco Exp $
;
;+
;  NAME:
;   INIT_CL_BUFFER
;
;  PURPOSE:
;   Load the structures for the closed loop diagnostic data buffer.
;
;  USAGE:
;   err = INIT_CL_BUFFER(LENGHT=lenght)
;
;  INPUT:
;
;  OUTPUT:
;   error               : error code.
;
;  COMMON BLOCKS:
;       cbuf_clin,cbuf_clout, cbuf_template.
;
;  PACKAGE:
;   LBT Adsec Libraries.
;
;  HISTORY
;   Created on March 2006 by Marco Xompero (MX).
;   marco@arcetri.astro.it
;   07 Feb 2007 MX
;       LUT of variable deleted and name refurbished.
;-
Pro init_cl_buffer, LENGTH=length

    @adsec_common
    if n_elements(length) eq 0 then length=3
    template = {                                    $
        bias_command  : fltarr(adsec.n_actuators),  $
        g_gain        : fltarr(adsec.n_actuators),  $
        block         : 0,                          $
        wfs_freq      : 0.0,                        $
        wfs_frames_counter: ulonarr(adsec.n_dsp_per_board * adsec.n_bcu*adsec.n_board_per_bcu),                        $
        wr_ptr        : ulonarr(adsec.n_board_per_bcu * adsec.n_bcu)                         $
    }
    
    cbuf_clin  = make_circ_buf(template, length)
    cbuf_clout = make_circ_buf(template, length)
    cbuf_template = template

End
