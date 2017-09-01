;+
;   NAME:
;    INIT_ADSEC_B_DELAY
;
;   PURPOSE:
;       Restore the B_delay   matrix
;
;   USAGE:
;    err = init_adsec_b_delay(B_delay_FILENAME=B_delay_filename, STRUCT=struct, NOFILL=nofill)
;
;   INPUT:
;       None.
;   OUTPUT:
;    Error code.
;
;   KEYWORD:
;
;    B_delay_FILENAME : path of file where is saved the B_delay_matrix[n_slope,n_act,n_slope_delay]
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
;
;   NOTE:
;    None.
;-

function init_adsec_b_delay, B_DELAY_FILENAME=b_delay_filename, STRUCT=struct, NOFILL=nofill

@adsec_common

;
n_slope = adsec.n_slope
n_slope_delay = adsec.n_slope_delay
n_ch = adsec.n_act_per_dsp
n_acts = adsec.n_actuators
n_dsp = adsec.n_dsp

; Restoring the B_delay matrix
if (keyword_set(b_delay_filename)) then begin
    
    check = file_search(b_delay_filename)

    if check[0] eq ""  then begin
        message, 'The file '+b_delay_filename+ $
        " containing the B_delay matrix doesn't exist", /INFO
        print, 'A zero matrix will be used as B_delay matrix'
        meas_b_delay_used = 0B
        b_delay_matrix = fltarr(n_slope,n_ch*n_dsp,n_slope_delay)
    endif else begin
        b_delay_matrix = 0 
        restore, b_delay_filename
        meas_b_delay_used = 1B
    endelse
endif else begin
    message, "No file containing B_delay_matrix was selected ", /INFO
    print, 'A zero matrix will be used as B_delay matrix gain'
    meas_b_delay_used = 0B
    b_delay_matrix = fltarr(n_slope,n_ch*n_dsp,n_slope_delay)
endelse
if test_type(b_delay_matrix, /REAL, DIM=dim) then $
   message, "The B_delay matrix must be real"
   if dim[0] ne 3 then $
      message, "The B_delay matrix must be a 3-D matrix"
      if total(dim eq [3, n_slope, n_acts, n_slope_delay]) ne 4 then $
         message, "The B_delay matrix must be a " $
                   +strtrim(n_slope,2)+"x"+strtrim(n_acts,2)$
                   +"x"+strtrim(n_slope_delay,2)+" matrix."


;===================================================================================================
;
struct = $
  { $
    meas_b_delay_used : meas_b_delay_used ,$  ; 1B: a k matrix stored in data dir has been used (0B: eitherwise)
    b_delay_matrix : b_delay_matrix $ ; feedforward matrix: n_act X n_act (ccount/pcount)
 }
 
;
;==================================================================================================
;
if not(keyword_set(nofill)) then begin
    adsec.meas_b_delay_used = meas_b_delay_used ; 1B: a k matrix stored in data dir has been used (0B: eitherwise)
    adsec.b_delay_matrix = b_delay_matrix ; feedforward matrix: n_act X n_act (ccount/pcount)
endif


return, adsec_error.ok
end
