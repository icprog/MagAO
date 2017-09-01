;
; IDL Event Callback Procedures
; xol_gui_eventcb
;
; Generated on:	11/16/2002 00:43.41
;
;-----------------------------------------------------------------
; Text Insert String Callback Procedure.
; Argument:
;   Event structure:
;
;   {WIDGET_TEXT_STR, ID:0L, TOP:0L, HANDLER:0L, TYPE:1, OFFSET:0L,
;       STR:'' }

;   ID is the widget ID of the component generating the event. TOP is
;       the widget ID of the top level widget containing ID. HANDLER
;       contains the widget ID of the widget associated with the
;       handler routine.

;   OFFSET is the (zero-based) insertion position that will result
;       after the character is inserted. STR is the string to be
;       inserted.

;   Retrieve the IDs of other widgets in the widget hierarchy using
;       id=widget_info(Event.top, FIND_BY_UNAME=name)

;-----------------------------------------------------------------
pro xol_gui_update_refresh_time, Event

end
;
; Empty stub procedure used for autoloading.
;
pro xol_gui_eventcb
end
;-----------------------------------------------------------------
; Activate Button Callback Procedure.
; Argument:
;   Event structure:
;
;   {WIDGET_BUTTON, ID:0L, TOP:0L, HANDLER:0L, SELECT:0}
;
;   ID is the widget ID of the component generating the event. TOP is
;       the widget ID of the top level widget containing ID. HANDLER
;       contains the widget ID of the widget associated with the
;       handler routine.

;   SELECT is set to 1 if the button was set, and 0 if released.
;       Normal buttons do not generate events when released, so
;       SELECT will always be 1. However, toggle buttons (created by
;       parenting a button to an exclusive or non-exclusive base)
;       return separate events for the set and release actions.

;   Retrieve the IDs of other widgets in the widget hierarchy using
;       id=widget_info(Event.top, FIND_BY_UNAME=name)

;-----------------------------------------------------------------
pro xol_gui_pos_opt_cuts, Event

	@adsec_common

	common xol_gui_block, frame, slope, dm, xol_data

	if n_elements(dm) ne 0 then begin
		if xol_data.pos_show_ave then position=xol_data.ave_position else position=xol_data.position
		if xol_data.pos_remove_offset then $
			vec = (position-xol_data.pos_offset)[adsec.act_w_pos] $
		else $
			vec = position[adsec.act_w_pos]
		m = (moment(vec, SDEV=s))[0]
;		minv = m-3*s
;		maxv = m+3*s
		minv = min(vec, MAX=maxv)
		fld_id=widget_info(Event.top, FIND_BY_UNAME="POS_MAX_CBAR_FLD")
		widget_control, fld_id, SET_VALUE=strtrim(maxv,2)
		fld_id=widget_info(Event.top, FIND_BY_UNAME="POS_MIN_CBAR_FLD")
		widget_control, fld_id, SET_VALUE=strtrim(minv,2)
	endif
end
;-----------------------------------------------------------------
; Activate Button Callback Procedure.
; Argument:
;   Event structure:
;
;   {WIDGET_BUTTON, ID:0L, TOP:0L, HANDLER:0L, SELECT:0}
;
;   ID is the widget ID of the component generating the event. TOP is
;       the widget ID of the top level widget containing ID. HANDLER
;       contains the widget ID of the widget associated with the
;       handler routine.

;   SELECT is set to 1 if the button was set, and 0 if released.
;       Normal buttons do not generate events when released, so
;       SELECT will always be 1. However, toggle buttons (created by
;       parenting a button to an exclusive or non-exclusive base)
;       return separate events for the set and release actions.

;   Retrieve the IDs of other widgets in the widget hierarchy using
;       id=widget_info(Event.top, FIND_BY_UNAME=name)

;-----------------------------------------------------------------
pro xol_gui_curr_opt_cuts, Event

	@adsec_common

	common xol_gui_block, frame, slope, dm, xol_data

	if n_elements(dm) ne 0 then begin
		if xol_data.curr_show_ave then current=xol_data.ave_current else current=xol_data.current
		if xol_data.curr_remove_offset then $
			vec = (long(current)-xol_data.curr_offset)[adsec.act_w_curr] $
		else $
			vec = current[adsec.act_w_curr]
		m = (moment(vec, SDEV=s))[0]
;		minv = fix((m-3*s) > (-32768))
;		maxv = fix((m+3*s) < 32767)
		minv = min(vec, MAX=maxv)
		fld_id=widget_info(Event.top, FIND_BY_UNAME="CURR_MAX_CBAR_FLD")
		widget_control, fld_id, SET_VALUE=strtrim(maxv,2)
		fld_id=widget_info(Event.top, FIND_BY_UNAME="CURR_MIN_CBAR_FLD")
		widget_control, fld_id, SET_VALUE=strtrim(minv,2)
	endif
end
;-----------------------------------------------------------------
; Activate Button Callback Procedure.
; Argument:
;   Event structure:
;
;   {WIDGET_BUTTON, ID:0L, TOP:0L, HANDLER:0L, SELECT:0}
;
;   ID is the widget ID of the component generating the event. TOP is
;       the widget ID of the top level widget containing ID. HANDLER
;       contains the widget ID of the widget associated with the
;       handler routine.

;   SELECT is set to 1 if the button was set, and 0 if released.
;       Normal buttons do not generate events when released, so
;       SELECT will always be 1. However, toggle buttons (created by
;       parenting a button to an exclusive or non-exclusive base)
;       return separate events for the set and release actions.

;   Retrieve the IDs of other widgets in the widget hierarchy using
;       id=widget_info(Event.top, FIND_BY_UNAME=name)

;-----------------------------------------------------------------
pro xol_gui_rms_opt_cuts, Event

	@adsec_common

	common xol_gui_block, frame, slope, dm, xol_data

	if xol_data.rms_show_ave then begin
		if xol_data.display_rms then $
			vec=xol_data.ave_rms[adsec.act_w_pos] $
		else $
			vec=xol_data.ave_rec_dpos[adsec.act_w_pos]
	endif else begin
		if xol_data.display_rms then $
			vec=xol_data.rms[adsec.act_w_pos] $
		else $
			vec=xol_data.rec_dpos[adsec.act_w_pos]
	endelse

	minv = min(vec, MAX=maxv)
	fld_id=widget_info(Event.top, FIND_BY_UNAME="RMS_MAX_CBAR_FLD")
	widget_control, fld_id, SET_VALUE=strtrim(maxv,2)
	fld_id=widget_info(Event.top, FIND_BY_UNAME="RMS_MIN_CBAR_FLD")
	widget_control, fld_id, SET_VALUE=strtrim(minv,2)

end
;-----------------------------------------------------------------
; Activate Button Callback Procedure.
; Argument:
;   Event structure:
;
;   {WIDGET_BUTTON, ID:0L, TOP:0L, HANDLER:0L, SELECT:0}
;
;   ID is the widget ID of the component generating the event. TOP is
;       the widget ID of the top level widget containing ID. HANDLER
;       contains the widget ID of the widget associated with the
;       handler routine.

;   SELECT is set to 1 if the button was set, and 0 if released.
;       Normal buttons do not generate events when released, so
;       SELECT will always be 1. However, toggle buttons (created by
;       parenting a button to an exclusive or non-exclusive base)
;       return separate events for the set and release actions.

;   Retrieve the IDs of other widgets in the widget hierarchy using
;       id=widget_info(Event.top, FIND_BY_UNAME=name)

;-----------------------------------------------------------------
pro xol_gui_spot_opt_cuts, Event

	@adsec_common

	common xol_gui_block, frame, slope, dm, xol_data

	if n_elements(frame) ne 0 then begin
		minv = min(frame, MAX=maxv)
		fld_id=widget_info(Event.top, FIND_BY_UNAME="SPOT_MAX_CBAR_FLD")
		widget_control, fld_id, SET_VALUE=strtrim(maxv,2)
		fld_id=widget_info(Event.top, FIND_BY_UNAME="SPOT_MIN_CBAR_FLD")
		widget_control, fld_id, SET_VALUE=strtrim(minv,2)
	endif
end
;-----------------------------------------------------------------
; Activate Button Callback Procedure.
; Argument:
;   Event structure:
;
;   {WIDGET_BUTTON, ID:0L, TOP:0L, HANDLER:0L, SELECT:0}
;
;   ID is the widget ID of the component generating the event. TOP is
;       the widget ID of the top level widget containing ID. HANDLER
;       contains the widget ID of the widget associated with the
;       handler routine.

;   SELECT is set to 1 if the button was set, and 0 if released.
;       Normal buttons do not generate events when released, so
;       SELECT will always be 1. However, toggle buttons (created by
;       parenting a button to an exclusive or non-exclusive base)
;       return separate events for the set and release actions.

;   Retrieve the IDs of other widgets in the widget hierarchy using
;       id=widget_info(Event.top, FIND_BY_UNAME=name)

;-----------------------------------------------------------------
pro xol_gui_comm_opt_cuts, Event

	@adsec_common

	common xol_gui_block, frame, slope, dm, xol_data

	if n_elements(dm) ne 0 then begin
		if xol_data.comm_show_ave then command=xol_data.ave_command else command=xol_data.command
		if xol_data.comm_remove_offset then $
			vec = (long(command)-xol_data.comm_offset)[adsec.act_w_cl] $
		else $
			vec = command[adsec.act_w_cl]
		m = (moment(vec, SDEV=s))[0]
;		minv = fix((m-3*s) > (-32768))
;		maxv = fix((m+3*s) < 32767)
		minv = min(vec, MAX=maxv)
		fld_id=widget_info(Event.top, FIND_BY_UNAME="COMM_MAX_CBAR_FLD")
		widget_control, fld_id, SET_VALUE=strtrim(maxv,2)
		fld_id=widget_info(Event.top, FIND_BY_UNAME="COMM_MIN_CBAR_FLD")
		widget_control, fld_id, SET_VALUE=strtrim(minv,2)
	endif
end
;-----------------------------------------------------------------
; Activate Button Callback Procedure.
; Argument:
;   Event structure:
;
;   {WIDGET_BUTTON, ID:0L, TOP:0L, HANDLER:0L, SELECT:0}
;
;   ID is the widget ID of the component generating the event. TOP is
;       the widget ID of the top level widget containing ID. HANDLER
;       contains the widget ID of the widget associated with the
;       handler routine.

;   SELECT is set to 1 if the button was set, and 0 if released.
;       Normal buttons do not generate events when released, so
;       SELECT will always be 1. However, toggle buttons (created by
;       parenting a button to an exclusive or non-exclusive base)
;       return separate events for the set and release actions.

;   Retrieve the IDs of other widgets in the widget hierarchy using
;       id=widget_info(Event.top, FIND_BY_UNAME=name)

;-----------------------------------------------------------------
pro xol_gui_exit, Event
	widget_control, Event.top, /DESTROY
end
;-----------------------------------------------------------------
; Post Create Widget Procedure.
; Argument:
;   wWidget - ID number of specific widget.
;
;   Any keywords passed into the generated widget creation procedure
;       are passed into this procudure.

;   Retrieve the IDs of other widgets in the widget hierarchy using
;       id=widget_info(Event.top, FIND_BY_UNAME=name)

;-----------------------------------------------------------------
pro xol_gui_post_creation, wWidget, _EXTRA=_VWBExtra_

	@adsec_common
	common xol_gui_block, frame, slope, dm, xol_data

    dummy = get_fast_diagnostic(dm, /EMPTY)

    theta_tv = 90                  ;matches from front on the horizont pointing telescope (@ MMT)
	x_reflect_tv = 1B		;matches from front on the horizont pointing telescope (@ MMT)

	tr = theta_tv/180.0*!PI
	rot_mat = [[cos(tr), -sin(tr)], $
	           [sin(tr),  cos(tr)]]
	if x_reflect_tv then rot_mat[*,0] = -rot_mat[*,0]

	; System coordinates of the adaptive secondary actuators [mm]
	; act_coord: x+ toward West; y+ toward North; z+ toward the primary mirror
	act_coord = rot_mat ## transpose(adsec.act_coordinates)
	; coordinates in [mm]

	n_act = adsec.n_actuators

	n_zern_modes = 11
	zern2pos = fltarr(n_zern_modes, n_act)
	for i=0,n_zern_modes-1 do $
		zern2pos[i,*]=zern(i+1, act_coord[*,0]/adsec_shell.out_radius, act_coord[*,1]/adsec_shell.out_radius)

	svdc, zern2pos[*,adsec.act_w_cl], w, u, v
	eps =(machar()).eps
	idx = where(w gt eps*max(w), count)
	inv_w = fltarr(n_zern_modes)
	inv_w[idx] = 1.0/w[idx]
	pos2zern_w_cl = fltarr(n_act, n_zern_modes)
	pos2zern_w_cl[adsec.act_w_cl,*] = v ## diagonal_matrix(inv_w) ## transpose(u)


	svdc, zern2pos[*,adsec.act_w_pos], w, u, v
	eps =(machar()).eps
	idx = where(w gt eps*max(w), count)
	inv_w = fltarr(n_zern_modes)
	inv_w[idx] = 1.0/w[idx]
	pos2zern_w_pos = fltarr(n_act, n_zern_modes)
	pos2zern_w_pos[adsec.act_w_pos,*] = v ## diagonal_matrix(inv_w) ## transpose(u)

	svdc, zern2pos[*,adsec.act_w_curr], w, u, v
	eps =(machar()).eps
	idx = where(w gt eps*max(w), count)
	inv_w = fltarr(n_zern_modes)
	inv_w[idx] = 1.0/w[idx]
	curr2zern = fltarr(n_act, n_zern_modes)
	curr2zern[adsec.act_w_curr,*] = v ## diagonal_matrix(inv_w) ## transpose(u)
    if rtr.block then rec_mat_used = *rtr.b0_a_matrix else rec_mat_used = *rtr.b0_b_matrix

	xol_data = { $
	    position          : fltarr(n_act), $
	    command           : fltarr(n_act), $
	    current           : fltarr(n_act), $
	    rms               : fltarr(n_act), $
	    rec_dpos          : fltarr(n_act), $

	    ave_position      : fltarr(n_act), $
	    ave_command       : fltarr(n_act), $
	    ave_current       : fltarr(n_act), $
	    ave_rms        : fltarr(n_act), $
	    ave_rec_dpos      : fltarr(n_act), $

	    pos_offset        : fltarr(n_act), $
	    comm_offset       : fltarr(n_act), $
	    curr_offset       : fltarr(n_act), $
	    rms_offset       : fltarr(n_act), $

	    pos_remove_offset : 0B, $
	    curr_remove_offset: 0B, $
	    comm_remove_offset: 0B, $
	    rms_remove_offset: 0B, $

	    pos_show_ave      : 0B, $
	    curr_show_ave     : 0B, $
	    comm_show_ave     : 0B, $
	    rms_show_ave   : 0B, $

	    read_dm_from_dds  : 0B, $
	    act_coord         : act_coord, $
	    rec_mat           : rec_mat_used, $
	    display_rms    : 1B, $

		sx                : 0, $
		sy                : 0, $
	    counter           : 0, $
		max_idx           : 0, $
		frame2ave         : 0, $

	    pos_counter       : 0, $
		pos_max_idx       : 0, $
		n_pos2ave         : 0, $

	    curr_counter      : 0, $
		curr_max_idx      : 0, $
		n_curr2ave        : 0, $

	    comm_counter      : 0, $
		comm_max_idx      : 0, $
		n_comm2ave        : 0, $

	    rms_counter    : 0, $
		rms_max_idx    : 0, $
		n_rms2ave      : 0, $

	    offload_counter   : 0, $
		offload_max_idx   : 0, $
		n_offload2ave     : 0, $

		pos2zern_w_cl     : pos2zern_w_cl, $
		pos2zern_w_pos    : pos2zern_w_pos, $
		curr2zern         : curr2zern, $
		modal_decomp_idx  : 0, $
		n_zern_modes      : n_zern_modes}

end
;-----------------------------------------------------------------
; Activate Button Callback Procedure.
; Argument:
;   Event structure:
;
;   {WIDGET_BUTTON, ID:0L, TOP:0L, HANDLER:0L, SELECT:0}
;
;   ID is the widget ID of the component generating the event. TOP is
;       the widget ID of the top level widget containing ID. HANDLER
;       contains the widget ID of the widget associated with the
;       handler routine.

;   SELECT is set to 1 if the button was set, and 0 if released.
;       Normal buttons do not generate events when released, so
;       SELECT will always be 1. However, toggle buttons (created by
;       parenting a button to an exclusive or non-exclusive base)
;       return separate events for the set and release actions.

;   Retrieve the IDs of other widgets in the widget hierarchy using
;       id=widget_info(Event.top, FIND_BY_UNAME=name)

;-----------------------------------------------------------------
pro xol_gui_start_stop, Event
	widget_control, Event.ID, GET_VALUE=button_label

	case button_label of

		"Start": begin
			xol_gui_refresh_time_id=widget_info(Event.top, FIND_BY_UNAME="REFRESH_TIME_FLD")
			widget_control, xol_gui_refresh_time_id, GET_VALUE=updating_time
			widget_control, Event.ID, TIMER=float(updating_time[0])
			widget_control, Event.ID, SET_VALUE="Stop"
		end

		"Stop": begin
			widget_control, Event.ID, SET_VALUE="Start"
		end

		else: begin
			dummy = dialog_message("Unexpected internal error", /ERROR)
		end
	endcase
end
;-----------------------------------------------------------------
; Timer Event Callback Procedure.
; Argument:
;   Event structure:
;
;   {WIDGET_TIMER, ID:0L, TOP:0L, HANDLER:0L}
;   ID is the widget ID of the component generating the event. TOP is
;       the widget ID of the top level widget containing ID. HANDLER
;       contains the widget ID of the widget associated with the
;       handler routine.

;
;   Retrieve the IDs of other widgets in the widget hierarchy using
;       id=widget_info(Event.top, FIND_BY_UNAME=name)

;-----------------------------------------------------------------
pro xol_gui_on_timer, Event

	widget_control, Event.ID, GET_VALUE=button_label

	xol_gui_update_panels, Event

	if button_label eq "Stop" then begin
		xol_gui_refresh_time_id=widget_info(Event.top, FIND_BY_UNAME="REFRESH_TIME_FLD")
		widget_control, xol_gui_refresh_time_id, GET_VALUE=updating_time
		widget_control, Event.ID, TIMER=float(updating_time[0])
	end
end
;-----------------------------------------------------------------
; Activate Button Callback Procedure.
; Argument:
;   Event structure:
;
;   {WIDGET_BUTTON, ID:0L, TOP:0L, HANDLER:0L, SELECT:0}
;
;   ID is the widget ID of the component generating the event. TOP is
;       the widget ID of the top level widget containing ID. HANDLER
;       contains the widget ID of the widget associated with the
;       handler routine.

;   SELECT is set to 1 if the button was set, and 0 if released.
;       Normal buttons do not generate events when released, so
;       SELECT will always be 1. However, toggle buttons (created by
;       parenting a button to an exclusive or non-exclusive base)
;       return separate events for the set and release actions.

;   Retrieve the IDs of other widgets in the widget hierarchy using
;       id=widget_info(Event.top, FIND_BY_UNAME=name)

;-----------------------------------------------------------------
pro pos_new_offset, Event

	common xol_gui_block, frame, slope, dm, xol_data

	if xol_data.pos_show_ave then $
		xol_data.pos_offset = xol_data.ave_position $
	else $
	 	xol_data.pos_offset = xol_data.position
end
;-----------------------------------------------------------------
; Activate Button Callback Procedure.
; Argument:
;   Event structure:
;
;   {WIDGET_BUTTON, ID:0L, TOP:0L, HANDLER:0L, SELECT:0}
;
;   ID is the widget ID of the component generating the event. TOP is
;       the widget ID of the top level widget containing ID. HANDLER
;       contains the widget ID of the widget associated with the
;       handler routine.

;   SELECT is set to 1 if the button was set, and 0 if released.
;       Normal buttons do not generate events when released, so
;       SELECT will always be 1. However, toggle buttons (created by
;       parenting a button to an exclusive or non-exclusive base)
;       return separate events for the set and release actions.

;   Retrieve the IDs of other widgets in the widget hierarchy using
;       id=widget_info(Event.top, FIND_BY_UNAME=name)

;-----------------------------------------------------------------
pro curr_new_offset, Event

	common xol_gui_block, frame, slope, dm, xol_data

	if xol_data.curr_show_ave then $
		xol_data.curr_offset = xol_data.ave_current $
	else $
		xol_data.curr_offset = xol_data.current
end

pro rms_new_offset, Event

	common xol_gui_block, frame, slope, dm, xol_data

	if xol_data.rms_show_ave then $
		xol_data.rms_offset = xol_data.ave_rms $
	else $
		xol_data.rms_offset = xol_data.rms
end



;-----------------------------------------------------------------
; Activate Button Callback Procedure.
; Argument:
;   Event structure:
;
;   {WIDGET_BUTTON, ID:0L, TOP:0L, HANDLER:0L, SELECT:0}
;
;   ID is the widget ID of the component generating the event. TOP is
;       the widget ID of the top level widget containing ID. HANDLER
;       contains the widget ID of the widget associated with the
;       handler routine.

;   SELECT is set to 1 if the button was set, and 0 if released.
;       Normal buttons do not generate events when released, so
;       SELECT will always be 1. However, toggle buttons (created by
;       parenting a button to an exclusive or non-exclusive base)
;       return separate events for the set and release actions.

;   Retrieve the IDs of other widgets in the widget hierarchy using
;       id=widget_info(Event.top, FIND_BY_UNAME=name)

;-----------------------------------------------------------------
pro comm_new_offset, Event

	common xol_gui_block, frame, slope, dm, xol_data

	if xol_data.comm_show_ave then $
		xol_data.comm_offset = xol_data.ave_command $
	else $
		xol_data.comm_offset = xol_data.command
end
;-----------------------------------------------------------------
; Activate Button Callback Procedure.
; Argument:
;   Event structure:
;
;   {WIDGET_BUTTON, ID:0L, TOP:0L, HANDLER:0L, SELECT:0}
;
;   ID is the widget ID of the component generating the event. TOP is
;       the widget ID of the top level widget containing ID. HANDLER
;       contains the widget ID of the widget associated with the
;       handler routine.

;   SELECT is set to 1 if the button was set, and 0 if released.
;       Normal buttons do not generate events when released, so
;       SELECT will always be 1. However, toggle buttons (created by
;       parenting a button to an exclusive or non-exclusive base)
;       return separate events for the set and release actions.

;   Retrieve the IDs of other widgets in the widget hierarchy using
;       id=widget_info(Event.top, FIND_BY_UNAME=name)

;-----------------------------------------------------------------
pro xol_gui_pos_set_offset, Event

	common xol_gui_block, frame, slope, dm, xol_data

	xol_data.pos_remove_offset = event.select

end
;-----------------------------------------------------------------
; Activate Button Callback Procedure.
; Argument:
;   Event structure:
;
;   {WIDGET_BUTTON, ID:0L, TOP:0L, HANDLER:0L, SELECT:0}
;
;   ID is the widget ID of the component generating the event. TOP is
;       the widget ID of the top level widget containing ID. HANDLER
;       contains the widget ID of the widget associated with the
;       handler routine.

;   SELECT is set to 1 if the button was set, and 0 if released.
;       Normal buttons do not generate events when released, so
;       SELECT will always be 1. However, toggle buttons (created by
;       parenting a button to an exclusive or non-exclusive base)
;       return separate events for the set and release actions.

;   Retrieve the IDs of other widgets in the widget hierarchy using
;       id=widget_info(Event.top, FIND_BY_UNAME=name)

;-----------------------------------------------------------------
pro xol_gui_curr_set_offset, Event

	common xol_gui_block, frame, slope, dm, xol_data

	xol_data.curr_remove_offset = event.select

end

pro xol_gui_rms_set_offset, Event

	common xol_gui_block, frame, slope, dm, xol_data

	xol_data.rms_remove_offset = event.select

end
;
;-----------------------------------------------------------------
; Activate Button Callback Procedure.
; Argument:
;   Event structure:
;
;   {WIDGET_BUTTON, ID:0L, TOP:0L, HANDLER:0L, SELECT:0}
;
;   ID is the widget ID of the component generating the event. TOP is
;       the widget ID of the top level widget containing ID. HANDLER
;       contains the widget ID of the widget associated with the
;       handler routine.

;   SELECT is set to 1 if the button was set, and 0 if released.
;       Normal buttons do not generate events when released, so
;       SELECT will always be 1. However, toggle buttons (created by
;       parenting a button to an exclusive or non-exclusive base)
;       return separate events for the set and release actions.

;   Retrieve the IDs of other widgets in the widget hierarchy using
;       id=widget_info(Event.top, FIND_BY_UNAME=name)

;-----------------------------------------------------------------
pro xol_gui_comm_set_offset, Event

	common xol_gui_block, frame, slope, dm, xol_data

	xol_data.comm_remove_offset = event.select

	pointing_error_id=widget_info(Event.top, FIND_BY_UNAME="POINTING_ERROR_BASE")
	widget_control, pointing_error_id, SENSITIVE=event.select

end
;-----------------------------------------------------------------
; Activate Button Callback Procedure.
; Argument:
;   Event structure:
;
;   {WIDGET_BUTTON, ID:0L, TOP:0L, HANDLER:0L, SELECT:0}
;
;   ID is the widget ID of the component generating the event. TOP is
;       the widget ID of the top level widget containing ID. HANDLER
;       contains the widget ID of the widget associated with the
;       handler routine.

;   SELECT is set to 1 if the button was set, and 0 if released.
;       Normal buttons do not generate events when released, so
;       SELECT will always be 1. However, toggle buttons (created by
;       parenting a button to an exclusive or non-exclusive base)
;       return separate events for the set and release actions.

;   Retrieve the IDs of other widgets in the widget hierarchy using
;       id=widget_info(Event.top, FIND_BY_UNAME=name)

;-----------------------------------------------------------------
pro xol_gui_read_from_dds, Event

	common xol_gui_block, frame, slope, dm, xol_data

	xol_data.read_dm_from_dds = event.select

	base_id = widget_info(Event.top, FIND_BY_UNAME="POS_BASE")
	widget_control, base_id, SENSITIVE=event.select
	base_id = widget_info(Event.top, FIND_BY_UNAME="CURR_BASE")
	widget_control, base_id, SENSITIVE=event.select
	base_id = widget_info(Event.top, FIND_BY_UNAME="COMM_BASE")
	widget_control, base_id, SENSITIVE=event.select
	base_id = widget_info(Event.top, FIND_BY_UNAME="RMS_BASE")
	if xol_data.display_rms then $
		widget_control, base_id, SENSITIVE=event.select $
	else $
		widget_control, base_id, SENSITIVE=1B


end
;-----------------------------------------------------------------
; Post Create Widget Procedure.
; Argument:
;   wWidget - ID number of specific widget.
;
;   Any keywords passed into the generated widget creation procedure
;       are passed into this procudure.

;   Retrieve the IDs of other widgets in the widget hierarchy using
;       id=widget_info(Event.top, FIND_BY_UNAME=name)

;-----------------------------------------------------------------
pro xol_gui_pos_base_post_creation, wWidget, _EXTRA=_VWBExtra_

	common xol_gui_block, frame, slope, dm, xol_data

	widget_control, wWidget, SENSITIVE=xol_data.read_dm_from_dds

end
;-----------------------------------------------------------------
; Post Create Widget Procedure.
; Argument:
;   wWidget - ID number of specific widget.
;
;   Any keywords passed into the generated widget creation procedure
;       are passed into this procudure.

;   Retrieve the IDs of other widgets in the widget hierarchy using
;       id=widget_info(Event.top, FIND_BY_UNAME=name)

;-----------------------------------------------------------------
pro xol_gui_curr_base_post_creation, wWidget, _EXTRA=_VWBExtra_

	common xol_gui_block, frame, slope, dm, xol_data

	widget_control, wWidget, SENSITIVE=xol_data.read_dm_from_dds

end
;-----------------------------------------------------------------
; Post Create Widget Procedure.
; Argument:
;   wWidget - ID number of specific widget.
;
;   Any keywords passed into the generated widget creation procedure
;       are passed into this procudure.

;   Retrieve the IDs of other widgets in the widget hierarchy using
;       id=widget_info(Event.top, FIND_BY_UNAME=name)

;-----------------------------------------------------------------
pro xol_gui_rms_base_post_creation, wWidget, _EXTRA=_VWBExtra_

	common xol_gui_block, frame, slope, dm, xol_data

	widget_control, wWidget, SENSITIVE=xol_data.read_dm_from_dds

end
;-----------------------------------------------------------------
; Post Create Widget Procedure.
; Argument:
;   wWidget - ID number of specific widget.
;
;   Any keywords passed into the generated widget creation procedure
;       are passed into this procudure.

;   Retrieve the IDs of other widgets in the widget hierarchy using
;       id=widget_info(Event.top, FIND_BY_UNAME=name)

;-----------------------------------------------------------------
pro xol_gui_comm_base_post_creation, wWidget, _EXTRA=_VWBExtra_

	common xol_gui_block, frame, slope, dm, xol_data

	widget_control, wWidget, SENSITIVE=xol_data.read_dm_from_dds

end
;-----------------------------------------------------------------
; Activate Button Callback Procedure.
; Argument:
;   Event structure:
;
;   {WIDGET_BUTTON, ID:0L, TOP:0L, HANDLER:0L, SELECT:0}
;
;   ID is the widget ID of the component generating the event. TOP is
;       the widget ID of the top level widget containing ID. HANDLER
;       contains the widget ID of the widget associated with the
;       handler routine.

;   SELECT is set to 1 if the button was set, and 0 if released.
;       Normal buttons do not generate events when released, so
;       SELECT will always be 1. However, toggle buttons (created by
;       parenting a button to an exclusive or non-exclusive base)
;       return separate events for the set and release actions.

;   Retrieve the IDs of other widgets in the widget hierarchy using
;       id=widget_info(Event.top, FIND_BY_UNAME=name)

;-----------------------------------------------------------------
pro xol_gui_offload_tilt, Event

	common xol_gui_block, frame, slope, dm, xol_data

	if not xol_data.comm_remove_offset	then message, "Unexpected behavior!"

	; LOCAL tilt error [arcsec]. az_tilt is not projected to horizon plane
	; the AZ correction will be az_tilt/cos(elev). TCS requests az_tilt
	az_tilt_id = widget_info(Event.top, FIND_BY_UNAME="DELTA_AZIMUTH_FLD")
	widget_control, az_tilt_id, GET_VALUE=az_tilt
	az_tilt = double(az_tilt)
	elev_tilt_id = widget_info(Event.top, FIND_BY_UNAME="DELTA_ELEVATION_FLD")
	widget_control, elev_tilt_id, GET_VALUE=elev_tilt
	elev_tilt = double(elev_tilt)

	;err = tcs_get_ao_offset(delta_alt0, delta_az0, /VERB) ; get elevation and azimuth offset [arcsec]
	;if err ne 0 then return
	delta_alt0=0.0
	delta_az0=0.0
	gain=0.4

	;err = tcs_set_ao_offset(delta_az0+gain*az_tilt, delta_alt0+gain*elev_tilt, /VERB)
	;if err ne 0 then return

	xol_data.n_offload2ave = 0
end
;-----------------------------------------------------------------
; Post Create Widget Procedure.
; Argument:
;   wWidget - ID number of specific widget.
;
;   Any keywords passed into the generated widget creation procedure
;       are passed into this procudure.

;   Retrieve the IDs of other widgets in the widget hierarchy using
;       id=widget_info(Event.top, FIND_BY_UNAME=name)

;-----------------------------------------------------------------
pro xol_gui_pointing_error_creation, wWidget, _EXTRA=_VWBExtra_

	common xol_gui_block, frame, slope, dm, xol_data

	widget_control, wWidget, SENSITIVE=xol_data.comm_remove_offset

end
;-----------------------------------------------------------------
; Activate Button Callback Procedure.
; Argument:
;   Event structure:
;
;   {WIDGET_BUTTON, ID:0L, TOP:0L, HANDLER:0L, SELECT:0}
;
;   ID is the widget ID of the component generating the event. TOP is
;       the widget ID of the top level widget containing ID. HANDLER
;       contains the widget ID of the widget associated with the
;       handler routine.

;   SELECT is set to 1 if the button was set, and 0 if released.
;       Normal buttons do not generate events when released, so
;       SELECT will always be 1. However, toggle buttons (created by
;       parenting a button to an exclusive or non-exclusive base)
;       return separate events for the set and release actions.

;   Retrieve the IDs of other widgets in the widget hierarchy using
;       id=widget_info(Event.top, FIND_BY_UNAME=name)

;-----------------------------------------------------------------
pro rec_dcomm_press_btn, Event

	common xol_gui_block, frame, slope, dm, xol_data

	xol_data.display_rms = not event.select
	base_id = widget_info(Event.top, FIND_BY_UNAME="RMS_BASE")
	widget_control, base_id, SENSITIVE=1B
	if xol_data.display_rms then str="PositionRMS" else str="RTR delta-comm"
	base_id = widget_info(Event.top, FIND_BY_UNAME="RMS_LABEL")
	widget_control, base_id, SET_VALUE=str

	;force the reset of the running mean buffer
	xol_data.n_rms2ave=0
end
;-----------------------------------------------------------------
; Droplist Select Item Callback Procedure.
; Argument:
;   Event structure:
;
;   {WIDGET_DROPLIST, ID:0L, TOP:0L, HANDLER:0L, INDEX:0L }
;
;   ID is the widget ID of the component generating the event. TOP is
;       the widget ID of the top level widget containing ID. HANDLER
;       contains the widget ID of the widget associated with the
;       handler routine.

;   INDEX returns the index of the selected item. This can be used to
;       index the array of names originally used to set the widget's
;       value.

;   Retrieve the IDs of other widgets in the widget hierarchy using
;       id=widget_info(Event.top, FIND_BY_UNAME=name)

;-----------------------------------------------------------------
pro xol_gui_modal_decomp_select, Event

	common xol_gui_block, frame, slope, dm, xol_data
	; 0=No plot
	; 1=Position
	; 2=Current
	; 3=Command
	; 4=Delta-command
	xol_data.modal_decomp_idx = Event.index
end
;-----------------------------------------------------------------
; Activate Button Callback Procedure.
; Argument:
;   Event structure:
;
;   {WIDGET_BUTTON, ID:0L, TOP:0L, HANDLER:0L, SELECT:0}
;
;   ID is the widget ID of the component generating the event. TOP is
;       the widget ID of the top level widget containing ID. HANDLER
;       contains the widget ID of the widget associated with the
;       handler routine.

;   SELECT is set to 1 if the button was set, and 0 if released.
;       Normal buttons do not generate events when released, so
;       SELECT will always be 1. However, toggle buttons (created by
;       parenting a button to an exclusive or non-exclusive base)
;       return separate events for the set and release actions.

;   Retrieve the IDs of other widgets in the widget hierarchy using
;       id=widget_info(Event.top, FIND_BY_UNAME=name)

;-----------------------------------------------------------------
pro xol_take_background, Event

	dummy = dialog_message("Option not yet available")

end
;-----------------------------------------------------------------
; Activate Button Callback Procedure.
; Argument:
;   Event structure:
;
;   {WIDGET_BUTTON, ID:0L, TOP:0L, HANDLER:0L, SELECT:0}
;
;   ID is the widget ID of the component generating the event. TOP is
;       the widget ID of the top level widget containing ID. HANDLER
;       contains the widget ID of the widget associated with the
;       handler routine.

;   SELECT is set to 1 if the button was set, and 0 if released.
;       Normal buttons do not generate events when released, so
;       SELECT will always be 1. However, toggle buttons (created by
;       parenting a button to an exclusive or non-exclusive base)
;       return separate events for the set and release actions.

;   Retrieve the IDs of other widgets in the widget hierarchy using
;       id=widget_info(Event.top, FIND_BY_UNAME=name)

;-----------------------------------------------------------------
pro xol_gui_pos_show_ave, Event
	common xol_gui_block, frame, slope, dm, xol_data

	xol_data.pos_show_ave = event.select
end
;-----------------------------------------------------------------
; Activate Button Callback Procedure.
; Argument:
;   Event structure:
;
;   {WIDGET_BUTTON, ID:0L, TOP:0L, HANDLER:0L, SELECT:0}
;
;   ID is the widget ID of the component generating the event. TOP is
;       the widget ID of the top level widget containing ID. HANDLER
;       contains the widget ID of the widget associated with the
;       handler routine.

;   SELECT is set to 1 if the button was set, and 0 if released.
;       Normal buttons do not generate events when released, so
;       SELECT will always be 1. However, toggle buttons (created by
;       parenting a button to an exclusive or non-exclusive base)
;       return separate events for the set and release actions.

;   Retrieve the IDs of other widgets in the widget hierarchy using
;       id=widget_info(Event.top, FIND_BY_UNAME=name)

;-----------------------------------------------------------------
pro xol_gui_curr_show_ave, Event
	common xol_gui_block, frame, slope, dm, xol_data

	xol_data.curr_show_ave = event.select
end
;-----------------------------------------------------------------
; Activate Button Callback Procedure.
; Argument:
;   Event structure:
;
;   {WIDGET_BUTTON, ID:0L, TOP:0L, HANDLER:0L, SELECT:0}
;
;   ID is the widget ID of the component generating the event. TOP is
;       the widget ID of the top level widget containing ID. HANDLER
;       contains the widget ID of the widget associated with the
;       handler routine.

;   SELECT is set to 1 if the button was set, and 0 if released.
;       Normal buttons do not generate events when released, so
;       SELECT will always be 1. However, toggle buttons (created by
;       parenting a button to an exclusive or non-exclusive base)
;       return separate events for the set and release actions.

;   Retrieve the IDs of other widgets in the widget hierarchy using
;       id=widget_info(Event.top, FIND_BY_UNAME=name)

;-----------------------------------------------------------------
pro xol_gui_rms_show_ave, Event
	common xol_gui_block, frame, slope, dm, xol_data

	xol_data.rms_show_ave = event.select
end
;-----------------------------------------------------------------
; Activate Button Callback Procedure.
; Argument:
;   Event structure:
;
;   {WIDGET_BUTTON, ID:0L, TOP:0L, HANDLER:0L, SELECT:0}
;
;   ID is the widget ID of the component generating the event. TOP is
;       the widget ID of the top level widget containing ID. HANDLER
;       contains the widget ID of the widget associated with the
;       handler routine.

;   SELECT is set to 1 if the button was set, and 0 if released.
;       Normal buttons do not generate events when released, so
;       SELECT will always be 1. However, toggle buttons (created by
;       parenting a button to an exclusive or non-exclusive base)
;       return separate events for the set and release actions.

;   Retrieve the IDs of other widgets in the widget hierarchy using
;       id=widget_info(Event.top, FIND_BY_UNAME=name)

;-----------------------------------------------------------------
pro xol_gui_comm_show_ave, Event
	common xol_gui_block, frame, slope, dm, xol_data

	xol_data.comm_show_ave = event.select

	id=widget_info(Event.top, FIND_BY_UNAME="APPLY_AVE_COMMAND_BTN")
	widget_control, id, SENSITIVE=event.select

end
;-----------------------------------------------------------------
; Activate Button Callback Procedure.
; Argument:
;   Event structure:
;
;   {WIDGET_BUTTON, ID:0L, TOP:0L, HANDLER:0L, SELECT:0}
;
;   ID is the widget ID of the component generating the event. TOP is
;       the widget ID of the top level widget containing ID. HANDLER
;       contains the widget ID of the widget associated with the
;       handler routine.

;   SELECT is set to 1 if the button was set, and 0 if released.
;       Normal buttons do not generate events when released, so
;       SELECT will always be 1. However, toggle buttons (created by
;       parenting a button to an exclusive or non-exclusive base)
;       return separate events for the set and release actions.

;   Retrieve the IDs of other widgets in the widget hierarchy using
;       id=widget_info(Event.top, FIND_BY_UNAME=name)

;-----------------------------------------------------------------
pro xol_apply_ave_command, Event

	common xol_gui_block, frame, slope, dm, xol_data

	err = set_ave_command(xol_data.ave_command, xol_data.comm_offset)
end
