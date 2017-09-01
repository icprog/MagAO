; $Id: adsec_sync.pro,v 1.1 2004/11/03 14:27:25 zanotti Exp $$
;
;+
;  NAME:
;   ADSEC_SYNC
;
;  PURPOSE:
;   Load in the system the parameters stored in the common structure.
;
;  USAGE:
;   err = ADSEC_SYNC(/FF_MATRIX, /LINEAR, /FCAL, /CTRLCOEFF)
;
;  INPUT:
;   None.
;
;  OUTPUT:
;   error               : error code.
;
;  KEYWORDS:
;   FF_MATRIX           : syncronize only the ff_matrix
;   LINEAR              : syncronize only the linearization coefficients.
;   FCAL                : syncronize only the force calibration coefficients.
;   CTRLCOEFF           : syncronize only the control coefficients.
;
;  PACKAGE:
;   LBT Adsec Libraries.
;
;  HISTORY
;   Created on 28 Oct 2004 by Marco Xompero (MX).
;   marco@arcetri.astro.it
;-

Function adsec_sync, FF_MATRIX=ff_matrix, LINEAR=linear, FCAL=fcal, CTRLCOEFF=ctrlcoeff

   @adsec_common
   if (n_elements(ff_matrix) + n_elements(linear) + n_elements(fcal) + n_elements(ctrlcoeff)) eq 0 then begin
      ff_matrix=1B
      linear=1B
      fcal=1B
      ctrlcoeff=1B
   endif

   if keyword_set(linear) then begin
      err = send_linearization()
      if err ne adsec_error.ok then return, err
   endif
      
   if keyword_set(fcal) then begin
      err = send_force_calibration()
      if err ne adsec_error.ok then return, err
   endif
      
   if keyword_set(ctrlcoeff) then begin
      err = send_ctrl_coeff(sc.all_actuators)
      if err ne adsec_error.ok then return, err
   endif
      
   if keyword_set(ff_matrix) then begin
      err = send_ff_matrix(adsec.ff_matrix)
      if err ne adsec_error.ok then return, err
   endif

   return, adesc_error.ok 

End
