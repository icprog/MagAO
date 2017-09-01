
; Procedure to remove force peaks on the AdSec, using AR's depeak routine
;
; acts = list with the DSP indexes of an actuator group (minimum 2)
;        with opposing forces
;
; Written by:  AP  2016-11-18


pro remove_peak, acts, MAXCMD=MAXCMD, NMODES=NMODES
@adsec_common

 if not keyword_set(MAXCMD) then MAXCMD = 400e-9
 if not keyword_set(NMODES) then NMODES = 100

 err = update_status()
 curr = sys_status.current
 cmd = depeak( acts, curr, NMODES=NMODES)
 mm = minmax(cmd)

 if max(abs(cmd)) gt MAXCMD then begin
     print, 'Command too large: minmax=',minmax(cmd)
     return
 endif

 print, fsm_apply_cmd(cmd, /PASS, /DELTA, /WITH)
end
