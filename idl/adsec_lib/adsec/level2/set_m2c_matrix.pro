;+
;   NAME:
;    SET_M2C_MATRIX
;
;   PURPOSE:
;    Write M2C matrix into the DSP memory by splittng the matrix in adsec.n_dsp part of size
;    adsec.n_act_per_dsp X adsec.n_actuators
;
;   USAGE:
;    err = send_m2c_matrix(m2c_matrix)
;
;   INPUT:
;    m2c_matrix: m2c matrix gain to send.
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
;   7 Feb 2005  Daniela Zanotti(DZ)
;   20 May 2005 Marco Xompero
;      Dsp_map updated to rtr_map. Minor changes in passed parameters.
;   20 Feb 2005 MX
;        RTR structure now updated at the end of operations
;   3 April 3007 MX
;        Bug in the reform matrix fixed, n_modes2correct instead of n_actuators.
;   09 Apr 2009 MX
;       Modified to accept fits file and to update MSGD-RTDB.
;-

function set_m2c_matrix, m2c_matrix, FITSFILE=fitsfile

    @adsec_common
    ;if n_elements(fitsfile) gt 0 then m2c_matrix = fitsfile

    dim = size(m2c_matrix, /DIM)
    if (dim[0] ne rtr.n_modes2correct) or (dim[1] ne adsec.n_actuators) then begin
        message, "Wrong M2C Matrix to upload. Nothig done.", CONT = (sc.debug eq 0)
        return, adsec_error.input_type
    endif

    m2c = m2c_matrix
    m2c= reform(float(m2c), rtr.n_modes2correct, adsec.n_act_per_dsp, adsec.n_dsp)
    m2c= transpose(temporary(m2c), [1,0,2])
    m2c= reform(temporary(m2c), adsec.n_act_per_dsp*rtr.n_modes2correct, adsec.n_dsp)
    ; WARNING! if board_number doesn't match the sequence 0,4,8,...then kk has to
    ; be reordered before uploding it to DSP memory.
    ;

    err = write_seq_dsp(sc.all, rtr_map.matrix_m2c, m2c, /CHECK)
    *rtr.m2c_matrix = m2c_matrix

    if n_elements(fitsfile) gt 0 then !AO_STATUS.m2c = fitsfile else !AO_STATUS.m2c = ""

    return, err

end
