; $Id: update_command.pro,v 1.6 2005/05/20 15:32:55 labot Exp $
;+
;
; UPDATE_COMMAND
;
; err = update_command([delay])
;
; HISTORY
;   27 May 2004, A. Riccardi
;    default delay casted to long for LBT format compatibility
;   02 Nov 2004, MX
;     Adsec.all changed in sc.all.
;   20 May 2005: Marco Xompero (MX)
;      Dsp_map updated to rtr_map.
;-
function update_command, delay

@adsec_common
if n_elements(delay) eq 0 then delay=1L
err = write_same_dsp(sc.all_dsp, rtr_map.update_cmd, delay)

return, err

end
