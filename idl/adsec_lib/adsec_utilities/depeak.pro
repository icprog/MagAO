;
;
; err = update_status()
; curr =sys_status.current()
;
; act2ave = [481, 482]
; cmd = depeak(act2ave, curr, NMODES=100)
; display, cmd[adsec.act_w_cl], adsec.act_w_cl, /sh, /NO_NUM, /SMOOTH
; print, minmax(cmd)
; print, fsm_apply_cmd(cmd, /PASS, /DELTA, /WITH)
;
;
;
function depeak, act2ave, cur, NMODES=nmodes
@adsec_common

if n_elements(nmodes) eq 0 then nmodes = 3 
ave_acts = act2ave
wcl = adsec.act_w_cl

err = intersection(ave_acts, wcl, dummy)
if err then message, "actuators must be in the CL list"

df = fltarr(1,adsec.n_actuators)
df[ave_acts]=-(cur[ave_acts]-mean(cur[ave_acts]))

s = adsec.ff_sval
s[0:nmodes-1] = 0
idx = where(s ne 0)
invs = s*0
invs[idx]=1.0/s[idx]

ffplus = adsec.ff_p_svec ## diagonal_matrix(invs) ## transpose(adsec.ff_f_svec)

return, ffplus ## df
end


