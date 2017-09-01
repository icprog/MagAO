; $Id: clear_mem.pro,v 1.2 2004/05/04 14:52:30 labot Exp $$
;
;+
;
; CLEAR_MEM
;
; 	This function permits to easily clear all the memory type.
;  The default action is to clear the dsp memory.
;
; 	err = clear_mem(first_dsp, last_dsp, dsp_address, data_len,$
;                    SET_PM=set_pm, BCU=bcu, SIGGEN=siggen)
;
; INPUTS
;
;  first_dsp:  	   idl int type scalar.
;                    First dsp to clear.
;							If the BCU or SIGGEN keyword is set, in this field there is
;							the BCU number to clear.
;
;  last_dsp :    	   idl int type scalar.
;                    Last dsp to clear.
;							If the BCU or SIGGEN keyword is set, this field is ignored.
;
;  dsp_address:     	idl long int type scalar. 
;							Internal DSP address for the clear beginning.
;
;  data_len:         data length of mem to clear.
;
;
; OUTPUTS
;
;   err:             long-int scalar. (see adsec_error in init_gvar.pro)
;
; KEYWORDS
;
;   SET_PM:          if set the dsp_address specify an address of the
;                    program memory (PM), instead of the data memory (DM).
;							(must be specified)
;
;							/* Opcode definition
;									setPM = 0 (OL from IDL) -> DSP MEMORY
;									setPM = 1 (1L from IDL) -> EMPTY MEMORY 
;									setPM = 2 (1L from IDL) -> SDRAM MEMORY 
;									setPM = 3 (1L from IDL) -> SRAM MEMORY 
;									setPM = 4 (1L from IDL) -> FLASH MEMORY */
;
;	 BCU:					used to communucate with the bcu board. Cannot be set
;							with SIGGEN keyword.
;
;	 SIGGEN:				used to communicate with the Signal Generator board.
;							Cannot be set with the bcu keyword.
;
; HISTORY:
;
;	Tue 27 18:07:21 2004, Marco Xompero (MX)
;  Osservatorio Astrofisico di Arcetri, ITALY
;  <marco@arcetri.astro.it>
;		04 May 2004, MX
;			debugging and variation SIGGEN address: from 0xFE to 0xFC.
;
;-
;

function clear_mem, first_dsp_tmp, last_dsp_tmp, dsp_address_tmp, data_len_tmp,$
                   SET_PM=set_pm, BCU=bcu, SIGGEN=siggen

@adsec_common

;std definitions for testing
first_dsp = first_dsp_tmp
last_dsp = last_dsp_tmp
dsp_address = dsp_address_tmp
data_len = data_len_tmp

if n_elements(set_pm) eq 0 then set_pm=dsp_datasheet.data_mem_id
op_sel = byte(set_pm)
str="short int (IDL int)"
is_dsp = (op_sel eq dsp_datasheet.data_mem_id) or (op_sel eq dsp_datasheet.prog_mem_id)

;=================================================
; checks for the input parameters
;=================================================
;test type of "first_dsp" variable
;check int type
if test_type(first_dsp, /INT, N_EL=n_el) then begin
    message, "first_dsp must be a "+str, CONT=(sc.debug eq 0B)
    return, adsec_error.input_type
endif
;check n elements (must be 1)
if n_el ne 1 then begin
    message, "first_dsp must be a scalar", CONT=(sc.debug eq 0B)
    return, adsec_error.input_type
endif
;check it's a valid dsp number
if first_dsp[0] gt adsec.last_dsp then begin
    message, "first_dsp must be a valid dsp or board number", CONT=(sc.debug eq 0B)
    return, adsec_error.input_type
endif

;test type of "last_dsp" variable
;check int type
if test_type(last_dsp, /INT, N_EL=n_el) then begin
    message, "last_dsp must be a "+str, CONT=(sc.debug eq 0B)
    return, adsec_error.input_type
endif
;check n elements (must be 1)
if n_el ne 1 then begin
    message, "last_dsp must be a scalar", CONT=(sc.debug eq 0B)
    return, adsec_error.input_type
endif
;check it's a valid dsp number
if last_dsp[0] gt adsec.last_dsp then begin
    message, "last_dsp must be a valid dsp or board number", CONT=(sc.debug eq 0B)
    return, adsec_error.input_type
endif

;contitions based on the set_pm parameter: case board based or dsp based
if not is_dsp then begin 
	first_dsp = first_dsp * adsec.n_dsp_per_board;
	last_dsp = last_dsp * adsec.n_dsp_per_board;
endif

;check if first and last are congruent
if last_dsp lt first_dsp then begin
    message, "last_dsp cannot be less then the first_one", $
      CONT=(sc.debug eq 0B)
    return, adsec_error.input_type
endif

if last_dsp / (adsec.n_board_per_BCU * adsec.n_dsp_per_board) ne $
	first_dsp / (adsec.n_board_per_BCU * adsec.n_dsp_per_board) then begin
    message, "last_dsp and first_dsp are not in the same crate", $
      CONT=(sc.debug eq 0B)
    return, adsec_error.input_type
endif

;test the keywords
;check the bcu or the siggen keyword
if keyword_set(BCU) and keyword_set(SIGGEN) then begin
  	message, "The BCU keyword and the SIGGEN keyword cannot be both set!", CONT=(sc.debug eq 0B)
	return, adsec_error.input_type;
endif

if keyword_set(BCU) xor keyword_set(SIGGEN) then begin
		
;set the mem size for the board mem case
		mem_size = bcu_datasheet.flash_mem_size;
		
;check if in first_dsp there is a correct bcu number
		if (first_dsp lt 0) or (first_dsp gt adsec.n_bcu) then begin
    		message, "The number of BCU in first_dsp is not correct!!!", CONT=(sc.debug eq 0B)
			return, adsec_error.input_type;
		endif

;codes definition for the bcu case
	if keyword_set(BCU) then begin 
		bcu_num = first_dsp; 
		new_first = 255;
		new_last = 255;
      mem_size = get_mem_size(bcu_datasheet, bcu_datasheet.flash_mem_id)
	endif

;codes definition for the siggen case
	if keyword_set(SIGGEN) then begin
		bcu_num = first_dsp; 
		new_first = 252;
		new_last = 252;
      mem_size = get_mem_size(siggen_datasheet, siggen_datasheet.flash_mem_id)
	endif

endif else begin

	bcu_num = last_dsp / (adsec.n_board_per_BCU * adsec.n_dsp_per_board)
	new_first = first_dsp mod (adsec.n_board_per_BCU * adsec.n_dsp_per_board)
	new_last = last_dsp mod (adsec.n_board_per_BCU * adsec.n_dsp_per_board)
   mem_size = get_mem_size(dsp_datasheet, dsp_datasheet.flash_mem_id)

endelse

;test type of "dsp_address" variable
if test_type(dsp_address, /LONG, N_EL=n_el) then begin
    message, "dsp_address must be a LONG int", CONT=(sc.debug eq 0B)
    return, adsec_error.input_type
endif

;check n elements (must be 1)
if n_el ne 1 then begin
    message, "dsp_address must be a scalar", CONT=(sc.debug eq 0B)
    return, adsec_error.input_type
endif

;check it's a valid mem address
if dsp_address[0] lt 0 or $
  dsp_address[0] ge mem_size then begin
    message, "dsp_address is out of range", CONT=(sc.debug eq 0B)
    return, adsec_error.input_type
endif

if test_type(data_len, /LONG, N_EL=n_el) then begin
   message, "data_len must be a long-int", CONT=(sc.debug eq 0B)
   return, adsec_error.input_type
endif
;check n elements (must be 1)
if n_el ne 1 then begin
   message, "data_len must be a scalar", CONT=(sc.debug eq 0B)
   return, adsec_error.input_type
endif
;check it's a valid reasonable length
if data_len[0] le 0 or $
   long(dsp_address[0])+data_len[0]-1 ge mem_size then begin
   message, "requested to read outside the DSP memory limit", CONT=(sc.debug eq 0B)
   return, adsec_error.input_type
endif

;;===============================================
;;============ end of the checks ================
;;===============================================

;=============================================================
;Begin of the real routine 
;=============================================================



if sc.debug then begin
	print, 'Debugging data'
	help, /traceback
	help, first_dsp
	help, last_dsp
	help, bcu_num
	help, new_first
	help, new_last
	if n_elements(data_len) ne 0 then help, data_len 
	help, dsp_address
	print, 'sc.debug ', sc.debug
	print, 'Size of mem ', mem_size
	print, 'Buffers Dimensions ', size(send_data_buffer, /dim)
endif

if sc.host_comm eq "Dummy" then begin
   err = adsec_error.ok
endif else begin

   ;Natural case and call external to .so
   err = CALL_EXTERNAL(sc.dll_name, 'clear_mem',               $
								bcu_num, new_first, new_last, dsp_address,$
		 						1L, data_len, op_sel)
   if err ne adsec_error.ok then begin
      message, "CLEAR MEM ERROR BCU", CONT=(sc.debug eq 0B)
      return, err
   endif

   return, err

endelse

end

