; $Id: init_menu.pro,v 1.6 2004/12/03 19:06:30 labot Exp $
;+
; HISTORY
;   30 Nov 2004: AR & MX
;     reload_wave_param, reload_ff_matrix and reload_ctrl_filters menu items added
;-
pro init_menu, base_bar

	@adsec_common
	@xadsec_common

	item_ids = {startup_sys_id  :0L, $
				load_program_id :0L, $
	            set_same_curr_id:0L, $
	            set_mirror_id   :0L, $
	            stop_control_id :0L}

	; system menu
	sys_menu = widget_button(base_bar, VALUE='System', /MENU)
		item_ids.startup_sys_id = widget_button(sys_menu, VALUE='Startup system', $
											UVALUE='startup_sys')
		item_ids.load_program_id = widget_button(sys_menu, VALUE='Load program', UVALUE='load_program')
		dummy = widget_button(sys_menu, VALUE='Reload FF matrix', UVALUE='reload_ff_matrix')
		dummy = widget_button(sys_menu, VALUE='Reload control filters', UVALUE='reload_ctrl_filters')
		dummy = widget_button(sys_menu, VALUE='Reload reference wave parameters', UVALUE='reload_wave_param')
		item_ids.set_same_curr_id = widget_button(sys_menu, VALUE='Same current on all actuators...', $
											UVALUE='set_same_curr')
		item_ids.set_mirror_id = widget_button(sys_menu, VALUE='Set the mirror in position', $
											UVALUE='mirror_set')
		dummy = widget_button(sys_menu, VALUE='Select all actuators', UVALUE='select_all')
		dummy = widget_button(sys_menu, VALUE='RIP the mirror', $
											UVALUE='rip', /SEP)
		dummy = widget_button(sys_menu, VALUE='Clear all DACs', $
											UVALUE='clear_dacs')
		item_ids.stop_control_id = widget_button(sys_menu, VALUE='Stop control on all actuators', $
											UVALUE='stop_control')
		dummy = widget_button(sys_menu, VALUE='Exit', UVALUE='exit', /SEP)

	; diagnostic menu
	;diagn_menu = widget_button(base_bar, VALUE='Diagnostic', /MENU)
		;dummy = widget_button(diagn_menu, VALUE='System setup...', UVALUE='sys_setup')

	; analysis menu
	anal_menu = widget_button(base_bar, VALUE='Analysis', /MENU)
		dummy = widget_button(anal_menu, VALUE='Set present position as offset', UVALUE='offset_pos')
		dummy = widget_button(anal_menu, VALUE='Reset offset position', UVALUE='reset_offset_pos')
		dummy = widget_button(anal_menu, VALUE='Temporal analisys...', UVALUE='temporal_analisys', /SEP)
		;dummy = widget_button(anal_menu, VALUE='Display mirror figure', UVALUE='disp_mirror')
		;dummy = widget_button(anal_menu, VALUE='View 3D mirror figure', UVALUE='view_mirror')

	; options menu
	opt_menu = widget_button(base_bar, VALUE='View', /MENU)
		dummy = widget_button(opt_menu, VALUE='Update graphic panels', $
											UVALUE='update_panels')
		dummy = widget_button(opt_menu, VALUE='Start continuous updating', UVALUE='start_update')
		dummy = widget_button(opt_menu, VALUE='Start continuous monitoring', UVALUE='start_monitor')
		dummy = widget_button(opt_menu, VALUE='Stop continuous updating/monitoring', UVALUE='stop_update')
		dummy = widget_button(opt_menu, VALUE='Set color cuts...', UVALUE='set_cuts', /SEP)
		dummy = widget_button(opt_menu, VALUE='Optimize color cuts', UVALUE='optimize_cuts')
		act_label = widget_button(opt_menu, VALUE='Actuator labels', /MENU, /SEP)
			dummy = widget_button(act_label, VALUE='DSP numbering', UVALUE='act_label_dsp')
			dummy = widget_button(act_label, VALUE='Mirror numbering', UVALUE='act_label_mirror')
			dummy = widget_button(act_label, VALUE='Warren numbering', UVALUE='act_label_warren')
			dummy = widget_button(act_label, VALUE='No numbers', UVALUE='act_label_no_numbers')
		dummy = widget_button(opt_menu, VALUE='Viewing options...', UVALUE='view_opt')
		dummy = widget_button(opt_menu, VALUE='DSP memory browser...', UVALUE='xdsp_mem_browser', /SEP)
		dummy = widget_button(opt_menu, VALUE='Load color table...', UVALUE='xloadct')
		dummy = widget_button(opt_menu, VALUE='Create palette...', UVALUE='xpalette')
	return
end

