
;+
;   NAME:
;    SET_MATRIX_PTR
;
;   PURPOSE:
;   Initialized the all address pointers to the respective matrix address. 
;
;   USAGE:
;    err = set_matrix_ptr()
;
;   INPUT:
;    None.
;
;   OUTPUT:
;    Error code.
;
;   KEYWORD:
;    None
;
;   NOTE:
;    None.
; HISTORY
;   4 february 2005: D. Zanotti(DZ)
;   20 May 2005: Marco Xompero (MX)
;      Dsp_map updated to rtr_map.
;-
function set_matrix_ptr

@adsec_common
; setting matrix_B0a pointer to the matrix_B0a address
 addr_matrix_B0a = rtr_map.matrix_B0a
 err = write_same_dsp(sc.all_dsp, dsp_map.matrix_B0a_ptr, addr_matrix_B0a, /CHECK)

 if err ne adsec_error.ok then begin
   print, 'Error writing address of matrix B0a'
   return, err
 endif

; setting matrix_B0b pointer to the matrix_B0b address
 addr_matrix_B0b = rtr_map.matrix_B0b
 err = write_same_dsp(sc.all_dsp, dsp_map.matrix_B0b_ptr, addr_matrix_B0b, /CHECK)

 if err ne adsec_error.ok then begin
   print, 'Error writing address of matrix B0b'
   return, err
 endif


; setting matrix_B0a_delay pointer to the matrix_B0a_delay address
 addr_matrix_Ba_delay = rtr_map.matrix_Ba_delay
 err = write_same_dsp(sc.all_dsp, dsp_map.matrix_Ba_delay_ptr, addr_matrix_Ba_delay, /CHECK)

 if err ne adsec_error.ok then begin
   print, 'Error writing address of matrix Ba_delay'
   return, err
 endif


; setting matrix_B0b_delay pointer to the matrix_B0b_delay address
 addr_matrix_Bb_delay = rtr_map.matrix_Bb_delay
 err = write_same_dsp(sc.all_dsp, dsp_map.matrix_Bb_delay_ptr, addr_matrix_Bb_delay, /CHECK)

 if err ne adsec_error.ok then begin
   print, 'Error writing address of matrix B0b_delay'
   return, err
 endif


; setting matrix_A_delay pointer to the matrix_A_delay address
 addr_matrix_A_delay = rtr_map.matrix_A_delay
 err = write_same_dsp(sc.all_dsp, dsp_map.matrix_A_delay_ptr, addr_matrix_A_delay, /CHECK)

 if err ne adsec_error.ok then begin
   print, 'Error writing address of matrix A_delay'
   return, err
 endif


 ; setting matrix_K pointer to the matrix_K address
 addr_matrix_K = rtr_map.matrix_K
 err = write_same_dsp(sc.all_dsp, dsp_map.matrix_K_ptr, addr_matrix_K, /CHECK)

 if err ne adsec_error.ok then begin
   print, 'Error writing address of matrix_K'
   return, err
 endif


 ; setting matrix_M2C pointer to the matrix_M2C address
 addr_matrix_M2C = rtr_map.matrix_M2C
 err = write_same_dsp(sc.all_dsp, dsp_map.matrix_M2C_ptr, addr_matrix_M2C, /CHECK)

 if err ne adsec_error.ok then begin
   print, 'Error writing address of matrix M2C'
   return, err
 endif



return,adsec_error.ok

end
