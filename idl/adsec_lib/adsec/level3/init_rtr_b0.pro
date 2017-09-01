;+
;   NAME:
;    INIT_RTR_B0
;
;   PURPOSE:
;       Restore the B0 matrix
;
;   USAGE:
;    err = init_rtr_B0(B0_FILENAME=B0_filename, STRUCT=struct, NOFILL=nofill)
;
;   INPUT:
;       None.
;   OUTPUT:
;    Error code.
;
;   KEYWORD:
;
;    B0_A_FILENAME : path of file where is saved the B0_matrix for the block memory A.
;    B0_B_FILENAME : path of file where is saved the B0_matrix for the block memory B.
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
;   January 2006 DZ
;   Added the fields for each block memory.
;   Deleted meas_b0_used field. 
;   Added a writefits if no B0 matrix exists.
;   Added keyword nowrite.  
;   NOTE:
;    None.
;-

function init_rtr_b0, B0_A_FILENAME=b0_a_filename,B0_B_FILENAME=b0_b_filename, STRUCT=struct, NOFILL=nofill, $
                      NO_WRITE=no_write  

@adsec_common

; Restoring the b0 matrix
;
n_slope = rtr.n_slope
n_ch = adsec.n_act_per_dsp
n_acts = adsec.n_actuators
n_dsp = adsec.n_dsp
no_write=1

if (keyword_set(b0_a_filename)) then begin

    check = file_search(b0_a_filename)

    if check[0] eq ""  then begin
        message, 'The file '+b0_a_filename+ $
        " containing the B0 matrix doesn't exist", /INFO
        print, 'A zero matrix will be used as B0 matrix'
        b0_a_matrix = fltarr(n_slope,n_ch*n_dsp)
        if ~keyword_set(no_write) then writefits, adsec_path.data+'b0_a_matrix.fits', b0_a_matrix
    endif else begin
        b0_a_matrix = 0
        b0_a_matrix = readfits(b0_a_filename)
    endelse
endif else begin
    message, "No file containing b0_matrix was selected ", /INFO
    print, 'A zero matrix will be used as B0 matrix gain'
    b0_a_matrix = fltarr(n_slope,n_ch*n_dsp)
    if ~keyword_set(no_write) then writefits, adsec_path.data+'b0_a_matrix.fits', b0_a_matrix
endelse

if test_type(b0_a_matrix, /REAL, DIM=dim) then $
   message, "The B0 matrix block A must be real"
   if dim[0] ne 2 then $
      message, "The B0 matrix block A must be a 2-D matrix"
      if total(dim eq [2, n_slope, n_acts]) ne 3 then $
         message, "The B0 matrix block A must be a " $
                +strtrim(n_slope,2)+"x"+strtrim(n_acts,2)+" matrix."

if (keyword_set(b0_b_filename)) then begin

    check = file_search(b0_b_filename)

    if check[0] eq ""  then begin
        message, 'The file '+b0_b_filename+ $
        " containing the B0 matrix doesn't exist", /INFO
        print, 'A zero matrix will be used as B0 matrix'
        b0_b_matrix = fltarr(n_slope,n_ch*n_dsp)
        if ~keyword_set(no_write) then writefits, adsec_path.data+'b0_b_matrix.fits', b0_b_matrix
    endif else begin
        b0_b_matrix = 0
        b0_b_matrix = readfits(b0_b_filename)
    endelse
endif else begin
    message, "No file containing b0_matrix was selected ", /INFO
    print, 'A zero matrix will be used as B0 matrix gain'
    b0_b_matrix = fltarr(n_slope,n_ch*n_dsp)
    if ~keyword_set(no_write) then writefits, adsec_path.data+'b0_b_matrix.fits', b0_b_matrix
endelse

if test_type(b0_b_matrix, /REAL, DIM=dim) then $
   message, "The B0 matrix block B must be real"
   if dim[0] ne 2 then $
      message, "The B0 matrix block B must be a 2-D matrix"
      if total(dim eq [2, n_slope, n_acts]) ne 3 then $
         message, "The B0 matrix block B must be a " $
                +strtrim(n_slope,2)+"x"+strtrim(n_acts,2)+" matrix."

;===================================================================================================
;===================================================================================================
;
struct = $
  { $
    b0_a_matrix : ptr_new(/alloc), $ ; slope filter matrix ptr, n_modes X n_slope (ccount/pcount)
    b0_b_matrix : ptr_new(/alloc) $ ; slope filter matrix ptr, n_modes X n_slope (ccount/pcount)
 }
*struct.b0_a_matrix = b0_a_matrix
*struct.b0_b_matrix = b0_b_matrix
;
;==================================================================================================
;
if not(keyword_set(nofill)) then begin
    ptr_free,rtr.b0_a_matrix
    ptr_free,rtr.b0_b_matrix
    rtr.b0_a_matrix = ptr_new(b0_a_matrix) ; slope filter matrix, n_modes X n_slope (ccount/pcount)
    rtr.b0_b_matrix = ptr_new(b0_b_matrix) ; slope filter matrix, n_modes X n_slope (ccount/pcount)
endif


return, adsec_error.ok
end
