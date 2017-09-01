;+
;   NAME:
;    INIT_RTR_G_GAIN
;
;   PURPOSE:
;       Restore the G  gain for reconstructor of commands
;
;   USAGE:
;    err = init_rtr_g_gain(G_GAIN_FILENAME=G_GAIN_filename, STRUCT=struct, NOFILL=nofill)
;
;   INPUT:
;       None.
;   OUTPUT:
;    Error code.
;
;   KEYWORD:
;
;    G_GAIN_A_FILENAME : path of file where is saved the Optical loop gain vector for block A.
;    G_GAIN_B_FILENAME : path of file where is saved the Optical loop gain vector for block B.
;    STRUCT      : structure creates in the file in which the feedforward data are saved.
;    NOFIL       : If it is set, the adsec feedforward fields are filled up.
;
;  COMMON BLOCKS:
;   rtr               : reconstructor base common block. A part of it will be filled.
;
;  PACKAGE:
;   LBT Adsec Libraries.
;
;  HISTORY
;   Created on 5 Feb 2005 by Daniela Zanotti (DZ).
;   07 Oct 2005, MX
;     RTR matrices now managed by pointers.
;    January 2006 DZ
;   Added the fields for each block memory.
;   Deleted meas_g_gain_used field. 
;   Added a writefits if no G_GAIN matrix exists. 
;   Added keyword nowrite.  
;
;   NOTE:
;    None.
;-

function init_rtr_g_gain, G_GAIN_A_FILENAME=g_gain_a_filename, G_GAIN_B_FILENAME=g_gain_b_filename,$
                           STRUCT=struct, NOFILL=nofill, NO_WRITE=no_write

@adsec_common

; Restoring the b0 matrix
;
n_slope = rtr.n_slope
n_ch = adsec.n_act_per_dsp
n_acts = adsec.n_actuators
n_dsp = adsec.n_dsp
no_write=1

if (keyword_set(g_gain_a_filename)) then begin

    check = file_search(g_gain_a_filename)

    if check[0] eq ""  then begin
        message, 'The file '+g_gain_a_filename+ $
        " containing the optical loop gain for block A  doesn't exist", /INFO
        print, 'A zero vrctor will be used as gain G'
        g_gain_a = fltarr(n_acts)
        if ~keyword_set(no_write) then writefits, adsec_path.data+'g_gain_a.fits', g_gain_a

    endif else begin
        g_gain_a = 0
        g_gain_a = readfits(g_gain_a_filename)
    endelse
endif else begin
    message, "No file containing the optical loop gain for block A was selected ", /INFO
    print, 'A zero vector will be used as G gain'
    g_gain_a = fltarr(n_acts)
    if ~keyword_set(no_write) then writefits, adsec_path.data+'g_gain_a.fits', g_gain_a
endelse

if test_type(g_gain_a, /FLOAT, DIM=dim) then $
   message, "The  G gain  must be float"
   if dim[0] ne 1 then $
      message, "The G gain must be a 1-D matrix"
      if total(dim eq [1, n_acts]) ne 2 then $
         message, "The g gain must be a " $
                +strtrim(n_acts,2)+" vector."

if (keyword_set(g_gain_b_filename)) then begin

    check = file_search(g_gain_b_filename)

    if check[0] eq ""  then begin
        message, 'The file '+g_gain_b_filename+ $
        " containing the optical loop gain for block B  doesn't exist", /INFO
        print, 'A zero vrctor will be used as gain G'
        g_gain_b = fltarr(n_acts)
        if ~keyword_set(no_write) then writefits, adsec_path.data+'g_gain_b.fits', g_gain_b
    endif else begin
        g_gain_b = 0
        g_gain_b = readfits(g_gain_b_filename)
    endelse
endif else begin
    message, "No file containing the optical loop gain for block B was selected ", /INFO
    print, 'A zero vector will be used as G gain'
    g_gain_b = fltarr(n_acts)
    if ~keyword_set(no_write) then writefits, adsec_path.data+'g_gain_b.fits', g_gain_b
endelse

if test_type(g_gain_b, /FLOAT, DIM=dim) then $
   message, "The  G gain  must be float"
   if dim[0] ne 1 then $
      message, "The G gain must be a 1-D matrix"
      if total(dim eq [1, n_acts]) ne 2 then $
         message, "The g gain must be a " $
                +strtrim(n_acts,2)+" vector."

;===================================================================================================
;===================================================================================================
;
struct = $
  { $
    g_gain_a : ptr_new(/alloc), $ ;G gain vector: n_act 
    g_gain_b : ptr_new(/alloc) $ ;G gain vector: n_act 
 }
*struct.g_gain_a = g_gain_a
*struct.g_gain_b = g_gain_b
;
;==================================================================================================
;
if not(keyword_set(nofill)) then begin
    ptr_free, rtr.g_gain_a
    ptr_free, rtr.g_gain_b
    rtr.g_gain_a = ptr_new(g_gain_a) ; G gain vector: n_act
    rtr.g_gain_b = ptr_new(g_gain_b) ; G gain vector: n_act
endif


return, adsec_error.ok
end
