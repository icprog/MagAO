; $Id: proc_monitor.pro,v 1.3 2003/06/10 18:32:27 riccardi Exp $

pro proc_monitor, prev_sys_status

@adsec_common

max_abs_curr = 10000
max_pos_ptv = replicate(200,adsec.n_actuators)
max_pos_ptv[5] = 200
max_pos_jump = 100
max_curr_jump = 5000

do_rip = 0B

idx = where(sys_status.closed_loop, count)
if count ne 0 then begin
	idx_fail = where(sys_status.position_sd[idx] gt max_pos_ptv[idx], count_fail)
	if count_fail ne 0 then begin
		disp_mess, "The following actuators exceeded the PtV limit:"
		for i=0,count_fail-1 do $
			disp_mess, "Act. #"+strtrim(idx[idx_fail[i]],2)+" PtV="+strtrim(sys_status.position_sd[idx[idx_fail[i]]],2)+" counts",/APPEND
		do_rip=1B
	endif

	idx_fail = where(abs(sys_status.position[idx]-prev_sys_status.position[idx]) gt max_pos_jump, count_fail)
	if count_fail ne 0 then begin
		disp_mess, "The following actuators exceeded the position jump limit:"
		for i=0,count_fail-1 do $
			disp_mess, "Act. #"+strtrim(idx[idx_fail[i]],2)+" jump="+strtrim(sys_status.position[idx[idx_fail[i]]]-prev_sys_status.position[idx[idx_fail[i]]],2)+" counts",/APPEND
		do_rip=1B
	endif
endif

idx = adsec.act_w_curr
if count ne 0 then begin
	idx_fail = where(abs(sys_status.current[idx]) gt max_abs_curr, count_fail)
	if count_fail ne 0 then begin
		disp_mess, "The following actuators exceeded the current limit:"
		for i=0,count_fail-1 do $
			disp_mess, "Act. #"+strtrim(idx[idx_fail[i]],2)+" current="+strtrim(sys_status.current[idx[idx_fail[i]]],2)+" counts",/APPEND
		do_rip=1B
	endif

	idx_fail = where(abs(sys_status.current[idx]-prev_sys_status.current[idx]) gt max_pos_jump, count_fail)
	if count_fail ne 0 then begin
		disp_mess, "The following actuators exceeded the current jump limit:"
		for i=0,count_fail-1 do $
			disp_mess, "Act. #"+strtrim(idx[idx_fail[i]],2)+" jump="+strtrim(sys_status.current[idx[idx_fail[i]]]-prev_sys_status.current[idx[idx_fail[i]]],2)+" counts",/APPEND
		do_rip=1B
	endif
endif

if do_rip then begin
	disp_mess, "Monitoring stopped.", /APPEND
	disp_mess, "Setting the mirror in rest position...",/APPEND
	err = rip()
	disp_mess, "... done.",/APPEND
	sys_status.monitor = 0B
	sys_status.cont_update = 0B
	mult_beep, 5
	return
endif

end
