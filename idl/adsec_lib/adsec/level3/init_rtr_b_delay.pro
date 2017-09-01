;+
;   NAME:
;    INIT_RTR_B_DELAY
;
;   PURPOSE:
;       Restore the B_delay matrix
;
;   USAGE:
;    err = init_rtr_b_delay(B_delay_FILENAME=B_delay_filename, STRUCT=struct, NOFILL=nofill)
;
;   INPUT:
;       None.
;   OUTPUT:
;    Error code.
;
;   KEYWORD:
;
;    B_DELAY_A_FILENAME : path of file where is saved the
;                       B_delay_matrix[n_slope,n_act,n_slope_delay] for block memory A.
;    B_DELAY_B_FILENAME : path of file where is saved the
;                       B_delay_matrix[n_slope,n_act,n_slope_delay] for block memory B.
;    STRUCT      : structure creates in the file in which the feedforward data are saved.
;    NOFIL       : If it is set, the adsec feedforward fields are filled up.
;
;  COMMON BLOCKS:
;   RTR               : reconstructor base common block. A part of it will be filled.
;
;  PACKAGE:
;   LBT Adsec Libraries.
;
;  HISTORY
;   Created on 4 Feb 2005 by Daniela Zanotti (DZ).
;   29 Aug 2005, Armando Riccardi (AR)
;    Case of one single non zero modes_delay now managed.
;   07 Oct 2005, MX
;     RTR matrices now managed by pointers.
;   January 2006 DZ
;   Added the fields for each block memory.
;   Deleted meas_b_delay_used field. 
;   Added a writefits if no B_DELAY matrix exists. 
;   Added keyword nowrite.  
;   07 Feb 2007 MX
;       LUT of variable deleted and name refurbished.
;   NOTE:
;    None.
;-

function init_rtr_b_delay, B_DELAY_A_FILENAME=b_delay_a_filename,B_DELAY_B_FILENAME=b_delay_b_filename,$
                           STRUCT=struct, NOFILL=nofill, NO_WRITE=NO_WRITE

@adsec_common

;
n_slope = rtr.n_slope
n_slope_delay = rtr.n_slope_delay
n_ch = adsec.n_act_per_dsp
n_acts = adsec.n_actuators
n_dsp = adsec.n_dsp
no_write=1

; Restoring the B_delay matrix
if (keyword_set(b_delay_a_filename)) then begin

    check = file_search(b_delay_a_filename)

    if check[0] eq ""  then begin
        message, 'The file '+b_delay_a_filename+ $
        " containing the B_delay matrix doesn't exist", /INFO
        print, 'A zero matrix will be used as B_delay matrix'
        b_delay_a_matrix = fltarr(n_slope,n_ch*n_dsp,n_slope_delay)
        if n_slope_delay eq 1 then b_delay_a_matrix = reform(temporary(b_delay_a_matrix),n_slope,n_ch*n_dsp,n_slope_delay)
        if ~keyword_set(no_write) then writefits, adsec_path.data+'b_delay_a_matrix.fits', b_delay_a_matrix

    endif else begin
        b_delay_a_matrix = 0
        b_delay_a_matrix = readfits(b_delay_a_filename)
        if n_slope_delay eq 1 then b_delay_a_matrix = reform(temporary(b_delay_a_matrix),n_slope,n_ch*n_dsp,n_slope_delay)
    endelse
endif else begin
    message, "No file containing B_delay_matrix block a was selected ", /INFO
    print, 'A zero matrix will be used as B_delay_A matrix  gain'
    b_delay_a_matrix = fltarr(n_slope,n_ch*n_dsp,n_slope_delay)
    if n_slope_delay eq 1 then b_delay_a_matrix = reform(temporary(b_delay_a_matrix),n_slope,n_ch*n_dsp,n_slope_delay)
    if ~keyword_set(no_write) then writefits, adsec_path.data+'b_delay_a_matrix.fits', b_delay_a_matrix

endelse
if test_type(b_delay_a_matrix, /REAL, DIM=dim) then $
   message, "The B_delay_A matrix must be real"
   if dim[0] ne 3 then $
      message, "The B_delay_A matrix must be a 3-D matrix"
      if total(dim eq [3, n_slope, n_acts, n_slope_delay]) ne 4 then $
         message, "The B_delay_A matrix must be a " $
                   +strtrim(n_slope,2)+"x"+strtrim(n_acts,2)$
                   +"x"+strtrim(n_slope_delay,2)+" matrix."


; Restoring the B_delay matrix
if (keyword_set(b_delay_b_filename)) then begin

    check = file_search(b_delay_b_filename)

    if check[0] eq ""  then begin
        message, 'The file '+b_delay_b_filename+ $
        " containing the B_delay matrix doesn't exist", /INFO
        print, 'A zero matrix will be used as B_delay matrix'
        b_delay_b_matrix = fltarr(n_slope,n_ch*n_dsp,n_slope_delay)
        if n_slope_delay eq 1 then b_delay_b_matrix = reform(temporary(b_delay_b_matrix),n_slope,n_ch*n_dsp,n_slope_delay)
        if ~keyword_set(no_write) then writefits, adsec_path.data+'b_delay_b_matrix.fits', b_delay_b_matrix
    endif else begin
        b_delay_b_matrix = 0
        b_delay_b_matrix = readfits(b_delay_b_filename)
        if n_slope_delay eq 1 then b_delay_b_matrix = reform(temporary(b_delay_b_matrix),n_slope,n_ch*n_dsp,n_slope_delay)
    endelse
endif else begin
    message, "No file containing B_delay_matrix block B was selected ", /INFO
    print, 'A zero matrix will be used as B_delay_B matrix  gain'
    b_delay_b_matrix = fltarr(n_slope,n_ch*n_dsp,n_slope_delay)
    if n_slope_delay eq 1 then b_delay_b_matrix = reform(temporary(b_delay_b_matrix),n_slope,n_ch*n_dsp,n_slope_delay)
    if ~keyword_set(no_write) then writefits, adsec_path.data+'b_delay_b_matrix.fits', b_delay_b_matrix
endelse

if test_type(b_delay_b_matrix, /REAL, DIM=dim) then $
   message, "The B_delay_B matrix must be real"
   if dim[0] ne 3 then $
      message, "The B_delay_B matrix must be a 3-D matrix"
      if total(dim eq [3, n_slope, n_acts, n_slope_delay]) ne 4 then $
         message, "The B_delay_B matrix must be a " $
                   +strtrim(n_slope,2)+"x"+strtrim(n_acts,2)$
                   +"x"+strtrim(n_slope_delay,2)+" matrix."
;===================================================================================================
;
struct = { $
    b_delay_a_matrix : ptr_new(/alloc),  $ ; feedforward matrix: n_act X n_act (ccount/pcount)
    b_delay_b_matrix : ptr_new(/alloc) $ ; feedforward matrix: n_act X n_act (ccount/pcount)
 }
*struct.b_delay_a_matrix = b_delay_a_matrix
*struct.b_delay_b_matrix = b_delay_b_matrix

;
;==================================================================================================
;
if not(keyword_set(nofill)) then begin
    ptr_free, rtr.b_delay_a_matrix
    ptr_free, rtr.b_delay_b_matrix
    rtr.b_delay_a_matrix = ptr_new(b_delay_a_matrix) ; feedforward matrix: n_act X n_act (ccount/pcount)
    rtr.b_delay_b_matrix = ptr_new(b_delay_b_matrix) ; feedforward matrix: n_act X n_act (ccount/pcount)
endif


return, adsec_error.ok
end
