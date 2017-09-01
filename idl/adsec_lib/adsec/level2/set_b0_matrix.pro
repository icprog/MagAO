;+
;   NAME:
;    SET_B0_MATRIX
;
;   PURPOSE:
;    Write B0 matrix into the DSP memory by splittng the matrix in adsec.n_dsp part of size
;    adsec.n_act_per_dsp X rtr.n_slope
;
;   USAGE:
;    err = send_b0_matrix(b0_matrix, /block)
;
;   INPUT:
;    b0_matrix: B0 matrix gain to send.
;
;   OUTPUT:
;    Error code.
;
;   KEYWORD:
;    Block : if set the B0_matrix is written in the block memory B.
;
;   NOTE:
;    None.
;   
; HISTORY
;   Created by 7 Feb 2005 Daniela Zanotti(DZ)
;   20 May 2005 Marco Xompero (MX)
;       Dsp_map updated to rtr_map. Minor changing in parameters passed.
;   07 Feb 2007 MX
;       LUT of variable deleted and name refurbished.
;   20 Feb 2007 MX
;       RTR structure updated after sending.
;   09 Apr 2009 MX
;       Modified to accept fits file and to update MSGD-RTDB.
;-

function set_b0_matrix, b0_matrix_in, BLOCK = block, FITSFILE=fitsfile

    @adsec_common

;    if n_elements(fitsfile) eq 0 then b0m = b0_matrix_in else b0m = readfits(fitsfile)
    b0m = b0_matrix_in
    dim = size(b0_matrix_in, /DIM)
    if (dim[0] ne rtr.n_slope) or (dim[1] ne adsec.n_actuators) then begin
        message, "Wrong B0 Matrix to upload. Nothig done.", CONT = (sc.debug eq 0)
        return, adsec_error.input_type
    endif

    b0 = reform(float(b0m), rtr.n_slope, adsec.n_act_per_dsp, adsec.n_dsp)
    b0 = transpose(temporary(b0), [1,0,2])
    b0 = reform(temporary(b0), adsec.n_act_per_dsp*rtr.n_slope, adsec.n_dsp)


    ; WARNING! if board_number doesn't match the sequence 0,4,8,...then kk has to
    ; be reordered before uploding it to DSP memory.
    ;
    ;if not(keyword_set(block)) then $
    ;    err = write_seq_dsp(sc.all, dsp_map.matrix_b0a, b0, /CHECK) else $
    ;    err = write_seq_dsp(sc.all, dsp_map.matrix_b0b, b0, /CHECK)

    if ~(keyword_set(block)) then begin
        err = write_seq_dsp(sc.all, rtr_map.matrix_b0a, b0, /CHECK) 
        *rtr.b0_a_matrix = b0m
        if err ne adsec_error.ok then return, err
        if n_elements(fitsfile) eq 0 then !AO_STATUS.b0_a = "" else !AO_STATUS.b0_a = fitsfile    
    endif else begin
        err = write_seq_dsp(sc.all, rtr_map.matrix_b0b, b0, /CHECK)
        *rtr.b0_b_matrix = b0m
        if err ne adsec_error.ok then return, err
        if n_elements(fitsfile) eq 0 then !AO_STATUS.b0_b = "" else !AO_STATUS.b0_b = fitsfile    
    endelse

    return, update_rtdb()

end
