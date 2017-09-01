;+
;   NAME:
;    INIT_ADSEC_B0
;
;   PURPOSE:
;       Restore the B0 matrix
;
;   USAGE:
;    err = init_adsec_B0(B0_FILENAME=B0_filename, STRUCT=struct, NOFILL=nofill)
;
;   INPUT:
;       None.
;   OUTPUT:
;    Error code.
;
;   KEYWORD:
;
;    B0_FILENAME : path of file where is saved the B0_matrix
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
;   Created on 5 Feb 2005 by Daniela Zanotti (DZ).
;
;   NOTE:
;    None.
;-

function init_adsec_b0, B0_FILENAME=b0_filename, STRUCT=struct, NOFILL=nofill

@adsec_common

; Restoring the b0 matrix
;
n_slope = adsec.n_slope
n_ch = adsec.n_act_per_dsp
n_acts = adsec.n_actuators
n_dsp = adsec.n_dsp

if (keyword_set(b0_filename)) then begin
    
    check = file_search(b0_filename)

    if check[0] eq ""  then begin
        message, 'The file '+b0_filename+ $
        " containing the B0 matrix doesn't exist", /INFO
        print, 'A zero matrix will be used as B0 matrix'
        meas_b0_used = 0B
        b0_matrix = fltarr(n_slope,n_ch*n_dsp)
    endif else begin
        b0_matrix = 0 
        restore, b0_filename
        meas_b0_used = 1B
    endelse
endif else begin
    message, "No file containing b0_matrix was selected ", /INFO
    print, 'A zero matrix will be used as B0 matrix gain'
    meas_b0_used = 0B
    b0_matrix = fltarr(n_slope,n_ch*n_dsp)
endelse

if test_type(b0_matrix, /REAL, DIM=dim) then $
   message, "The B0 matrix must be real"
   if dim[0] ne 2 then $
      message, "The B0 matrix must be a 2-D matrix"
      if total(dim eq [2, n_slope, n_acts]) ne 3 then $
         message, "The B0 matrix must be a " $
                +strtrim(n_slope,2)+"x"+strtrim(n_acts,2)+" matrix."

;===================================================================================================
;
struct = $
  { $
    meas_b0_used : meas_b0_used ,$  ; 1B: a k matrix stored in data dir has been used (0B: eitherwise)
    b0_matrix : b0_matrix $ ; feedforward matrix: n_act X n_act (ccount/pcount)
 }
;
;==================================================================================================
;
if not(keyword_set(nofill)) then begin
    adsec.meas_b0_used = meas_b0_used ; 1B: a k matrix stored in data dir has been used (0B: eitherwise)
    adsec.b0_matrix = b0_matrix ; feedforward matrix: n_act X n_act (ccount/pcount)
endif


return, adsec_error.ok
end
