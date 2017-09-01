;+
;   NAME:
;    SET_G_GAIN
;
;   PURPOSE:;    Writes G into the DSP memory by splittng the matrix in adsec.n_dsp part of size
;    adsec.n_act_per_dsp or resets it to zero.
;
;   USAGE:
;    err = set_g_gain(mode_list, gain_diag, /block, /reset_all)
;
;   INPUT:
;    
;
;   OUTPUT:
;    Error code.
;
;   KEYWORD:
;    BLOCK = if set gain diagonal is written on block B.
;    RESET_ALL = if set reset chosen gain to zero. (mode_list and gain diag params are ignored)
;
;   NOTE:
;    None.
;   
; HISTORY
;   8 Feb 2005 Daniela Zanotti(DZ)
;   07 Oct 2005, Marco Xompero (MX)
;     RTR filter gain now managed by pointers.
;   29 Nov 2005 MX
;     Minor changes in routine call.
;   05 Dec 2005 MX
;     Mode list now managed. 
;     Test on input added.
;     Comments on code written.
;   07 Feb 2007 MX
;       LUT of variable deleted and name refurbished.
;   20 Feb 2007 MX
;       RTR structure now updated at the end of operations
;   09 Apr 2009 MX
;       Modified to accept fits file and to update MSGD-RTDB.
;-

Function set_g_gain, the_mode_list, g_diag_tmp, RESET_ALL=reset_all, BLOCK = block, FITSFILE = fitsfile


    ;Input parameters checks
    @adsec_common
   ; if n_elements(fitsfile) gt 0 then begin
   ;     mm = readfits(fitsfile)
   ;     the_mode_list = mm[0,*] 
   ;     g_diag_tmp = mm[1,*]
   ; endif

    if keyword_set(reset_all) then begin
        g_diag_tmp=fltarr(adsec.n_actuators)
        the_mode_list = indgen(adsec.n_actuators)
    endif


    ;mode list verification: spacial case
    if the_mode_list[0] eq sc.all_actuators then the_mode_list=indgen(adsec.n_actuators)

    ;mode list data type verification
    if test_type(the_mode_list, /INT, /LONG, N_EL=nel) then begin
        message, "Wrong format of mode list. It should be an integer array. No gains raised up.", CONT = (sc.debug eq 0)
        return, adsec_error.input_type
    endif 

    ;mode list format verification
    dummy = where(the_mode_list gt adsec.n_actuators, cm)
    if cm gt 0 then begin
        message, "Wrong format of mode list. Bad length.", CONT = (sc.debug eq 0)
        return, adsec_error.input_type
    endif 

    if n_elements(g_diag_tmp) eq 1 then g_diag_tmp = replicate(g_diag_tmp, n_elements(the_mode_list))

    ;mode list uniq verification
    if n_elements(g_diag_tmp) ne n_elements(uniq(the_mode_list)) then begin
        message, "Wrong format of mode list. There are modes repeated more than one time. No gains raised up.", CONT = (sc.debug eq 0)
        return, adsec_error.input_type
    endif 

    
    ;gains varification
    g_diag = reform(g_diag_tmp)

    ;gains data type verification
    if test_type(g_diag, /float,  dim=dim) then begin
        message, "The gain matrix must be float. No gains raised up.", CONT= (sc.debug eq 0)
        return, adsec_error.input_type
    endif

    ;gain dimensions verification
    if n_elements(g_diag) eq 1 then begin 
        g_diag = g_diag[0]
		g_diag_tmp=replicate(g_diag_tmp, n_elements(the_mode_list)) 
    endif

    if dim[0] ne 1 then begin
         message, "The gain matrix must be a vector. No gains raised up."
         return, adsec_error.input_type
    endif

    if dim[1] ne n_elements(the_mode_list) then begin
        message, "The gain must be of 'mode_list' elements. No gains raised up.", CONT = (sc.debug eq 0)
        return, adsec_error.input_type
    endif

    if (n_elements(g_diag_tmp) ne n_elements(the_mode_list)) then begin
        message, "The gains set must be a scalar or a vector of n_actuators size. No gain raised up.", CONT = (sc.debug eq 0)
        return, adsec_error.input_type
    endif

    mode_list = the_mode_list
    dummy = fltarr(adsec.n_actuators)
    dummy[mode_list] = g_diag

    ;reset all loop gains (A and B block)
    if keyword_set(reset_all) then begin 

        gain_diag = reform(dummy, adsec.n_act_per_dsp,adsec.n_dsp)
        err = write_seq_dsp(sc.all, dsp_map.gain_g_a, gain_diag , /CHECK) 
        if err ne adsec_error.ok then return, err
        err = write_seq_dsp(sc.all, dsp_map.gain_g_b, gain_diag , /CHECK) 
        if err ne adsec_error.ok then return, err
        *rtr.g_gain_a = dummy
        *rtr.g_gain_b = dummy 

    endif else begin 

        ;read the current gain and modify only the selected gains
        err = get_g_gain(old_gain, BLOCK=block)
        if err ne adsec_error.ok then return, err

        new_gain = old_gain
        new_gain[mode_list] = g_diag
        gain_diag = reform(new_gain, adsec.n_act_per_dsp,adsec.n_dsp)

        ;mansge the two blocks
        if ~keyword_set(block) then begin

            err = write_seq_dsp(sc.all, dsp_map.gain_g_a, gain_diag , /CHECK) 
            if err ne adsec_error.ok then return, err
            *rtr.g_gain_a = new_gain
            if n_elements(fitsfile) gt 0 then !AO_STATUS.g_gain_a = fitsfile else !AO_STATUS.g_gain_a=""

        endif else begin

            err = write_seq_dsp(sc.all, dsp_map.gain_g_b, gain_diag, /CHECK)
            if err ne adsec_error.ok then return, err
            *rtr.g_gain_b = new_gain
            if n_elements(fitsfile) gt 0 then !AO_STATUS.g_gain_b = fitsfile else !AO_STATUS.g_gain_b=""

        endelse

    endelse



    return, update_rtdb()

end
