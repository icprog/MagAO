; $Id: xview_opt_setup.pro,v 1.2 2002/03/14 18:36:04 riccardi Exp $


pro xview_opt_setup_event, event

	common system_block, sys_status
	common vopt_block, field_ids

	widget_control, event.id, GET_UVALUE = uvalue

	case uvalue of

		'vopt_setup_ok': begin

			widget_control, field_ids.next_update_id, GET_VALUE=value
			sys_status.next_update = value
			widget_control, field_ids.ndata2ave_id, GET_VALUE=value
			sys_status.ndata2ave = value

			widget_control, event.top, /DESTROY
		end

		'vopt_setup_cancel': begin

			widget_control, event.top, /DESTROY
		end

		else: begin
		end
	endcase

end



pro xview_opt_setup, GROUP=group

	common system_block, sys_status
	common vopt_block, field_ids

	root_vopt_setup = widget_base(TITLE = 'Viewing options settings', /COLUMN, /MODAL, GROUP=group)
	vopt_base = widget_base(root_vopt_setup, /COLUMN)
	buttons_vopt_setup = widget_base(root_vopt_setup, /ROW)

	field_ids = {	next_update_id:	0L, $
					ndata2ave_id:	0L}

	field_ids.next_update_id = cw_fslider(root_vopt_setup, $
                        TITLE="Time of refresh [s]", $
                        /DRAG, $
                        /EDIT, $
                        /FRAME, $
                        MINIMUM=0.1, $
                        MAXIMUM=10.0, $
                        VALUE=sys_status.next_update, $
                        UVALUE="next_update_slider")
	field_ids.ndata2ave_id = cw_lslider(root_vopt_setup, $
                        TITLE="Number of data values to average [s]", $
                        /DRAG, $
                        /EDIT, $
                        /FRAME, $
                        MINIMUM=1L, $
                        MAXIMUM=1024L, $
                        VALUE=sys_status.ndata2ave, $
                        UVALUE="ndata2ave_slider")


	dummy = widget_button(buttons_vopt_setup, VALUE='OK', UVALUE='vopt_setup_ok')
	dummy = widget_button(buttons_vopt_setup, VALUE='Cancel', $
			UVALUE='vopt_setup_cancel')

	widget_control, root_vopt_setup, /REALIZE

	xmanager, 'xview_opt_setup', root_vopt_setup, GROUP_LEADER=group

end

