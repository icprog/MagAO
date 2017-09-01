; $Id: plot_step.pro,v 1.13 2009/04/10 13:55:41 labot Exp $
;
;   HISTORY:
;    Written by Armando Riccardi on 2003 (AR)
;
;   08 Feb 2005 Marco Xompero (MX)
;       Settling time, Rising time and overshoot print added.
;       Added true parameter.
;       January 2006 D.Zanotti(DZ)
;       Corrected the calculus of settling time.
;   26 Feb 2007 MX
;       SET_TIME keyword added to pass the settling time get by response.
;   13 Sep 2007 AR
;       OVERSHOOT and STATIC_ERROR keyword added, returning overshoot and static error ratio
;       (normalized to command step). Computation and print of normalized overshoot and static error.
;

pro plot_step, type, number, amp, times, trig, pr_cmd, pr_cur, response $
             , SMOOTH=smooth, MIN_T=min_t_range, MAX_T=max_t_range $
             , EXTRA_TIT=extra_title, ADSEC_SAVE=adsec_save $
             , OVERPLOT_LIST=oplot_list, POS_PSYM=pos_psym, MKS=mks $
             , NO_NEW_WINDOW=no_new_win, NO_DISPLAY_MODE=no_disp, _EXTRA=extra $
             , CURRENT=plot_curr, GAIN=gain, SAME_DISPLAY_CUTS=same_cuts $
             , SET_TIME=set_time, OVERSHOOT=overshoot, STATIC_ERROR=static_error, FILE=filename_base

@adsec_common

if n_elements(adsec_save) eq 0 then begin
	ff_f_svec = adsec.ff_f_svec
	ff_p_svec = adsec.ff_p_svec
	act_w_pos = adsec.act_w_pos
	gain_to_bbgain = adsec.gain_to_bbgain
       true = n_elements(adsec.true_act)
endif else begin
	ff_f_svec = adsec_save.ff_f_svec
	ff_p_svec = adsec_save.ff_p_svec
	act_w_pos = adsec_save.act_w_pos
       gain_to_bbgain = adsec_save.gain_to_bbgain
       true = n_elements(adsec_save.true_act)
endelse

n_op = n_elements(oplot_list)
xtit = 'Time [ms]'
if n_elements(pos_psym) eq 0 then pos_psym=-1
if n_elements(extra_title) eq 0 then extra_title=""
if ((!d.flags and 256) eq 256) and (not keyword_set(no_new_win)) then window,/free
time=(findgen(times)-trig)*adsec.sampling_time*1000
if pr_cmd eq 0 then max_t=256 else max_t=pr_cmd
if pr_cur gt 0 then max_t = min([max_t,pr_cur])
if n_elements(max_t_range) eq 0 then max_t_range = (256/max_t*adsec.sampling_time*1e3*2.5) > 3.0
if n_elements(min_t_range) eq 0 then min_t_range = -1.0

np = n_elements(response[*,0,0])

if keyword_set(plot_curr) then begin
	;; computes the bit_bit_gain of the loop
	idx0=trig+ceil(256.0/trig)+1
	pos = reform(response[idx0:*,0,*])
	cur = reform(response[idx0:*,1,*])
	com = reform(response[idx0:*,2,*])
	cur_cmd = intarr(np,adsec.n_actuators)

	bbgain = fltarr(adsec.n_actuators)
	for i_act=0,adsec.n_actuators-1 do begin
		if min(cur[*,i_act])-max(cur[*,i_act]) ne 0 then begin
			if n_elements(gain) eq 0 then begin
				aa = linfit(com[*,i_act]-pos[*,i_act], cur[*,i_act])
				bbgain[i_act] = -aa[1]
			endif else bbgain[i_act]=gain*gain_to_bbgain[i_act]

			cur_cmd[*,i_act] = response[*,1,i_act] + round(bbgain[i_act]*(response[*,2,i_act]-response[*,0,i_act]))
		endif else begin
			cur_cmd[*,i_act] = response[*,1,i_act]
		endelse
	endfor
endif

if type eq 'actuator' then begin
	if keyword_set(mks) then begin
		if keyword_set(plot_curr) then begin
			scale=adsec.N_per_ccount[number]*1e9
			YTIT = "Act. force [nN]"
		endif else begin
			scale=adsec.m_per_pcount[number]*1e9
			YTIT = "Act. displacement [nm]"
		endelse
	endif else begin
		scale=1
		if keyword_set(plot_curr) then begin
			YTIT = "Act. force [N]"
		endif else begin
			YTIT = "Act. displacement [m]"
		endelse
	endelse

	if keyword_set(plot_curr) then begin
		com = (cur_cmd[*,number]-cur_cmd[0,number])*scale
		pos = (response[*,1,number]-cur_cmd[0,number])*scale
	endif else begin
		com = (response[*,2,number]-response[0,2,number])*scale
		pos = (response[*,0,number]-rebin(response[0:trig-1,2,number],1))*scale
	endelse
	c0 = com[0]
	c1 = com[np-1]
        yrange = minmax([[pos],[com]]) 
    plot,time,pos,psym=pos_psym,xr=[min_t_range,max_t_range] $
        , SYMSIZE=0.5 ,tit=' Actuator #'+strtrim(number,2)+extra_title $
        , YTIT=ytit, XTIT=xtit, _EXTRA=extra,thick=1.5, charsize = 1.5, charthick=1.0, yr=yrange;, ytickf='(i6)'
    oplot,time, com, LINE=3

    window, /free   
    plot,time,pos,psym=pos_psym,xr=[min_t_range,max_t_range] $
        , SYMSIZE=0.5 ,tit=' Actuator #'+strtrim(number,2)+extra_title $
        , YTIT=ytit, XTIT=xtit, _EXTRA=extra,thick=1.5, charsize = 1.5, charthick=1.0, yr=minmax(pos);, ytickf='(i6)'
    oplot,time, com/(max(com))*mean(pos[np-200:np-1]), LINE=4, col=255l

    idst_act=where(pos gt (c0+1.1*(c1-c0)) or pos lt c0+0.9*(c1-c0), n_st)
    if n_st eq 0 then set_time = -1 else set_time = time(idst_act[n_st-1])-time[trig]

    maxypos= max(pos, id_tmax)
    max_ov_p = maxypos 
    max_ov_t=time[id_tmax]-time[trig]
    
    idrt_act=where(pos ge c0+0.9*(c1-c0), pp)
    if pp eq 0 or  idrt_act[0] eq 0 then rise_time = -1 else rise_time = time[idrt_act[0]-1]-time[trig]    

	dc = c1-c0
	overshoot = (max_ov_p-steady_pos)/steady_pos
	static_error = (steady_pos-dc)/dc
    print, 'actuator :', number
    print, 'Step command [m]:', dc
    print, 'Actuated step [m]:', steady_pos, 'Statical error:'+string(static_error*100,FORMAT="(f5.2)")+"%"
    if set_time eq -1 then print, 'Impossible to compute the settling time' else $
    print, 'Settling time [msec] : ', set_time
    print, 'Maximum Overshoot : ',max_ov_p,' [m] ('+strtrim(overshoot*100,2)+'%) at ',max_ov_t,'[msec]'
    if rise_time eq -1 then print, 'The step response does not reach the command ' else $
    print,  'Rising time [msec] :', rise_time


    if keyword_set(plot_curr) then begin
		if n_op ne 0 then for ii=0,n_op-1 do begin
			if keyword_set(mks) then scale=adsec.N_per_pcount[oplot_list[ii]]*1e9 else scale=1
			oplot,time,(response[*,1,oplot_list[ii]]-cur_cmd[0,oplot_list[ii]])*scale, LINE=2
		endfor
	endif else begin
	    oplot,minmax(time),replicate(c0+1.1*(c1-c0), 2), /LINE
	    oplot,minmax(time),replicate(c0+0.9*(c1-c0), 2), /LINE
		if n_op ne 0 then for ii=0,n_op-1 do begin
			if keyword_set(mks) then scale=adsec.m_per_pcount[oplot_list[ii]]*1e9 else scale=1
			oplot,time,(response[*,0,oplot_list[ii]]-response[0,2,oplot_list[ii]])*scale, LINE=2
		endfor
	endelse
endif else begin
	if keyword_set(mks) then begin
		if keyword_set(plot_curr) then begin
			scale=mean(adsec.N_per_ccount[adsec.act_w_curr])*1e9
			YTIT = "Modal current amp [nN rms]"
		endif else begin
			scale=mean(adsec.m_per_pcount[adsec.act_w_pos])*1e9
			YTIT = "Modal amp [nm rms]"
		endelse
	endif else begin
		if keyword_set(plot_curr) then begin
			scale=1
			YTIT = "Modal current amp [N rms]"
		endif else begin
			scale=1
			YTIT = "Modal amp [m rms]"
		endelse
	endelse


	if keyword_set(plot_curr) then begin
	    dummy_pos=(reform(response[*,1,*])-rebin(cur_cmd[0,*],np,adsec.n_actuators, /SAMP))*scale
	    mod_pos=transpose(ff_f_svec[number,*])##dummy_pos
	    dummy_com=(cur_cmd-rebin(cur_cmd[0,*],np,adsec.n_actuators, /SAMP))*scale
	    mod_com=transpose(ff_f_svec[number,*])##dummy_com
	endif else begin
	    dummy_pos=reform(response[*,0,*]-rebin(response[0,0,*],np,1,adsec.n_actuators))*scale
	    mod_pos=transpose(ff_p_svec[number,*])##dummy_pos
	    dummy_com=reform(response[*,2,*]-rebin(response[0,2,*],np,1,adsec.n_actuators))*scale
	    mod_com=transpose(ff_p_svec[number,*])##dummy_com
	endelse
    mod_pos=mod_pos/sqrt(true)     ;qui li cambio in metri!!!!!
    mod_com=mod_com/sqrt(true)     ;qui li cambio in metri!!!!!
	c0 = mod_com[0]
	c1 = mod_com[np-1]
    plot,time,mod_pos,psym=pos_psym,xr=[min_t_range,max_t_range] $
        ,tit='Mode #'+strtrim(number,2)+extra_title, YSTY=18 $
        , SYMSIZE=0.5, YTIT=ytit, XTIT=xtit, _EXTRA=extra,thick=1.5, charsize = 2.0, charthick=1.5;,BACK='FFFFFFFF'xul, col=0;MOM
    oplot, time, mod_com, LINE=3;, col=0
    

    idst=where(abs(mod_pos) gt abs(c0+1.1*(c1-c0)) or abs(mod_pos) lt abs(c0+0.9*(c1-c0)), n_st)
    if n_st eq 0 then set_time = -1 else set_time = time(idst[n_st-1])-time[trig]
    maxypos= max(mod_pos, id_tmax)
    max_ov_v = maxypos
    max_ov_t = time[id_tmax]-time[trig]
    idrt=where(abs(mod_pos) ge abs(c0+0.9*(c1-c0)), pp)
    if pp eq 0 or  idrt[0] eq 0 then rise_time = -1 else rise_time =time[idrt[0]-1]-time[trig]
    steady_pos=rebin(mod_pos[n_elements(mod_pos)-10:*],1)

    dc = c1-c0
    overshoot = (maxypos-steady_pos)/steady_pos
    static_error = (steady_pos-dc)/dc


    print, 'mode :', number
    print, 'Step command :', dc
    print, 'Actuated step [m]:', steady_pos, 'Statical error:'+string(static_error*100,FORMAT="(f5.2)")+"%"
    if set_time  eq -1 then print, 'Impossible to compute the settling time' else $
      print, 'Settling time [msec] : ', set_time
    print, 'Maximum Overshoot : ',maxypos,' [m] ('+strtrim(overshoot*100,2)+'%) at ',max_ov_t,'[msec]'

    if rise_time eq -1 then print, 'The step response does not reach the command ' else $
      print,  'Rising time [msec] :', rise_time

    if keyword_set(plot_curr) then begin
        if n_op ne 0 then for ii=0,n_op-1 do begin
            mod_pos=transpose(ff_f_svec[oplot_list[ii],*])##dummy_pos
            mod_pos=mod_pos/sqrt(true)
            mod_com=transpose(ff_f_svec[oplot_list[ii],*])##dummy_com
            mod_com=mod_com/sqrt(true)
            oplot,time,mod_pos, LINE=2
        endfor
    endif else begin
        oplot,minmax(time),replicate(c0+1.1*(c1-c0), 2), /LINE ;, col=0
        oplot,minmax(time),replicate(c0+0.9*(c1-c0), 2), /LINE ;, col=0
        if n_op ne 0 then for ii=0,n_op-1 do begin
            mod_pos=transpose(ff_p_svec[oplot_list[ii],*])##dummy_pos
            mod_pos=mod_pos/sqrt(true)
            mod_com=transpose(ff_p_svec[oplot_list[ii],*])##dummy_com
            mod_com=mod_com/sqrt(true)
            oplot,time,mod_pos, LINE=2
        endfor
    endelse

     save, file=filename_base+'_single_step.sav', time, mod_com, mod_pos, number, dc, steady_pos, set_time, maxypos, max_ov_t, rise_time, overshoot

endelse


if not keyword_set(no_disp) then begin
	display,response[times-1,0,act_w_pos]-response[0,0,act_w_pos] $
	        ,act_w_pos, /sh, SMOOTH=smooth, /NONUM, /NO_PLOT, POS=ima, OUT_VAL=0
	display,response[times-1,2,act_w_pos]-response[0,2,act_w_pos] $
	        ,act_w_pos, /sh, SMOOTH=smooth, /NONUM, /NO_PLOT, POS=ima_com, OUT_VAL=0
	px = !x.window * !d.x_vsize ;Get position of plot window in device units
	py = !y.window * !d.y_vsize
	swx = px(1)-px(0)       ;Size in x in device units
	swy = py(1)-py(0)       ;Size in y
	s=min([swx/4, swy/4])
       ima_com = ima-ima_com

	if (!d.flags and 1) then begin
		; device has scalable pixels
		if keyword_set(same_cuts) then begin
                    tvscl1, [[ima_com],[ima]], px[1]-s*1.1, py[0]+swy/2-s/2, XSIZE=s, YSIZE=2*s, /DEVICE
                endif else begin
                    tvscl1, ima, px[1]-s*1.1, py[0]+swy/2-s/2, XSIZE=s, YSIZE=s, /DEVICE
                    tvscl1, ima_com, px[1]-s*1.1, py[0]+swy/2-3*s/2, XSIZE=s, YSIZE=s, /DEVICE
                endelse
	endif else begin
		; device has not scalable pixels
		sima=(size(ima))[1]
              if sima gt s then begin
                  dd=fix(sima/s)
                  idd=dd
                  while(sima mod idd ne 0) do idd +=  1
                  ima=rebin(ima, sima/idd, sima/idd,/SAMP) 
                  ima_com=rebin(ima_com, sima/idd, sima/idd,/SAMP) 
              endif else begin
                  ima=rebin(ima, sima*round(s/sima), sima*round(s/sima),/SAMP)
                  ima_com=rebin(ima_com, sima*round(s/sima), sima*round(s/sima),/SAMP)
              endelse
              sima=(size(ima))[1]
              if keyword_set(same_cuts) then begin
                  tvscl1, [[ima_com],[ima]], px[1]-sima*1.1, py[0]+swy/2-s, /DEVICE
              endif else begin
                  tvscl1, ima, px[1]-sima*1.1, py[0]+swy/2, /DEVICE
                  tvscl1, ima_com, px[1]-sima*1.1, py[0]+swy/2-s, /DEVICE
              endelse
	endelse
endif
print,' Max abs current ', max(abs(response[*,1,*]), vv)
nmeas=n_elements(response[*,1,0])
print,' Max abs current ', (vv mod nmeas)
end
