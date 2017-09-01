; $Id: powerup.pro,v 1.1 2004/12/20 16:53:50 labot Exp $
;
;+
;
; POWERUP
;
; This function permits to enable th powering of the secondary and
; to switch it off.
;
; err = powerup(first_bcu, last_bcu, STATUS=status)
;
; INPUTS
;
;  first_bcu:          idl int type scalar.
;                    First DSP to which the data are sent.
;                If the BCU or SIGGEN keyword is set, in this field there is
;                the BCU number to read.
;
;  last_bcu :           idl int type scalar.
;                    Last DSP to which the data are sent.
;                If the BCU or SIGGEN keyword is set, this field is ignored.
;
;
; OUTPUTS
;
;  err:             long-int scalar. (see adsec_error in init_gvar.pro)
;
; KEYWORDS
;
;  STATUS:       status variable on output.
;
; HISTORY:
;
;     16 Dec 2004, written by Marco Xompero
;       <marco@arcetri.astro.it>
;-

Function powerup, bcu_num_tmp, STATUS=status, OFF=off

@adsec_common


;=================================================
; checks for the input parameters
;=================================================

;std definitions for testing
bcu_num = bcu_num_tmp
dsp_address = 0L

str="short int (IDL int)"

;test type of "first_dsp" variable
;check int type
if test_type(bcu_num, /INT, N_EL=n_el) then begin
    message, "first_bcu must be a "+str, CONT=(sc.debug eq 0B)
    return, adsec_error.input_type
endif
;check n elements (must be 1)
if n_el ne 1 then begin
    message, "first_bcu must be a scalar", CONT=(sc.debug eq 0B)
    return, adsec_error.input_type
endif
if bcu_num[0] ge adsec.n_bcu or bcu_num[0] lt 0 then begin
    message, "first_bcu must be a valid bcu number", CONT=(sc.debug eq 0B)
    return, adsec_error.input_type
endif

;set the codes for the bcu case
new_first = 255;
new_last = 255;
block_size=0
n_bcu = 1
flags = 1l 
words2send = 1L

;
;end of the checks
;=============================================================
;Begin of the real routine
;=============================================================

;calculating the number of dsp and board to read
timeout = long(1L * 1000d * dsp_const.mean_transfer_time * n_bcu *1.2) 
timeout = timeout > sc.min_timeout

dummy_set = ishft(1UL, 30)
first_relais = ishft(1UL,0)

if keyword_set(off) then mask=dummy_set else mask= (dummy_set or first_relais)

if sc.ll_debug then begin
    print, 'Debugging data'
    help, bcu_num
    help, new_first
    help, new_last
    help, timeout
    printhex, mask
    help, flags
    print, 'dsp address ', strtrim(string(dsp_address,format='(Z8.8)'),2)
    print, 'sc.debug ', sc.debug
endif

if sc.host_comm eq "Dummy" then begin
    err = adsec_error.ok
endif else begin

;Natural case and call external to .so
     err = CALL_EXTERNAL(sc.dll_name, 'powerup',              $
                    bcu_num, new_first, new_last, dsp_address, $
                       words2send, mask, timeout)
endelse

    if err ne adsec_error.ok then begin
        message, "WRITING ERROR BCU", CONT=(sc.debug eq 0B)
        return, err
    endif

return, err

end
