;$Id: ff2bias.pro,v 1.6 2007/02/07 17:21:41 marco Exp $
;+
;   NAME:
;    FF2BIAS
;
;   PURPOSE:
;    The routine clear the feed forward current (ff_ud_current.ch0) and sets up the
;    the bias current (currend_ch0) ad sum of initial bias current and ff_current.
;
;   USAGE:
;    err = ff2bias()
;
;   INPUT:
;    None.
;
;   OUTPUT:
;    Error code.
;
;   KEYWORD:
;    None.
;
;   NOTE:
;    None.
;   
; HISTORY
;   28 May 2004 Armando Riccardi (AR)
;    Completely rewritten to match the LBT format.
;   22 Jul 2004 Marco Xompero (MX)
;    Adsec.ff2bias_step used, read_seq_ch routine calling and error code checks fixed.
;   02 Nov 2004, MX
;     Adsec.all changed in sc.all.
;   07 Feb 2007 MX
;       LUT of variable deleted and name refurbished.
;-

Function ff2bias

@adsec_common

; bias current ramp step number
nsteps = adsec.ff2bias_step

;reads all the ff currents
err = read_seq_ch(sc.all_actuators,dsp_map.ff_ud_current,1L,ff_current)
if err ne adsec_error.ok then begin
    message, "Error reading Feed Forward current (dsp_map.ff_ud_current).", CONT= (sc.debug eq 0B)
    return, err
endif

;reads all the bias currents
err = read_seq_ch(sc.all_actuators,dsp_map.bias_current,1L,bias_current)
if err ne adsec_error.ok then begin
    message, "Error reading Bias current (dsp_map.bias_current).", CONT= (sc.debug eq 0B)
    return, err
endif

;writes a new set of bias currents
ini_bias_current = bias_current
fin_bias_current = bias_current + ff_current
delta_bias = fin_bias_current - ini_bias_current
ramp = findgen(nsteps) / (nsteps-1)

;clears the ff forces
err = clear_ff()
if err ne adsec_error.ok then return,err

;ramps up the bias to the fin_bias
for i=0,nsteps-1 do begin
	new_bias_current = ini_bias_current + ramp[i] * delta_bias
	err = write_bias_curr(sc.all_actuators,new_bias_current)
	if err ne adsec_error.ok then begin
        message, "Error writing new bias current (dsp_map.bias_current).", CONT= (sc.debug eq 0B)
        return, err
    endif
endfor

return,adsec_error.ok

end
