; $Id: init_adsec_filters.pro,v 1.3 2009/09/18 08:25:01 marco Exp $$
;
;+
;  NAME:
;   INIT_ADSEC_FILTERS
;
;  PURPOSE:
;   Load filters parameter in the adsec structure.
;
;  USAGE:
;   err = INIT_ADSEC_FILTERS(filename, TFL=tfl_data_name, STRUCT=struct, NOFILL=nofill)
;
;  INPUT:
;   filename(optional)  : filename of data to load. Default adsec_path.conf+'filters_LBT672.txt'.
;
;  OUTPUT:
;   error               : error code.
;
;  KEYWORDS:
;   TFL                 : filename con fullpath of a .sav file with tfl_par and err_par structure.
;   STRUCT              : structure with parameters loaded and calculated.
;   NOFILL              : do not fill the adsec structure.
;
;  COMMON BLOCKS:
;   ADSEC               : secondary adaptive base common block. A part of it will be filled.
;
;  PACKAGE:
;   LBT Adsec Libraries.
;
;  HISTORY
;   Created on 26 Oct 2004 by Marco Xompero (MX).
;   marco@arcetri.astro.it
;  
;  01/12/2004 MX
;   Parameters riorganization and default configuration filename changed.
;-

Function init_adsec_filters, filename, TFL=tfl_data_filename, STRUCT=struct, NOFILL=nofill


   @adsec_common

   if n_elements(filename) eq 0 then filename=filepath(ROOT=adsec_path.conf,SUB='general', 'filters.txt')
   if n_elements(tfl_data_filename) eq 0 then tfl_data_filename = filepath(ROOT=adsec_path.data,'tfl_data.sav')



   exist = file_search(filename)

   if exist eq "" then begin
      message, "Filter parameters file not found!!", CONT = (sc.debug eq 0)
      return, adsec_error.invalid_file
   endif else begin

      data = read_ascii_structure(filename)
   endelse

   n_acts = adsec.n_actuators
;===========================================================================
; Restoring the time filter data
   gain_to_bbgain=fltarr(n_acts)
   default_deriv_gain = data.default_deriv_gain
   default_deriv_poles_freq = 2*!DPI*data.default_deriv_poles
   default_deriv_zeros_freq = 2*!DPI*data.default_deriv_zeros

	idx_notnull_poles = where(data.default_deriv_poles ne 0 and data.default_deriv_poles ne -2, ccp)
	idx_notnull_zeros = where(data.default_deriv_zeros ne 0 and data.default_deriv_zeros ne -2, ccz)

; returns a structure for speed loop filter definition
   tfl_par = tfl_gen_default()

	tfl_max_n_coeff = n_elements(tfl_par.s_zero)
   tfl_par.n_s_pole = data.tfl_n_s_pole
   tfl_par.n_s_zero = data.tfl_n_s_zero

   if tfl_par.n_s_pole lt tfl_par.n_s_zero then $
      message, "Unexpected default par structure. Max #poles have match max #zeros.", CONT=(sc.debug eq 0)

   if (tfl_par.n_s_pole gt dsp_const.max_filters_coeff) or (tfl_par.n_s_zero gt dsp_const.max_filters_coeff) then $
      message, "Unexpected default par structure. Max #poles or #zeros must be less than dsp_const.max_filters_coeff.", CONT=(sc.debug eq 0)

   tfl_par.s_zero = default_deriv_zeros_freq
   tfl_par.s_pole = default_deriv_poles_freq

	if ccp eq 0 then prodp = 1d else prodp = product(default_deriv_poles_freq[idx_notnull_poles])
	if ccz eq 0 then prodz = 1d else prodz = product(default_deriv_zeros_freq[idx_notnull_zeros])

   tfl_par.s_const = default_deriv_gain*prodp/prodz

; copy the same filter for all actuators
   tfl_par = replicate(tfl_par, n_acts)

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Restoring the time filter data

   default_err_gain = data.default_err_gain
	
	if data.err_n_s_pole eq 0 then default_err_poles_freq = 0d else default_err_poles_freq = 2*!DPI*data.default_err_poles
	if data.err_n_s_zero eq 0 then default_err_zeros_freq = 0d else default_err_zeros_freq = 2*!DPI*data.default_err_zeros

	idx_notnull_poles = where(data.default_err_poles ne 0 and data.default_err_poles ne -2, ccp)
	idx_notnull_zeros = where(data.default_err_zeros ne 0 and data.default_err_zeros ne -2, ccz)

; returns a structure for position error loop filter definition
   err_par = tfl_gen_default()

   err_par.n_s_pole = data.err_n_s_pole
   err_par.n_s_zero = data.err_n_s_zero

   if err_par.n_s_pole lt err_par.n_s_zero then $
      message, "Unexpected default par structure. Max #poles have match max #zeros.", CONT=(sc.debug eq 0)

   if (err_par.n_s_pole gt dsp_const.max_filters_coeff) or (err_par.n_s_zero gt dsp_const.max_filters_coeff) then $
      message, "Unexpected default par structure. Max #poles or #zeros must be less than dsp_const.max_filters_coeff.", CONT=(sc.debug eq 0)

   err_par.s_zero = default_err_zeros_freq
   err_par.s_pole = default_err_poles_freq

	if ccp eq 0 then prodp = 1d else prodp = product(default_err_poles_freq[idx_notnull_poles])
	if ccz eq 0 then prodz = 1d else prodz = product(default_err_zeros_freq[idx_notnull_zeros])

   err_par.s_const = default_err_gain*prodp/prodz

; copy the same filter for all actuators
   err_par = replicate(err_par, n_acts)
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

; check for filter data already saved
   check = file_search(tfl_data_filename)
   if check[0] eq "" then begin
       message, 'The file '+tfl_data_filename+ $
         " containing the control law data (speed and pos-error) doesn't exist", /INFO
       print, 'Default values for the control law will be used'

   endif else begin

       tfl_par=0 & err_par=0
       restore, tfl_data_filename

       if test_type(tfl_par, /STRUC, DIM=dim) then $
         message, "tfl_par must be a vector of structures", CONT = (sc.debug eq 0)
          if dim[0] ne 1 then $
         message, "tfl_par must be a vector", CONT = (sc.debug eq 0)
       if total(dim ne [1, n_acts]) ne 0 then $
         message, "The tfl_par structure vector must have " $
        +strtrim(n_acts,2)+" elements.", CONT = (sc.debug eq 0)

       if test_type(err_par, /STRUC, DIM=dim) then $
         message, "err_par must be a vector of structures", CONT = (sc.debug eq 0)
       if dim[0] ne 1 then $
         message, "err_par must be a vector", CONT = (sc.debug eq 0)
       if total(dim ne [1, n_acts]) ne 0 then $
         message, "The err_par structure vector must have " $
           +strtrim(n_acts,2)+" elements.", CONT = (sc.debug eq 0)

   endelse

; check that the position (no position error!) loop has zero DC gain
   dummy_dc_gain = tfl_par.s_const
   for i_act=0,n_acts-1 do begin
       for i=0,tfl_par[i_act].n_s_zero-1 do dummy_dc_gain[i_act]=dummy_dc_gain[i_act]*tfl_par[i_act].s_zero[i]
       for i=0,tfl_par[i_act].n_s_pole-1 do dummy_dc_gain[i_act]=dummy_dc_gain[i_act]/tfl_par[i_act].s_pole[i]
   endfor
   if total(dummy_dc_gain ne 0d0) gt 0 then message, "The position loop filter cannot have DC gain gt 0", CONT=(sc.debug eq 0)

; computes the position error DC gain
   tfl_err_gain = err_par.s_const
   for i_act=0,n_acts-1 do begin
       for i=0,err_par[i_act].n_s_zero-1 do tfl_err_gain[i_act]=tfl_err_gain[i_act]*err_par[i_act].s_zero[i]
       for i=0,err_par[i_act].n_s_pole-1 do tfl_err_gain[i_act]=tfl_err_gain[i_act]/err_par[i_act].s_pole[i]
   endfor

;
; End of: Restoring the time filter data
;==========================================================================

; Filling the adsec structure.
   struct = {                                                              $ 
      gain_to_bbgain : float(tfl_err_gain/adsec.N_per_ccount*adsec.m_per_pcount), $
      tfl_par : tfl_par,                                                   $
      err_par : err_par,                                                   $
      tfl_max_n_coeff : tfl_max_n_coeff,                                   $
      tfl_err_gain   :   tfl_err_gain                                      $
   
}

   if not keyword_set(NOFILL) then begin
      adsec.gain_to_bbgain = float(tfl_err_gain/adsec.N_per_ccount*adsec.m_per_pcount)
      adsec.tfl_par = tfl_par
      adsec.err_par = err_par
      adsec.tfl_max_n_coeff = tfl_max_n_coeff 
      adsec.tfl_err_gain = tfl_err_gain  
   endif

   return, adsec_error.ok

End
