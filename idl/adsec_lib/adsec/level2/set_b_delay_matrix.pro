;+
;   NAME:
;    SET_B_DELAY_MATRIX
;
;   PURPOSE:;    Write B_DELAY matrix into the DSP memory by splittng the matrix in adsec.n_dsp
;                part of size adsec.n_act_per_dsp X rtr.n_slope X rtr.n_slope_delay
;
;   USAGE:
;    err = send_b_delay_matrix(b_delay_matrix, /block)
;
;   INPUT:
;    b_delay_matrix: B_delay matrix .
;
;   OUTPUT:
;    Error code.
;
;   KEYWORD:
;    BLOCK = if set the B_delay is written on block B.
;
;   NOTE:
;    None.
;
; HISTORY
;   5 Feb 2005 Daniela Zanotti(DZ)
;   20 May 2005 Marco Xompero (MX)
;    Dsp_map updated to rtr_map. Minor changing in parameters passed.
;   29 Aug 2005 Armando Riccardi (AR)
;    Long casting added into the code for a wrap around problem.
;   07 Oct 2005, MX
;     RTR matrices now managed by pointers.
;   09 Apr 2009 MX
;       Modified to accept fits file and to update MSGD-RTDB.
;-

function set_b_delay_matrix, b_delay_tmp, BLOCK = block , FITSFILE=fitsfile

    @adsec_common

;    if n_elements(fitsfile) gt 0 then b_delay_in = readfits(fitsfile) else b_delay_in = b_delay_tmp 
    b_delay_in = b_delay_tmp 
    dim = size(b_delay_in, /DIM)
    if n_elements(dim) eq 2 then dim = [dim, 1]
    if (dim[0] ne rtr.n_slope) or (dim[1] ne adsec.n_actuators) or (dim[2] ne rtr.n_slope_delay) then begin
        message, "Wrong Slope Delay Matrix to upload. Nothig done.", CONT = (sc.debug eq 0)
        return, adsec_error.input_type
    endif

    b_delay = reverse(float(b_delay_in),1)
    b_delay=reform(temporary(b_delay), rtr.n_slope,adsec.n_act_per_dsp,adsec.n_dsp,rtr.n_slope_delay)
    b_delay=transpose(temporary(b_delay), [1,0,3,2])
    b_delay=reform(temporary(b_delay), long(adsec.n_act_per_dsp)*rtr.n_slope*rtr.n_slope_delay,adsec.n_dsp)

    if not(keyword_set(block)) then begin
        err = write_seq_dsp(sc.all, rtr_map.matrix_ba_delay, b_delay, /CHECK) 
        if err ne adsec_error.ok then return, err
        *rtr.b_delay_a_matrix = b_delay_in
        if n_elements(fitsfile) eq 0 then !AO_STATUS.b_delay_a = "" else !AO_STATUS.b_delay_a = fitsfile

    endif else begin
        err = write_seq_dsp(sc.all, rtr_map.matrix_bb_delay, b_delay, /CHECK)
        if err ne adsec_error.ok then return, err
        *rtr.b_delay_b_matrix = b_delay_in
        if n_elements(fitsfile) eq 0 then !AO_STATUS.b_delay_b = "" else !AO_STATUS.b_delay_b = fitsfile

    endelse

    return, update_rtdb()

end
