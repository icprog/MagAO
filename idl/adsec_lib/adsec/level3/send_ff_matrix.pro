; $Id: send_ff_matrix.pro,v 1.13 2009/04/10 13:34:50 labot Exp $
;+
;   NAME:
;    SEND_FF_MATRIX
;
;   PURPOSE:
;    Write FF matrix into the DSP memory by splittng the matrix in adsec.n_dsp part of size
;    adsec.n_act_per_dsp X adsec.n_actuators
;
;   USAGE:
;    err = send_ff_matrix(ff_matrix)
;
;   INPUT:
;    ff_matrix: feed-foeward matrix to send.
;
;   OUTPUT:
;    Error code.
;
;   KEYWORD:
;    None.
;
;   NOTE:
;    None.
;   
; HISTORY
;   28 May 2004 Armando Riccardi (AR)
;       Completely rewritten to match the LBT format.
;   22 Jul 2004 Marco Xompero (MX)
;       Checked for the exact memory dsp writing.
;   02 Nov 2004, MX
;       Adsec.all changed in sc.all.
;   20 May 2005 Marco Xompero
;       Dsp_map variables updated to rtr_map.
;   07 Feb 2007 MX
;       LUT of variable deleted and name refurbished.
;   20 Feb 2007 MX
;       FF matrix update in RTR structure added.
;   09 Apr 2009 MX
;       Added fits file support and RTDB update
;-

function send_ff_matrix, ff_matrix, FITSFILE=fitsfile

    @adsec_common

    if n_elements(fitsfile) gt 0 then ff_matrix = readfits(ff_matrix)

    dim = size(ff_matrix, /DIM)
    if dim[0] ne adsec.n_actuators or dim[1] ne adsec.n_actuators then begin
        message, "Wrong FF matrix to send. Nothig done.", CONT = (sc.debug eq 0)
        return, adsec_error.input_type
    endif
    ;adsec.ff_matrix = ff_matrix    

    kk = reform(float(ff_matrix), adsec.n_actuators, adsec.n_act_per_dsp, adsec.n_dsp)
    kk = transpose(temporary(kk), [1,0,2])
    kk = reform(temporary(kk), adsec.n_act_per_dsp*adsec.n_actuators, adsec.n_dsp)

    ; WARNING! if board_number doesn't match the sequence 0,4,8,...then kk has to
    ; be reordered before uploding it to DSP memory.
    ;
    err = write_seq_dsp(sc.all, rtr_map.matrix_k, kk, /CHECK)
    if err ne adsec_error.ok then return, erro


    if n_elements(fitsfile) gt 0 then !AO_STATUS.ff_matrix = fitsfile else !AO_STATUS.ff_matrix = ""
    return, update_rtdb()

end
