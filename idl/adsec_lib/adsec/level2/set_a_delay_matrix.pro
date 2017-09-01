;+
;   NAME:
;    SET_A_DELAY_MATRIX
;
;   PURPOSE:;    Write A_DELAY matrix into the DSP memory by splittng the matrix in adsec.n_dsp part of size
;    adsec.n_act_per_dsp X adsec.n_actuators X rtr.n_modes_delay
;
;   USAGE:
;    err = send_A_delay_matrix(a_delay_matrix)
;
;   INPUT:
;    A_delay_matrix: A_delay matrix .
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
;   5 Feb 2005 Daniela Zanotti(DZ)
;   20 May 2005 Marco Xompero (MX)
;       Dsp map updated to rtr map. Minor changes in passed parameters.
;   29 Aug 2005, AR
;       rtr.n_modes_delay used instead of the wrong rtr.n_slope_delay.
;   07 Feb 2007 MX
;       LUT of variable deleted and name refurbished.
;   20 Feb 2007 MX
;       A delay matrix now tested and RTR structure updated.
;   09 Apr 2009 MX
;       Modified to accept fits file and to update MSGD-RTDB.
;-

function set_a_delay_matrix, a_delay_tmp, FITSFILE=fitsfile

    @adsec_common
   ; if n_elements(fitsfile) gt 0 then  a_delay_in = readfits(fitsfile)  else a_delay_in = a_delay_tmp

    a_delay_in = a_delay_tmp
    dim = size(a_delay_in, /DIM)
    if n_elements(dim) eq 2 then dim = [dim, 1]
    if (dim[0] ne adsec.n_actuators) or (dim[1] ne adsec.n_actuators) or (dim[2] ne rtr.n_modes_delay) then begin
        message, "Wrong A Delay Matrix to send. Nothing done.", CONT = (sc.debug eq 0)
        return, adsec_error.input_type
    endif
    a_delay = reverse(float(a_delay_in),1)
    a_delay=reform(temporary(a_delay), adsec.n_actuators,adsec.n_act_per_dsp,adsec.n_dsp,rtr.n_modes_delay)
    a_delay=transpose(temporary(a_delay), [1,0,3,2])
    a_delay=reform(temporary(a_delay), adsec.n_act_per_dsp*adsec.n_actuators*rtr.n_modes_delay,adsec.n_dsp)
    ; be reordered before uploding it to DSP memory.
    err = write_seq_dsp(sc.all, rtr_map.matrix_a_delay, a_delay, /CHECK)
    if err ne adsec_error.ok then return, err
    *rtr.a_delay_matrix = a_delay_in

    
    if n_elements(fitsfile) eq 0 then !AO_STATUS.a_delay = "" else !AO_STATUS.a_delay = fitsfile

    return, update_rtdb()

end
