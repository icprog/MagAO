;Procedure to clean the delay lines parameters
;+
;   NAME:
;    CLEAR_DL
;
;   PURPOSE:
;       Clear the dsp memory dedicated to the delay lines parameters.
;
;   USAGE:
;    err = clear_dl()
;
;   INPUT:
;    None.
;   OUTPUT:
;    Error code.
;
;   KEYWORD:
;    None.
;
;   HISTORY
;    Created on April 2005 by Daniela Zanotti (DZ).
;    20 May 2005 Marco Xompero (MX)
;       Dsp_map updated to rtr_map. Help written
;    29 Nov 2005 MX
;       New and more complete clean added. MX
;    23 January MX
;       Added the keyword verbose.
;   07 Feb 2007 MX
;       LUT of variable deleted and name refurbished.
;   NOTE:
;    None.
;
;
Function  clear_dl, VERBOSE=verbose

    @adsec_common

    if keyword_set(verbose) then print, format='("Cleaning modes...",$)'
    err = write_same_dsp(sc.all, dsp_map.modes, fltarr(adsec.n_act_per_dsp))
    if err ne adsec_error.ok then begin
        message, "Cleaning error.", CONT=(sc.debug eq 0B)
        return, adsec_error.communication
    endif
    if keyword_set(verbose) then print, 'done' 

    if keyword_set(verbose) then print, format='("Cleaning modes_vector...",$)'
    err = write_same_dsp(sc.all, rtr_map.modes_vector, fltarr(adsec.n_actuators))
    if err ne adsec_error.ok then begin
        message, "Cleaning error.", CONT=(sc.debug eq 0B)
        return, adsec_error.communication
    endif
    if keyword_set(verbose) then print, 'done'

    if keyword_set(verbose) then print, format='("Cleaning modes_vector_delay (modes_delay)...",$)'
    err = write_same_dsp(sc.all, rtr_map.modes_delay, fltarr(adsec.n_actuators*rtr.n_modes_delay))
    if err ne adsec_error.ok then begin
        message, "Cleaning error.", CONT=(sc.debug eq 0B)
        return, adsec_error.communication
    endif
    if keyword_set(verbose) then print, 'done'

    if keyword_set(verbose) then print, format='("Cleaning slope_vector...",$)'
    err = write_same_dsp(sc.all, rtr_map.slope_vector, fltarr(rtr.n_slope))
    if err ne adsec_error.ok then begin
        message, "Cleaning error.", CONT=(sc.debug eq 0B)
        return, adsec_error.communication
    endif
    if keyword_set(verbose) then print, 'done' 

    if keyword_set(verbose) then print, format='("Cleaning slope_delay...",$)'
    err = write_same_dsp(sc.all, rtr_map.slope_delay, fltarr(rtr.n_slope*rtr.n_slope_delay))
    if err ne adsec_error.ok then begin
        message, "Cleaning error.", CONT=(sc.debug eq 0B)
        return, adsec_error.communication
    endif
    if keyword_set(verbose) then print, 'done'

    if keyword_set(verbose) then print, format='("Cleaning precalculated mode filter...",$)'
     err = write_same_dsp(sc.all, dsp_map.precalc_modes, fltarr(adsec.n_act_per_dsp))
    if err ne adsec_error.ok then begin
        message, "Cleaning error.", CONT=(sc.debug eq 0B)
        return, adsec_error.communication
    endif
    if keyword_set(verbose) then print, 'done'

    if keyword_set(verbose) then print, format='("Cleaning precalculated slope filter...",$)'
    err = write_same_dsp(sc.all, dsp_map.precalc_slope, fltarr(adsec.n_act_per_dsp))
    if err ne adsec_error.ok then begin
        message, "Cleaning error.", CONT=(sc.debug eq 0B)
        return, adsec_error.communication
    endif
    if keyword_set(verbose) then print, 'done'

;    print, 'Cleaning all SDRAM...'
;    err = write_same_board(sc.all, 0l, fltarr(dsp_datasheet.sdram_mem_size), SET_PM=2)
;    if err ne adsec_error.ok then begin
;        message, "Cleaning error.", CONT=(sc.debug eq 0B)
;        return, adsec_error.communication
;    endif
;    print, '... done.'


    return, adsec_error.ok


;print, 'Cleaning delay line slope counter'
;err = write_same_dsp(sc.all, '180181'xl, dsp_map.slope_delay)
;if err ne adsec_error.ok then begin
;    message, "Cleaning error.", CONT=(sc.debug eq 0B)
;    return, adsec_error.communication
;endif
;print, '... done.' 

;print, 'Cleaning delay line modes counter'
;err = write_same_dsp(sc.all, '180182'xl, dsp_map.modes_delay)
;if err ne adsec_error.ok then begin
;    message, "Cleaning error.", CONT=(sc.debug eq 0B)
;    return, adsec_error.communication
;endif
;print, '... done.' 

End
