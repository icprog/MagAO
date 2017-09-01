; $Id: xcuts_setup.pro,v 1.3 2007/11/05 15:29:30 marco Exp $


pro xcuts_setup_event, event

	common graph_block, gr
	common cuts_block, field_ids

	widget_control, event.id, GET_UVALUE = uvalue

	case uvalue of

		'cuts_setup_ok': begin

			widget_control, field_ids.pos_low_id, GET_VALUE=value
			gr.pos_cuts(0) = value
			widget_control, field_ids.pos_hi_id, GET_VALUE=value
			gr.pos_cuts(1) = value

			widget_control, field_ids.curr_low_id, GET_VALUE=value
			gr.curr_cuts(0) = value
			widget_control, field_ids.curr_hi_id, GET_VALUE=value
			gr.curr_cuts(1) = value

			widget_control, event.top, /DESTROY
		end

		'cuts_setup_cancel': begin

			widget_control, event.top, /DESTROY
		end

		else: begin
		end
	endcase

end



pro xcuts_setup, GROUP=group

	common graph_block, gr
	common cuts_block, field_ids

	root_cuts_setup = widget_base(TITLE = 'Color cuts setup', /COLUMN, /MODAL, GROUP=group)
	cuts_base = widget_base(root_cuts_setup, /ROW)
	buttons_cuts_setup = widget_base(root_cuts_setup, /ROW)

	pos_cuts_base = widget_base(cuts_base, /COLUMN, /FRAME)
	curr_cuts_base = widget_base(cuts_base, /COLUMN, /FRAME)

	field_ids = {pos_low_id:0L, pos_hi_id:0L, curr_low_id:0L, curr_hi_id:0L}

	field_ids.pos_hi_id = cw_field(pos_cuts_base,	TITLE="Position high cut:", $
													VALUE=gr.pos_cuts(1), $
													/COLUMN)
	field_ids.pos_low_id = cw_field(pos_cuts_base,	TITLE="Position low cut:", $
													VALUE=gr.pos_cuts(0), $
													/COLUMN)

	field_ids.curr_hi_id = cw_field(curr_cuts_base,	TITLE="Current high cut:", $
													VALUE=gr.curr_cuts(1), $
													/COLUMN)
	field_ids.curr_low_id = cw_field(curr_cuts_base, TITLE="Current low cut:", $
													VALUE=gr.curr_cuts(0), $
													/COLUMN)

	dummy = widget_button(buttons_cuts_setup, VALUE='OK', UVALUE='cuts_setup_ok')
	dummy = widget_button(buttons_cuts_setup, VALUE='Cancel', $
			UVALUE='cuts_setup_cancel')

	widget_control, root_cuts_setup, /REALIZE

	xmanager, 'xcuts_setup', root_cuts_setup, GROUP_LEADER=group

end

