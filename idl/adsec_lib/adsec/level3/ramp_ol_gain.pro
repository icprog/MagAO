; $Id: ramp_ol_gain.pro,v 1.2 2005/12/12 12:10:37 labot Exp $
;+
; NAME:
;   RAMP_OL_GAIN
;
; PURPOSE:
;	 This .pro provides to raise the proportional or speed gain control by steps.
;
;
; CATEGORY:
;   Adsec Hardware Function, Level 3
;
; CALLING SEQUENCE:
;		err=ramp_ol gain(final_gain,  BLOCK=block, MAXSTEP = max_ol_step, FITSFILE = fitsfile, MODE_LIST=mode_list, WFSST = wfsst, NFRAMES2CHECK=nframes2check)
;
; KEYWORDS:
;   BLOCK = block   :if set or unset, it refers to B or A matrices blocks gain of the slope filter.
;
; HISTORY:
;  Created by Marco Xompero (MX) on 29 Nov 2005
;  marco@arcetri.astro.it
;-
;
Function ramp_ol_gain, final_gain,  BLOCK=block, MAXSTEP = max_ol_step, FITSFILE = fitsfile, MODE_LIST=mode_list, WFSST = wfsst, NFRAMES2CHECK=nframes2check

    @adsec_common

    ;test on fits file
    if keyword_set(fitsfile) then begin

        file = file_search(fitsfile)
       
        if file eq "" then begin
            message, "The fits file does not exist. No gain raised up.", CONT = (sc.debug eq 0)
            return, adsec_error.input_type
        endif else begin

            final_gain = readfits(fitsfile)

        endelse
    endif

    
    ;test on mode list
    if n_elements(mode_list) eq 0  then mode_list = indgen(adsec.n_actuators)

    if test_type(MODE_LIST, /INT, /LONG, N_EL=nel) then begin
        message, "Wrong format of mode list. It should be an integer array. No gains raised up.", CONT = (sc.debug eq 0)
        return, adsec_error.input_type
    endif 

    dummy = where(mode_list gt adsec.n_actuators, cm)
    if cm gt 0 then begin
        message, "Wrong format of mode list. There are modes not permitted. No gains raised up.", CONT = (sc.debug eq 0)
        return, adsec_error.input_type
    endif 

    if n_elements(mode_list) ne n_elements(uniq(mode_list)) then begin
        message, "Wrong format of mode list. There are modes repeated more than one time. No gains raised up.", CONT = (sc.debug eq 0)
        return, adsec_error.input_type
    endif 

    the_mode_list = mode_list
        

    ;test on final gain set
    if test_type(final_gain, /FLOAT) then begin
        message, "The gains set must be float type. No gain raised up.", CONT = (sc.debug eq 0)
        return, adsec_error.input_type
    endif

    if n_elements(final_gain) eq 1 then begin 
        final_gain = final_gain[0]
		the_final_gain=replicate(final_gain, n_elements(the_mode_list)) 
    endif else begin
        the_final_gain = final_gain
    endelse


    if  (n_elements(the_final_gain) ne n_elements(the_mode_list)) then begin
        message, "The gains set must be a scalar or a vector of n_actuators size. No gain raised up.", CONT = (sc.debug eq 0)
        return, adsec_error.input_type
    endif



    ;test on gain step set
    if n_elements(max_ol_step) eq 0 then max_ol_step = 1e-2
    max_ol_step = max_ol_step[0]
    if test_type(max_ol_step, /FLOAT) then begin
        message, "The step  must be float type. No gain raised up.", CONT = (sc.debug eq 0)
        return, adsec_error.input_type
    endif

    if max_ol_step gt 1e-1 then begin
        message, 'Too large step set. gain no applied.', cont=(sc.debug eq 0)
        return, adsec_error.input_type
    endif

;	if n_elements(final_gain) eq 1 then $
;		the_final_gain=replicate(final_gain, n_elements(the_mode_list)) $
;	else $
;		if n_elements(final_gain) ne n_elements(the_mode_list) then begin
;			message, "Gain and actuator list must have the same size"
;		endif;
;
	if min(final_gain) lt 0 then begin
            message, "Gain cannot be negative", CONT=(sc.debug eq 0)
            return, adsec_error.input_type
	endif

    if n_elements(nframes2check) eq 0 then nframes2check = 30

    ;fill the gains vector with appropriate gains
    new_gain = fltarr(adsec.n_actuators)
    new_gain[the_mode_list] = final_gain

    ;step planning
    uidx = uniq(final_gain, sort(final_gain))
    uniq_gain = final_gain[uidx]
    ;sidx = sort(uniq_gain)
    ;sort_gain = uniq_gain[sidx]

    ;get the current gain
    err = get_g_gain(base_gain, BLOCK=block)
    if err ne adsec_error.ok then return, err
   
    ;cicle on the gains
    for i=0, n_elements(uidx)-1 do begin
        
      ;  print, uidx
       ; stop
        ;get the current gain
        err = get_g_gain(curr_gain, BLOCK=block)
        if err ne adsec_error.ok then return, err

        id_modes = where(new_gain ge uniq_gain[i], cc)
    
        ;print, "Gain attuale :",  curr_gain[id_modes]
        ;print, "Guadagno parziale da raggiungere", sort_gain[i]
        ;print, "step "+strtrim(i, 2)+" con modi " + strtrim(cc,2)
        ;print, "indice modi", id_modes

        ;find the delta gain
        dgain = dblarr(adsec.n_actuators)
        dgain[id_modes] = replicate(uniq_gain[i], cc) - curr_gain[id_modes]
        id_modes_change = where(dgain ne 0, imc)
        ;print, "delta gain", dgain
        print, "Modes to change " + strtrim(imc, 2) 
        
        ;if there's no modes to change, skip
        if imc eq 0 then continue;
        
        n_step = round((max(abs(dgain)) > 0)/(max_ol_step)) > 1

        min_gain = curr_gain[id_modes] < new_gain[id_modes]
        max_gain = curr_gain[id_modes] > new_gain[id_modes]

        time2wait = wfsst*nframes2check


        for j=1,n_step do begin

            intermediate_gain = float(((curr_gain[id_modes_change]+dgain[id_modes_change]/n_step*j) < max_gain) > min_gain)

            print, "Set gain ", mean(intermediate_gain)

            err = set_g_gain(id_modes_change, intermediate_gain, BLOCK=block)
            if err ne adsec_error.ok then return, err

            ;wait for fast diagnostic recovering data
            wait, time2wait
            undefine, status

            ;test oscillation via fast diag
            err = oscillation_check(status, nframes2check)
            if err ne adsec_error.ok then return, err

            if status then begin
                err =  set_g_gain(sc.all, base_gain, BLOCK=block)
                if err ne adsec_error.ok then return, err
                    message, "The mirror is going to oscillate. Gain reported to previous value.", CONT = (sc.debug eq 0)
                    return, err
                endif

         endfor
    endfor

	return, adsec_error.ok
end
