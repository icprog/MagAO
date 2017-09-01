;$Id: reset_devices.pro,v 1.10 2009/04/10 13:52:15 labot Exp $$

;+
;	RESET_DEVICES
; 
; This function permits to reset all the boards or the dsp in each different part.
;
; err = reset_devices ([fist_dsp,last_dsp,] ALLCRATE=allcrate, ALLNOCOM=allnocom,
;							 BOARD=board, FPGA=fpga, FLASH=flash, DSP0=dsp0, DSP1=dsp1, 
;							 ETHERNET=ethernet)
;
; INPUTS
;	 
;	 first_dsp:					first dsp to reset (first board if it'is a memory
;									board based)
;	 last_dsp:					last dsp to reset (last board if id's a memory
;									board based)
;
; OUTPUTS
;
;   err:             long-int scalar. (see adsec_error in init_gvar.pro)
;
; KEYWORDS
;
;		ALLCRATE:				reset all crate (only in comm board).
;		ALLNOCOM:				reset all boards excluded the communication board
;									(only in comm board).
;		BOARD:					reset board (excluded the comm board).
;		FPGA:						reset only FPGA in the specified boards.
;		FLASH:					reset FLASH memory.
;                          This keyword is allowed only by a call of 
;                          FLASH_MANAGE function.
;		DSP0:						reset only the DSP#0 in the board.
;		DSP1:						reset only the DSP#1 in the board (excluded Sig Gen Board).
;		ETHERNET:				reset the Ethernet chip only in the comm board.
;		
;		All keyword generate a sequence of reset if the signal was operating.
;
; 		Note: The use of the ALLCRATE or ALLNOCOM or BOARD keywords causes 
;		the discarding of all sequent keywords setted.
;
;		BCU:						if you want to operate in the BCU communication board
;		SIGGEN:					if you want to operate in the Signal Generator Board
;       MULTI:                  if you want to operate on multi bcu, in this case the first_dsp is the first_bcu, the last_dsp is the last_bcu. 
;       TIMEOUT:                in the case multi bcu, it is possible introduce a timeout for the bcu reply.  
;
; HISTORY:
;
;      Wen Mar 18 18:07:21 2004, Marco Xompero
;       <marco@arcetri.astro.it>
;		04 May 2004, MX
;			debugging and variation SIGGEN address: from 0xFE to 0xFC.
;       03 April 2007, MX
;           Added some comments.
;       19 April Added timeout
;
;-

Function reset_devices, first_dsp_tmp, last_dsp_tmp, $
							ALLCRATE = allcrate, ALLNOCOM = allnocom, $
							BOARD = 	board, FPGA = fpga, FLASH = flash, $
							DSP0 = dsp0, DSP1 = dsp1, ETHERNET = ethernet, $
							BCU = bcu, SIGGEN = siggen, SWITCHB=switchb,   $
                            MULTI_BCU = multi_bcu, TIMEOUT = timeout 


@adsec_common
 if ~!AO_CONST.side_master then return, adsec_error.IDL_NOT_SIDE_MASTER

if keyword_set(SWITCHB) then begin
    first_dsp_tmp = adsec.n_bcu
    last_dsp_tmp = adsec.n_bcu
    bcu = 1
endif

;std definitions for testing
first_dsp = first_dsp_tmp
last_dsp = last_dsp_tmp
; checks ALLCRATE keyword and eventually unsets the others

mask = 0uL;

if keyword_set (ALLCRATE) then begin
	print, 'Warning: the keywords BOARD, FPGA, FLASH, DSP0, DSP1, ETHERNET, SIGGEN and BCU  will be ignored.';
	mask = mask + 2^1;
	allnocom = 0;
	board = 0;
	fpga = 0;
	flash = 0;
	dsp0 = 0;
	dsp1 = 0;
	ethernet = 0;
	siggen = 0;
    bcu=1;
endif

; checks ALLNOCOM keyword and eventually unsets the others
if keyword_set(ALLNOCOM) then begin
	print, 'Warning: the keywords BOARD, FPGA, FLASH, DSP0, DSP1, ETHERNET, SIGGEN and BCU will be ignored.';
	mask = mask + 2^3;
	allcrate = 0;
	board = 0;
	fpga = 0;
	flash = 0;
	dsp0 = 0;
	dsp1 = 0;
	siggen=0;
    bcu=1;

endif

str = 'short int!!!'

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
;check if first and last are congruent
if last_dsp lt first_dsp then begin
    message, "last_dsp cannot be less then the first_one", $
      CONT=(sc.debug eq 0B)
    return, adsec_error.input_type
endif

;check if the first and the last are in the same crate
if last_dsp / (adsec.n_board_per_BCU * adsec.n_dsp_per_board) ne $
	first_dsp / (adsec.n_board_per_BCU * adsec.n_dsp_per_board) then begin
    message, "dsps or boards cannnot be in different crates", $
      CONT=(sc.debug eq 0B)
    return, adsec_error.input_type
endif

if keyword_set(BCU) and keyword_set(SIGGEN) then begin
  	message, "The BCU keyword and the SIGGEN keyword cannot be both set!", CONT=(sc.debug eq 0B)
	return, adsec_error.input_type;
endif

if keyword_set(BCU) xor keyword_set(SIGGEN) then begin

;check if in first_dsp there is a correct bcu number
		if (first_dsp lt 0) or (first_dsp gt adsec.n_bcu) then begin
    		message, "The number of BCU in first_dsp is not correct!!!", CONT=(sc.debug eq 0B)
			return, adsec_error.input_type;
		endif
        
;set the codes for the bcu case
	if keyword_set(BCU) then begin 
		bcu_num = first_dsp; 
		new_first = 255;
		new_last = 255;
	    if keyword_set(MULTI_BCU) then begin
            bcu_num_last = last_dsp
            if (bcu_num_last lt bcu_num) or (bcu_num_last gt adsec.n_bcu) then begin
                message, "The number of BCU in last_dsp is not correct!!!", CONT=(sc.debug eq 0B)
                return, adsec_error.input_type;
            endif

        endif
    	
        if keyword_set(BOARD) then begin
			message, "The keyword BOARD is not permitted with BCU.", CONT=(sc.debug eq 0)
			return, adsec_error.input_type
		endif
		if keyword_set(DSP1) then begin
			message, "The keyword DSP1 is not permitted with BCU.", CONT=(sc.debug eq 0)
			return, adsec_error.input_type
		endif
	endif

;set the codes for the bcu case
	if keyword_set(SIGGEN) then begin
		bcu_num = first_dsp; 
		new_first = 252;
		new_last = 252;
		if keyword_set(ETHERNET) then begin
			message, "The keyword ETHERNET is not permitted with SIGGEN.", CONT=(sc.debug eq 0)
			return, adsec_error.input_type
		endif
		if keyword_set(DSP1) then begin
			message, "The keyword DSP1 is not permitted with SIGGEN.", CONT=(sc.debug eq 0)
			return, adsec_error.input_type
		endif
	endif

endif else begin

	bcu_num = last_dsp / (adsec.n_board_per_BCU * adsec.n_dsp_per_board)
	new_first = first_dsp mod (adsec.n_board_per_BCU * adsec.n_dsp_per_board)
	new_last = last_dsp mod (adsec.n_board_per_BCU * adsec.n_dsp_per_board)

endelse

if keyword_set(BOARD) then begin
	if (new_first eq 255) or (new_last eq 255) then begin
		message, "It's no possible to reset only the communication board.", $
			CONT = (sc.debug eq 0)
		return, adsec_error.input_type
	endif else begin
		mask = mask + 2^5;
		fpga = 0;
		flash = 0;
		dsp0 = 0;
		dsp1 = 0;
		ethernet = 0;
	endelse
endif


if keyword_set(FPGA) then mask = mask +  2^7;

if keyword_set(FLASH) then begin 
   help, calls=list_call
   if ((strmatch(list_call,'*flash_manage.pro*'))[1] ge 1) then begin
      mask = mask + 2^9 
   endif else begin
		message, "It's no possible to reset the flash mem by a direct call of reset_devices function. Use flash_manage function instead.", $
			CONT = (sc.debug eq 0)
		return, adsec_error.input_type
   endelse
endif

if keyword_set(DSP0) then mask = mask + 2^11;

if keyword_set(DSP1) then mask = mask + 2^13;

if keyword_set(ETHERNET) then begin
	if (new_first eq 255) and (new_last eq 255) then begin
	mask = mask + 2^15
	print, 'Resetting ethernet connection'
	endif else begin
	message, "It's no possible to reset the ethernet connection of a non-communication board", CONT = (sc.debug eq 0)
	return, adsec_error.input_type
	endelse
endif

if (mask eq 0) then begin
	message, "It's no possible to have a null mask. Please select some option!!", CONT = (sc.debug eq 0)
   return, adsec_error.input_type
endif
;==============================================================
;debug printing variables
dspAddress = 0L
datalength = 1L
if sc.debug then begin
print, 'Debugging data'
;help, /traceback
help, first_dsp
help, last_dsp
help, bcu_num
help, new_first
help, new_last
help, datalength
help, dspAddress
print, 'sc.debug ', sc.debug
print, 'Mask for reset ', mask
endif

;Dummy case
if sc.host_comm eq "Dummy" then begin
    err = adsec_error.ok
endif else begin
    if keyword_set(MULTI_BCU) then begin

        if keyword_set(TIMEOUT) then begin
            print, 'MULTI & TIMEOUT'
            err = reset_devices_multi_check_wrap(sc.mirrctrl_id[bcu_num], sc.mirrctrl_id[bcu_num_last], new_first, new_last, dspAddress, datalength, mask, timeout)
        endif else begin
			print, 'MULTI'
            err = reset_devices_multi_wrap(sc.mirrctrl_id[bcu_num], sc.mirrctrl_id[bcu_num_last], new_first, new_last, dspAddress, datalength, mask)
        endelse
    
    endif else begin
;Standard case
	err = reset_devices_wrap(sc.mirrctrl_id[bcu_num], new_first, new_last, dspAddress, datalength, mask)
	endelse
endelse

	;if err ne adsec_error.ok then begin
	;    message, "READING ERROR BCU", CONT=(sc.debug eq 0B)
	;    return, err
	;endif

return, adsec_error.ok

end

