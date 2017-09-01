;
; IDL Event Callback Procedures
; housekeeping_eventcb
;
; Generated on:	11/21/2007 00:04.10
;
;
; Empty stub procedure used for autoloading.
;
pro housekeeping_eventcb

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
pro housekeeping_on_timer, Event

	@adsec_common
	common housekeeping_gui_block, diag_buffer, idx_buffer, gui_data

	if sc.debug then print, "reading housekeeping diagnostics..."
	diag_struct = diag_buffer[0]
	err=get_slow_diagnostic(diag_struct)
	if err ne adsec_error.ok then begin
		message, "get_slow_diagnostic function failed!", CONT=(sc.debug eq 0)
		return
	endif

	status_bmp = gui_data.status_bmp

	widget_control, event.top, SET_UVALUE=diag_struct

	n_diag_buffer = n_elements(diag_buffer)
	idx_buffer = (idx_buffer+1L) mod n_diag_buffer
	diag_buffer[idx_buffer] = diag_struct

	tag_name_list = tag_names(diag_struct)
	n_tags = n_elements(tag_name_list)
	for i=0,n_tags-1 do begin
		tag_name = tag_name_list[i]
		case strlowcase(strmid(tag_name,0,3)) of
			"bcu": begin
				;print, tag_name
				id=widget_info(Event.top, FIND_BY_UNAME="bcu_crate_num1")
				widget_control, id, get_value=crate_id

				id=widget_info(Event.top, FIND_BY_UNAME=strlowcase(tag_name))
				if id eq 0 then begin
					message, "Housekeeping variable "+tag_name+" is not shown in GUI.", /INFO
				endif else begin
					widget_type=widget_info(id, /TYPE)
					if widget_type eq 1 then begin
						;button
						widget_control, id, set_button=diag_struct.(i).average[crate_id] gt 0.5
					endif else begin
						;field
						widget_control, id, set_value=string(diag_struct.(i).average[crate_id])
						id=widget_info(Event.top, FIND_BY_UNAME=strlowcase(tag_name)+"_")
						widget_control, id, GET_UVALUE=curr_status
						if n_elements(curr_status) eq 0 then curr_status=-1
						if curr_status ne diag_struct.(i).status[crate_id] then begin
							new_status = diag_struct.(i).status[crate_id]
							widget_control, id, SET_VALUE=status_bmp[*,*,*,new_status]
							widget_control, id, SET_UVALUE=new_status
						endif
					endelse
				endelse
			end

			"tot": begin
				;print, tag_name
				id=widget_info(Event.top, FIND_BY_UNAME="bcu_crate_num1")
				widget_control, id, get_value=crate_id

				id=widget_info(Event.top, FIND_BY_UNAME=strlowcase(tag_name))
				if id eq 0 then begin
					message, "Housekeeping variable "+tag_name+" is not shown in GUI.", /INFO
				endif else begin
					widget_type=widget_info(id, /TYPE)
					if widget_type eq 1 then begin
						;button
						widget_control, id, set_button=diag_struct.(i).average[crate_id] gt 0.5
					endif else begin
						;field
						widget_control, id, set_value=string(diag_struct.(i).average[crate_id])
						id=widget_info(Event.top, FIND_BY_UNAME=strlowcase(tag_name)+"_")
						widget_control, id, GET_UVALUE=curr_status
						if n_elements(curr_status) eq 0 then curr_status=-1
						if curr_status ne diag_struct.(i).status[crate_id] then begin
							new_status = diag_struct.(i).status[crate_id]
							widget_control, id, SET_VALUE=status_bmp[*,*,*,new_status]
							widget_control, id, SET_UVALUE=new_status
						endif
					endelse
				endelse
			end

			"sgn": begin
				;print, tag_name
				id=widget_info(Event.top, FIND_BY_UNAME="sgn_crate_num")
				widget_control, id, get_value=crate_id

				id=widget_info(Event.top, FIND_BY_UNAME=strlowcase(tag_name))
				if id eq 0 then begin
					message, "Housekeeping variable "+tag_name+" is not shown in GUI.", /INFO
				endif else begin
					widget_type=widget_info(id, /TYPE)
					if widget_type eq 1 then begin
						;button
						widget_control, id, set_button=diag_struct.(i).average[crate_id] gt 0.5
					endif else begin
						;field
						widget_control, id, set_value=string(diag_struct.(i).average[crate_id])
						id=widget_info(Event.top, FIND_BY_UNAME=strlowcase(tag_name)+"_")
						widget_control, id, GET_UVALUE=curr_status
						if n_elements(curr_status) eq 0 then curr_status=-1
						if curr_status ne diag_struct.(i).status[crate_id] then begin
							new_status = diag_struct.(i).status[crate_id]
							widget_control, id, SET_VALUE=status_bmp[*,*,*,new_status]
							widget_control, id, SET_UVALUE=new_status
						endif
					endelse
				endelse
			end

			"dsp": begin
				;print, tag_name
				id=widget_info(Event.top, FIND_BY_UNAME="dsp_board_num")
				widget_control, id, get_value=board_id

				tag_name =strlowcase(tag_name)
				exep_list = ["dspadcspioffset","dspadcspigain","dspadcoffset","dspadcgain","dspdacoffset","dspdacgain"]
				if total(tag_name eq exep_list) ne 0 then begin
					id=widget_info(Event.top, FIND_BY_UNAME="dsp_board_channel")
					widget_control, id, get_value=channel_id
					str_ch=""
					n_act_per_board=adsec.n_dsp_per_board*adsec.n_act_per_dsp
					idx_offset = board_id*n_act_per_board+channel_id
				endif else begin
					if n_elements(diag_struct.(i).average) gt adsec.n_board_per_bcu*adsec.n_crates then begin
						n_act_per_board=adsec.n_dsp_per_board*adsec.n_act_per_dsp
						str_ch=strtrim(indgen(n_act_per_board),2)
						idx_offset = board_id*n_act_per_board
					endif else begin
						idx_offset=board_id
						str_ch=""
					endelse
				endelse
				n_ch=n_elements(str_ch)
				for i_ch=0,n_ch-1 do begin
					uname = strlowcase(tag_name)+str_ch[i_ch]
					id=widget_info(Event.top, FIND_BY_UNAME=uname)
					;print, uname
					if id eq 0 then begin
						message, "Housekeeping variable "+tag_name+" is not shown in GUI.", /INFO
					endif else begin
						widget_type=widget_info(id, /TYPE)
						if widget_type eq 1 then begin
							;button
							widget_control, id, set_button=diag_struct.(i).average[idx_offset+i_ch] gt 0.5
						endif else begin
							;field
							widget_control, id, set_value=string(diag_struct.(i).average[idx_offset+i_ch])
							id=widget_info(Event.top, FIND_BY_UNAME=strlowcase(tag_name)+str_ch[i_ch]+"_")
							widget_control, id, GET_UVALUE=curr_status
							if n_elements(curr_status) eq 0 then curr_status=-1
							if curr_status ne diag_struct.(i).status[idx_offset+i_ch] then begin
								new_status = diag_struct.(i).status[idx_offset+i_ch]
								widget_control, id, SET_VALUE=status_bmp[*,*,*,new_status]
								widget_control, id, SET_UVALUE=new_status
							endif
						endelse
					endelse
				endfor
			end

			else: begin
				;print, tag_name
;				id=widget_info(Event.top, FIND_BY_UNAME="dsp_board_num")
;				widget_control, id, get_value=board_id

				id=widget_info(Event.top, FIND_BY_UNAME=strlowcase(tag_name))
				if id eq 0 then begin
					message, "Housekeeping variable "+tag_name+" is not shown in GUI.", /INFO
				endif else begin
					widget_type=widget_info(id, /TYPE)
					if widget_type eq 1 then begin
						;button
						widget_control, id, set_button=diag_struct.(i).average gt 0.5
					endif else begin
						;field
						widget_control, id, set_value=string(diag_struct.(i).average)
						id=widget_info(Event.top, FIND_BY_UNAME=strlowcase(tag_name)+"_")
						widget_control, id, GET_UVALUE=curr_status
						if n_elements(curr_status) eq 0 then curr_status=-1
						if curr_status ne diag_struct.(i).status then begin
							new_status = diag_struct.(i).status
							widget_control, id, SET_VALUE=status_bmp[*,*,*,new_status]
							widget_control, id, SET_UVALUE=new_status
						endif
					endelse
				endelse
			end

		endcase
	endfor
	if gui_data.is_running eq 1B then widget_control, event.top, TIMER=2.0
end
;-----------------------------------------------------------------
; Notify Realize Callback Procedure.
; Argument:
;   wWidget - ID number of specific widget.
;
;
;   Retrieve the IDs of other widgets in the widget hierarchy using
;       id=widget_info(Event.top, FIND_BY_UNAME=name)

;-----------------------------------------------------------------
pro housekeeping_on_realize, wWidget

	@adsec_common
	common housekeeping_gui_block, diag_buffer, idx_buffer, gui_data

	print, "Initializing"
	n_diag_buffer = 50L
	if sc.debug then print, "reading housekeeping diagnostics..."
	err=get_slow_diagnostic(diag_struct, /EMPTY)
	if err ne adsec_error.ok then begin
		message, "get_slow_diagnostic function failed!", CONT=(sc.debug eq 0)
		return
	endif

	diag_buffer = replicate(diag_struct, n_diag_buffer)
	idx_buffer = -1L

	status_bmp=bytarr(16,16,3,3)
	status_bmp[*,*,1,0]=255B
	status_bmp[*,*,0,1]=255B
	status_bmp[*,*,1,1]=255B
	status_bmp[*,*,0,2]=255B

	gui_data = { $
		status_bmp: status_bmp, $
		is_running: 0B }
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
pro start_btn_on_button_press, Event
	common housekeeping_gui_block, diag_buffer, idx_buffer, gui_data
	widget_control, Event.id, GET_VALUE=name
	if strlowcase(name) eq "start" then begin
		gui_data.is_running = 1B
		widget_control, Event.id, SET_VALUE="Stop"
		widget_control, event.top, TIMER=2.0
	endif else begin
		gui_data.is_running = 0B
		widget_control, Event.id, SET_VALUE="Start"
	endelse
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
pro housekeeping_post_creation, wWidget, _EXTRA=_VWBExtra_

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
pro exit_btn_on_button_press, Event
	WIDGET_CONTROL, Event.top, /DESTROY
end
