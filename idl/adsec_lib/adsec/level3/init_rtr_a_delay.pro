;+
;   NAME:
;    INIT_RTR_A_DELAY
;
;   PURPOSE:
;       Restore the A_delay  matrix
;
;   USAGE:
;    err = init_rtr_a_delay(A_delay_FILENAME=A_delay_filename, STRUCT=struct, NOFILL=nofill)
;
;   INPUT:
;       None.
;   OUTPUT:
;    Error code.
;
;   KEYWORD:
;
;    A_DELAY_FILENAME : path of file where is saved the A_delay_matrix[...]
;    STRUCT      : structure creates in the file in which the feedforward data are saved.
;    NOFIL       : If it is set, the adsec feedforward fields are filled up.
;
;  COMMON BLOCKS:
;   ADSEC               : secondary adaptive base common block. A part of it will be filled.
;
;  PACKAGE:
;   LBT Adsec Libraries.
;
;  HISTORY
;   Created on 4 Feb 2005 by Daniela Zanotti (DZ).
;   29 Aug 2005, Armando Riccardi
;    Wromg use of rtr.n_slope_delay corrected with rtr.n_delatpos_delay. Case of one single
;    non zero modes_delay now managed.
;   07 Oct 2005, MX
;     RTR matrices now managed by pointers.
;   23 Jan 2006, DZ
;     Deleted meas_a_delay_used.  
;     Added a writefits if no A_DELAY matrix exists.
;   February, 2006, DZ
;     Added keyword nowrite.        
;   07 Feb 2007 MX
;       LUT of variable deleted and name refurbished.
;   NOTE:
;    None.
;-

function init_rtr_a_delay,  A_DELAY_FILENAME=a_delay_filename, STRUCT=struct, NOFILL=nofill, NO_WRITE=no_write

@adsec_common

;
n_modes_delay = rtr.n_modes_delay
n_ch = adsec.n_act_per_dsp
n_acts = adsec.n_actuators
n_dsp = adsec.n_dsp
n_modes = rtr.n_modes2correct
no_write = 1

; Restoring the A_delay matrix
if (keyword_set(a_delay_filename)) then begin

    check = file_search(a_delay_filename)

    if check[0] eq ""  then begin
        message, 'The file '+a_delay_filename+ $
        " containing the A_delay matrix doesn't exist", /INFO
        print, 'A zero matrix will be used as A_delay matrix'
        a_delay_matrix = fltarr(n_acts,n_ch*n_dsp,n_modes_delay)
        if n_modes_delay eq 1 then a_delay_matrix = reform(temporary(a_delay_matrix),n_acts,n_ch*n_dsp,n_modes_delay)
        if ~keyword_set(no_write) then writefits, adsec_path.data+'a_delay_matrix.fits', a_delay_matrix
    endif else begin
        a_delay_matrix = 0
        a_delay_matrix =  readfits(a_delay_filename)
        if n_modes_delay eq 1 then a_delay_matrix = reform(temporary(a_delay_matrix),n_acts,n_ch*n_dsp,n_modes_delay)
    endelse
endif else begin
    message, "No file containing A_delay_matrix was selected ", /INFO
    print, 'A zero matrix will be used as A_delay matrix gain'
    meas_a_delay_used = 0B
    a_delay_matrix = fltarr(n_acts,n_ch*n_dsp,n_modes_delay)
    if n_modes_delay eq 1 then a_delay_matrix = reform(temporary(a_delay_matrix),n_acts,n_ch*n_dsp,n_modes_delay)
    if ~keyword_set(no_write) then writefits, adsec_path.data+'a_delay_matrix.fits', a_delay_matrix
endelse

if test_type(a_delay_matrix, /REAL, DIM=dim) then $
   message, "The A_delay matrix must be real"
   if dim[0] ne 3 then $
      message, "The A_delay must be a 3-D matrix"
      if total(dim eq [3, n_acts, n_acts, n_modes_delay]) ne 4 then $
          message, "The A_delay matrix must be a " $
          +strtrim(n_acts,2)+"x"+strtrim(n_acts,2)$
          +"x"+strtrim(n_modes_delay,2)+" matrix."



;===================================================================================================
;
struct = $
  { $
    a_delay_matrix : ptr_new(/alloc) $ ; feedforward matrix: n_act X n_act (ccount/pcount)
 }
*struct.a_delay_matrix = a_delay_matrix
;
;==================================================================================================
;
if not(keyword_set(nofill)) then begin
    ptr_free, rtr.a_delay_matrix
    rtr.a_delay_matrix = ptr_new(a_delay_matrix) ; feedforward matrix: n_act X n_act (ccount/pcount)
endif


return, adsec_error.ok
end
