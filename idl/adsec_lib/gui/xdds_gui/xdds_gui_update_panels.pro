pro xdds_gui_update_panels, event

	@adsec_common

	common xdds_gui_block, frame, slope, dm, xdds_data
	common xdds_gui_buffer_block, frame_cube2ave, offload_cube2ave, coeff_cube2ave $
	                            , pos_cube2ave, curr_cube2ave, comm_cube2ave, minmax_cube2ave
	common xdds_gui_last_frame, last_frame
	common seed_block, seed

    fstr="(F12.3)"
    dist_scale = 1e6 ;1e9  ;display unit convertion factor for position/commands
    ddist_scale = 1e9 ;display unit convertion factor for delta-positions
    curr_scale = 1.0;1e3  ;display unit conversion factor for forces
	err=0L
	debug = 0B

       xdds_gui_refresh_time_id=widget_info(Event.top, FIND_BY_UNAME="REFRESH_TIME_FLD")
       widget_control, xdds_gui_refresh_time_id, GET_VALUE=updating_time

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; RIGA DI DEBUG PRIMA DI IMPLEMENTARE IL TIMEOUT CORRETTAMENTE NELLA
;; SHARED
updating_time = 3600.0
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
       err = 5001
       while (err eq 5001) or (err eq 3506) do err = get_fast_diagnostic(dm, EMPTY=debug, TIMEOUT=(float(updating_time[0]) > 0.1))

	if err ne 0 then begin
		beep
		message, "Error reading diagnostic data", CONT=sc.debug eq 0B
              print, "Errore: ",err
		return
	endif
   	if debug then begin
    	dm.Frame_Number      = 0UL
        dm.DistAverage       = float(randomu(seed,adsec.n_actuators)*10e-6)
        dm.CurrAverage       = float(randomu(seed,adsec.n_actuators)*1.2)
        dm.DistAverageDouble = double(randomu(seed,adsec.n_actuators)*10e-6)
        dm.CurrAverageDouble = double(randomu(seed,adsec.n_actuators)*1.2)
        dm.Modes             = float(randomn(seed,adsec.n_actuators)*10e-6)
        dm.NewDeltaCommand   = float(randomn(seed,adsec.n_actuators)*0.1e-6)
        dm.FFCommand         = float(randomn(seed,adsec.n_actuators)*0.1e-6)
        dm.IntControlCurrent = float(randomu(seed,adsec.n_actuators)*0.01)
        dm.FFPureCurrent     = float(randomu(seed,adsec.n_actuators)*0.1)
        dm.DistRMS           = double(randomn(seed,adsec.n_actuators)*10e-9)
        dm.CurrRMS           = double(randomn(seed,adsec.n_actuators)*1e-3)
	endif


	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; position display
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

	xdds_data.position=dm.DistAverage*dist_scale

	num_set_to_ave_id=widget_info(Event.top, FIND_BY_UNAME="POS_NUM_SET_TO_AVE")
	widget_control, num_set_to_ave_id, GET_VALUE=n_pos2ave
	if xdds_data.n_pos2ave ne n_pos2ave then begin
		pos_cube2ave = findgen(adsec.n_actuators,n_pos2ave)
		xdds_data.pos_counter = 0
		xdds_data.pos_max_idx = 0
		xdds_data.n_pos2ave=n_pos2ave
	endif
	if n_pos2ave gt 1 then begin
		pos_cube2ave[*,xdds_data.pos_counter]=xdds_data.position
		if xdds_data.pos_max_idx gt 0 then $
			xdds_data.ave_position=total(pos_cube2ave[*,0:xdds_data.pos_max_idx], 2)/(xdds_data.pos_max_idx+1)
		xdds_data.pos_counter = (xdds_data.pos_counter+1) mod n_pos2ave
		xdds_data.pos_max_idx = xdds_data.pos_counter > xdds_data.pos_max_idx
	endif else begin
		xdds_data.ave_position=xdds_data.position
	endelse
	if xdds_data.pos_show_ave then pos=xdds_data.ave_position else pos=xdds_data.position

	if xdds_data.pos_remove_offset then pos = pos-xdds_data.pos_offset

	draw_id=widget_info(Event.top, FIND_BY_UNAME="POS_DRAW")
	widget_control, draw_id, GET_VALUE=win_num
	geom = widget_info(draw_id, /GEOMETRY)
	xs = fix(geom.scr_xsize) & ys = fix(geom.scr_ysize)
	min_cbar_id=widget_info(Event.top, FIND_BY_UNAME="POS_MIN_CBAR_FLD")
	widget_control, min_cbar_id, GET_VALUE=min_cbar
	max_cbar_id=widget_info(Event.top, FIND_BY_UNAME="POS_MAX_CBAR_FLD")
	widget_control, max_cbar_id, GET_VALUE=max_cbar
	cuts = float([min_cbar[0], max_cbar[0]])

	ima = bytarr(xs, ys)
	xdds_gui_draw_rings, ima, pos2color(pos, CUTS=cuts) $
		, LIST=adsec.act_w_pos
	wset, win_num
	tv1, ima
	wset, -1

	minv = min(pos[adsec.act_w_pos], MAX=maxv)
	fld_id=widget_info(Event.top, FIND_BY_UNAME="POS_MAX_FLD")
	widget_control, fld_id, SET_VALUE=string(maxv, FORMAT=fstr)
	fld_id=widget_info(Event.top, FIND_BY_UNAME="POS_MIN_FLD")
	widget_control, fld_id, SET_VALUE=string(minv,FORMAT=fstr)

	if xdds_data.modal_decomp_idx eq 1 and xdds_data.pos_remove_offset then begin
		draw_id=widget_info(Event.top, FIND_BY_UNAME="MODAL_DECOMP_DRAW")
		widget_control, draw_id, GET_VALUE=win_num

		modal_decomp_yr_id=widget_info(Event.top, FIND_BY_UNAME="MODAL_DECOMP_YRANGE_SLIDE")
		widget_control, modal_decomp_yr_id, GET_VALUE=modal_decomp_yrange

		wset, win_num
		plot, indgen(xdds_data.n_zern_modes)+1 $
		    , xdds_data.pos2zern_w_pos ## transpose(adsec.m_per_pcount*pos) $
		    , PSYM=10, YR=[-1,1]*modal_decomp_yrange, YTIT="[nm]"
		wset, -1
	endif

	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; current display
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

	xdds_data.current=dm.CurrAverage*curr_scale

	num_set_to_ave_id=widget_info(Event.top, FIND_BY_UNAME="CURR_NUM_SET_TO_AVE")
	widget_control, num_set_to_ave_id, GET_VALUE=n_curr2ave
	if xdds_data.n_curr2ave ne n_curr2ave then begin
		curr_cube2ave = findgen(adsec.n_actuators,n_curr2ave)
		xdds_data.curr_counter = 0
		xdds_data.curr_max_idx = 0
		xdds_data.n_curr2ave=n_curr2ave
	endif
	if n_curr2ave gt 1 then begin
		curr_cube2ave[*,xdds_data.curr_counter]=xdds_data.current
		if xdds_data.curr_max_idx gt 0 then $
			xdds_data.ave_current=total(curr_cube2ave[*,0:xdds_data.curr_max_idx], 2)/(xdds_data.curr_max_idx+1)
		xdds_data.curr_counter = (xdds_data.curr_counter+1) mod n_curr2ave
		xdds_data.curr_max_idx = xdds_data.curr_counter > xdds_data.curr_max_idx
	endif else begin
		xdds_data.ave_current=xdds_data.current
	endelse
	if xdds_data.curr_show_ave then curr=xdds_data.ave_current else curr=xdds_data.current

    if xdds_data.curr_remove_offset then curr = curr-xdds_data.curr_offset

	draw_id=widget_info(Event.top, FIND_BY_UNAME="CURR_DRAW")
	widget_control, draw_id, GET_VALUE=win_num
	geom = widget_info(draw_id, /GEOMETRY)
	xs = fix(geom.scr_xsize) & ys = fix(geom.scr_ysize)
	min_cbar_id=widget_info(Event.top, FIND_BY_UNAME="CURR_MIN_CBAR_FLD")
	widget_control, min_cbar_id, GET_VALUE=min_cbar
	max_cbar_id=widget_info(Event.top, FIND_BY_UNAME="CURR_MAX_CBAR_FLD")
	widget_control, max_cbar_id, GET_VALUE=max_cbar
	cuts = float([min_cbar[0], max_cbar[0]])

	ima = bytarr(xs, ys)
	xdds_gui_draw_rings, ima, curr2color(curr, CUTS=cuts) $
		, LIST=adsec.act_w_curr
	wset, win_num
	tv1, ima
	wset, -1

	minv = min(curr[adsec.act_w_curr], MAX=maxv)
	fld_id=widget_info(Event.top, FIND_BY_UNAME="CURR_MAX_FLD")
	widget_control, fld_id, SET_VALUE=string(maxv,FORMAT=fstr)
	fld_id=widget_info(Event.top, FIND_BY_UNAME="CURR_MIN_FLD")
	widget_control, fld_id, SET_VALUE=string(minv,FORMAT=fstr)

	if xdds_data.modal_decomp_idx eq 2 then begin
		; modal decomposition of current
		draw_id=widget_info(Event.top, FIND_BY_UNAME="MODAL_DECOMP_DRAW")
		widget_control, draw_id, GET_VALUE=win_num

		modal_decomp_yr_id=widget_info(Event.top, FIND_BY_UNAME="MODAL_DECOMP_YRANGE_SLIDE")
		widget_control, modal_decomp_yr_id, GET_VALUE=modal_decomp_yrange

		wset, win_num
		plot, indgen(xdds_data.n_zern_modes)+1 $
		    , xdds_data.curr2zern ## transpose((adsec.N_per_ccount*curr)) $
		    , PSYM=10, YR=[-1,1]*modal_decomp_yrange, YTIT="[mN]"
		wset, -1
	endif


	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; command display
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

	xdds_data.command = dm.NewDeltaCommand*ddist_scale

    if xdds_data.comm_remove_offset then begin
    	comm = xdds_data.command-xdds_data.comm_offset

		zern_coeff = xdds_data.pos2zern_w_cl[*,1:3] ## transpose(comm*adsec.m_per_pcount)

		n_offload2ave_id=widget_info(Event.top, FIND_BY_UNAME="OFFLOAD_AVE_SLIDER")
		widget_control, n_offload2ave_id, GET_VALUE=n_offload2ave
		if xdds_data.n_offload2ave ne n_offload2ave then begin
			offload_cube2ave = findgen(3,n_offload2ave)
			xdds_data.offload_counter = 0
			xdds_data.offload_max_idx = 0
			xdds_data.n_offload2ave=n_offload2ave
		endif
		if n_offload2ave gt 1 then begin
			offload_cube2ave[*,xdds_data.offload_counter]=zern_coeff
			if xdds_data.offload_max_idx gt 0 then $
				zern_coeff=total(offload_cube2ave[*,0:xdds_data.offload_max_idx], 2)/(xdds_data.offload_max_idx+1)
			xdds_data.offload_counter = (xdds_data.offload_counter+1) mod n_offload2ave
			xdds_data.offload_max_idx = xdds_data.offload_counter > xdds_data.offload_max_idx
		endif

		F1=(adsec_shell.curv_radius/2)/(2*adsec_shell.out_radius)
		F2=15.0
		longitudinal_defocus = 2*(2*sqrt(3.0)*zern_coeff[2])*(8.0*F2^2) ;; z axis from secondary to primary
		hexapode_delta_z = (F1/F2)^2*longitudinal_defocus


		elev_tilt = -2*(2*zern_coeff[1])/(6.5/2)*206265.0 ; elevation [arcsec]
		az_tilt   = -2*(2*zern_coeff[0])/(6.5/2)*206265.0 ; azimuth [arcsec]

		az_tilt_id=widget_info(Event.top, FIND_BY_UNAME="DELTA_AZIMUTH_FLD")
		widget_control, az_tilt_id, SET_VALUE=string(az_tilt, FORMAT="(F6.2)")
		elev_tilt_id=widget_info(Event.top, FIND_BY_UNAME="DELTA_ELEVATION_FLD")
		widget_control, elev_tilt_id, SET_VALUE=string(elev_tilt, FORMAT="(F6.2)")
		elev_tilt_id=widget_info(Event.top, FIND_BY_UNAME="DELTA_FOCUS_FLD")
		widget_control, elev_tilt_id, SET_VALUE=string(hexapode_delta_z*1e6, FORMAT="(F7.2)")
	endif else begin
		az_tilt_id=widget_info(Event.top, FIND_BY_UNAME="DELTA_AZIMUTH_FLD")
		widget_control, az_tilt_id, SET_VALUE=strtrim(0.0, 2)
		elev_tilt_id=widget_info(Event.top, FIND_BY_UNAME="DELTA_ELEVATION_FLD")
		widget_control, elev_tilt_id, SET_VALUE=strtrim(0.0, 2)
		elev_tilt_id=widget_info(Event.top, FIND_BY_UNAME="DELTA_FOCUS_FLD")
		widget_control, elev_tilt_id, SET_VALUE=string(0.0, FORMAT="(F7.2)")
	endelse

	num_set_to_ave_id=widget_info(Event.top, FIND_BY_UNAME="COMM_NUM_SET_TO_AVE")
	widget_control, num_set_to_ave_id, GET_VALUE=n_comm2ave
	if xdds_data.n_comm2ave ne n_comm2ave then begin
		comm_cube2ave = findgen(adsec.n_actuators,n_comm2ave)
		xdds_data.comm_counter = 0
		xdds_data.comm_max_idx = 0
		xdds_data.n_comm2ave=n_comm2ave
	endif
	if n_comm2ave gt 1 then begin
		comm_cube2ave[*,xdds_data.comm_counter]=xdds_data.command
		if xdds_data.comm_max_idx gt 0 then $
			xdds_data.ave_command=total(comm_cube2ave[*,0:xdds_data.comm_max_idx], 2)/(xdds_data.comm_max_idx+1)
		xdds_data.comm_counter = (xdds_data.comm_counter+1) mod n_comm2ave
		xdds_data.comm_max_idx = xdds_data.comm_counter > xdds_data.comm_max_idx
	endif else begin
		xdds_data.ave_command=xdds_data.command
	endelse
	if xdds_data.comm_show_ave then comm=xdds_data.ave_command else comm=xdds_data.command

    if xdds_data.comm_remove_offset then comm = comm-xdds_data.comm_offset

	draw_id=widget_info(Event.top, FIND_BY_UNAME="COMM_DRAW")
	widget_control, draw_id, GET_VALUE=win_num
	geom = widget_info(draw_id, /GEOMETRY)
	xs = fix(geom.scr_xsize) & ys = fix(geom.scr_ysize)
	min_cbar_id=widget_info(Event.top, FIND_BY_UNAME="COMM_MIN_CBAR_FLD")
	widget_control, min_cbar_id, GET_VALUE=min_cbar
	max_cbar_id=widget_info(Event.top, FIND_BY_UNAME="COMM_MAX_CBAR_FLD")
	widget_control, max_cbar_id, GET_VALUE=max_cbar
	cuts = float([min_cbar[0], max_cbar[0]])

	ima = bytarr(xs, ys)
	xdds_gui_draw_rings, ima, pos2color(comm, CUTS=cuts) $
		, LIST=adsec.act_w_cl
	wset, win_num
	tv1, ima
	wset, -1

	minv = min(comm[adsec.act_w_cl], MAX=maxv)
	fld_id=widget_info(Event.top, FIND_BY_UNAME="COMM_MAX_FLD")
	widget_control, fld_id, SET_VALUE=string(maxv,FORMAT=fstr)
	fld_id=widget_info(Event.top, FIND_BY_UNAME="COMM_MIN_FLD")
	widget_control, fld_id, SET_VALUE=string(minv,FORMAT=fstr)

	if xdds_data.modal_decomp_idx eq 3 and xdds_data.comm_remove_offset then begin
		; Modal decomposition of command
		draw_id=widget_info(Event.top, FIND_BY_UNAME="MODAL_DECOMP_DRAW")
		widget_control, draw_id, GET_VALUE=win_num

		modal_decomp_yr_id=widget_info(Event.top, FIND_BY_UNAME="MODAL_DECOMP_YRANGE_SLIDE")
		widget_control, modal_decomp_yr_id, GET_VALUE=modal_decomp_yrange

		wset, win_num
		plot, indgen(xdds_data.n_zern_modes)+1 $
		    , xdds_data.pos2zern_w_cl ## transpose(adsec.m_per_pcount*comm) $
		    , PSYM=10, YR=[-1,1]*modal_decomp_yrange, YTIT="[nm]"
		wset, -1
	endif


	;;;;;;;;;;;;;;;;;;;;;;;;;;
	; minmax display
	;;;;;;;;;;;;;;;;;;;;;;;;;;

	if xdds_data.display_minmax then begin
    	; max-min display

		xdds_data.minmax=dm.DistRMS*ddist_scale

		num_set_to_ave_id=widget_info(Event.top, FIND_BY_UNAME="MINMAX_NUM_SET_TO_AVE")
		widget_control, num_set_to_ave_id, GET_VALUE=n_minmax2ave
		if xdds_data.n_minmax2ave ne n_minmax2ave then begin
			minmax_cube2ave = findgen(adsec.n_actuators,n_minmax2ave)
			xdds_data.minmax_counter = 0
			xdds_data.minmax_max_idx = 0
			xdds_data.n_minmax2ave=n_minmax2ave
		endif
		if n_minmax2ave gt 1 then begin
			minmax_cube2ave[*,xdds_data.minmax_counter]=xdds_data.minmax
			if xdds_data.minmax_max_idx gt 0 then $
				xdds_data.ave_minmax=total(minmax_cube2ave[*,0:xdds_data.minmax_max_idx], 2)/(xdds_data.minmax_max_idx+1)
			xdds_data.minmax_counter = (xdds_data.minmax_counter+1) mod n_minmax2ave
			xdds_data.minmax_max_idx = xdds_data.minmax_counter > xdds_data.minmax_max_idx
		endif else begin
			xdds_data.ave_minmax=xdds_data.minmax
		endelse
		if xdds_data.minmax_show_ave then minmax=xdds_data.ave_minmax else minmax=xdds_data.minmax
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

		draw_id=widget_info(Event.top, FIND_BY_UNAME="MINMAX_DRAW")
		widget_control, draw_id, GET_VALUE=win_num
		geom = widget_info(draw_id, /GEOMETRY)
		xs = fix(geom.scr_xsize) & ys = fix(geom.scr_ysize)
		min_cbar_id=widget_info(Event.top, FIND_BY_UNAME="MINMAX_MIN_CBAR_FLD")
		widget_control, min_cbar_id, GET_VALUE=min_cbar
		max_cbar_id=widget_info(Event.top, FIND_BY_UNAME="MINMAX_MAX_CBAR_FLD")
		widget_control, max_cbar_id, GET_VALUE=max_cbar
		cuts = float([min_cbar[0], max_cbar[0]])

		ima = bytarr(xs, ys)
		xdds_gui_draw_rings, ima, pos2color(minmax, CUTS=cuts) $
			, LIST=adsec.act_w_pos
		wset, win_num
		tv1, ima
		wset, -1

		minv = min(minmax[adsec.act_w_pos], MAX=maxv)
		fld_id=widget_info(Event.top, FIND_BY_UNAME="MINMAX_MAX_FLD")
		widget_control, fld_id, SET_VALUE=string(maxv,FORMAT=fstr)
		fld_id=widget_info(Event.top, FIND_BY_UNAME="MINMAX_MIN_FLD")
		widget_control, fld_id, SET_VALUE=string(minv,FORMAT=fstr)
	endif





	if not xdds_data.display_minmax then begin
		; reconstructed mirror shape display
		xdds_data.rec_dpos=dm.FFCommand*ddist_scale

		num_set_to_ave_id=widget_info(Event.top, FIND_BY_UNAME="MINMAX_NUM_SET_TO_AVE")
		widget_control, num_set_to_ave_id, GET_VALUE=n_minmax2ave
		if xdds_data.n_minmax2ave ne n_minmax2ave then begin
			minmax_cube2ave = findgen(adsec.n_actuators,n_minmax2ave)
			xdds_data.minmax_counter = 0
			xdds_data.minmax_max_idx = 0
			xdds_data.n_minmax2ave=n_minmax2ave
		endif
		if n_minmax2ave gt 1 then begin
			minmax_cube2ave[*,xdds_data.minmax_counter]=xdds_data.rec_dpos
			if xdds_data.minmax_max_idx gt 0 then $
				xdds_data.ave_rec_dpos=total(minmax_cube2ave[*,0:xdds_data.minmax_max_idx], 2)/(xdds_data.minmax_max_idx+1)
			xdds_data.minmax_counter = (xdds_data.minmax_counter+1) mod n_minmax2ave
			xdds_data.minmax_max_idx = xdds_data.minmax_counter > xdds_data.minmax_max_idx
		endif else begin
			xdds_data.ave_rec_dpos=xdds_data.rec_dpos
		endelse
		if xdds_data.minmax_show_ave then rec_dpos=xdds_data.ave_rec_dpos else rec_dpos=xdds_data.rec_dpos


		draw_id=widget_info(Event.top, FIND_BY_UNAME="MINMAX_DRAW")
		widget_control, draw_id, GET_VALUE=win_num
		geom = widget_info(draw_id, /GEOMETRY)
		xs = fix(geom.scr_xsize) & ys = fix(geom.scr_ysize)
		min_cbar_id=widget_info(Event.top, FIND_BY_UNAME="MINMAX_MIN_CBAR_FLD")
		widget_control, min_cbar_id, GET_VALUE=min_cbar
		max_cbar_id=widget_info(Event.top, FIND_BY_UNAME="MINMAX_MAX_CBAR_FLD")
		widget_control, max_cbar_id, GET_VALUE=max_cbar
		cuts = float([min_cbar[0], max_cbar[0]])

		ima = bytarr(xs, ys)

		xdds_gui_draw_rings, ima, pos2color(rec_dpos, CUTS=cuts) $
			, LIST=adsec.act_w_pos
		wset, win_num
		tv1, ima
		wset, -1


		minv = min(rec_dpos[adsec.act_w_pos], MAX=maxv)
		fld_id=widget_info(Event.top, FIND_BY_UNAME="MINMAX_MAX_FLD")
		widget_control, fld_id, SET_VALUE=string(maxv,FORMAT=fstr)
		fld_id=widget_info(Event.top, FIND_BY_UNAME="MINMAX_MIN_FLD")
		widget_control, fld_id, SET_VALUE=string(minv,FORMAT=fstr)

		if xdds_data.modal_decomp_idx eq 4 then begin
			draw_id=widget_info(Event.top, FIND_BY_UNAME="MODAL_DECOMP_DRAW")
			widget_control, draw_id, GET_VALUE=win_num

			modal_decomp_yr_id=widget_info(Event.top, FIND_BY_UNAME="MODAL_DECOMP_YRANGE_SLIDE")
			widget_control, modal_decomp_yr_id, GET_VALUE=modal_decomp_yrange

			wset, win_num
			plot, indgen(xdds_data.n_zern_modes)+1 $
			    , xdds_data.pos2zern_w_cl ## (transpose(adsec.m_per_pcount)*rec_dpos) $
			    , PSYM=10, YR=[-1,1]*modal_decomp_yrange, YTIT="nm"
			wset, -1
		endif

	endif
end
