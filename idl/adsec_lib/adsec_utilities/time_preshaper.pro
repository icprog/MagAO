; $Id: time_preshaper.pro,v 1.5 2007/12/03 10:30:20 labot Exp $
;+
;  HISTORY:
;   28 Jul 2004 D.Zanotti(DZ)
;     Data type adapted for LBT	
;   02 Nov 2004, MX
;     Adsec.all changed in sc.all.
;   01 Dec 2004, MX
;     New FF and POS keywords
;   11 Aug 2005, A. Riccardi (AR)
;     The function now uses the new step2time_preshaper routine
;
;-
function time_preshaper, ACT_LIST_TIME=act_list_time, FF=tff, POS=tpos 
@adsec_common
if n_elements(act_list_time) eq 0 then begin
	time_list = sc.all_actuators
endif else begin
	time_list = act_list_time
endelse

err = get_preshaper(time_list, step_pp)    ;read cmd preshaper
if err ne adsec_error.ok then return,err
err = get_preshaper(time_list,step_ff,/FF) ;read cur preshaper
if err ne adsec_error.ok then return,err

tpos = step2time_preshaper(step_pp)
tff = step2time_preshaper(step_ff)
t_preshaper = max([tpos,tff])

if sc.name_comm eq "Dummy" then begin
    return, 0.1
endif else begin
    return, t_preshaper
endelse
end




