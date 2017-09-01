; $Id: tanal_panel.pro,v 1.6 2004/11/30 10:07:46 riccardi Exp $
;
;+
;  HISTORY
;    21 Jul 2004:
;      Removed code top force format labels display using integers.
;
;    30 Nov 2004: A. Riccardi (AR)
;      added tanal_save_ascii event and related button
;      some modifications in the tanal_save event handler
;
;-
pro tanal_panel_event, event

	@adsec_common
	common tanal_panel_block, tanal_slide_id, tanal_nums, decimation, ndata2store, $
	       voltage_output, data

	widget_control, event.id, GET_UVALUE = uvalue

	case uvalue of

		'tanal_plot': begin

			psym=3
			widget_control, tanal_slide_id, GET_VALUE=the_act

			buffer = data[*, *, the_act]
			tvec = findgen(ndata2store)*adsec.sampling_time*(decimation+1) ; time [s]

			str_unit = ["us", "ms", "s", "min"]
			str_mult = [1e-6, 1e-3, 1.0, 60.0]

			str_idx = optim_unit(tvec, str_unit, str_mult)

			mean0 = (moment(buffer(*,0), SDEV=sdev0))[0]
		    xtitle = "Time ["+str_unit[str_idx]+"]"
			if voltage_output then ytitle="Voltage [counts]" else ytitle="Position [counts]"
			title = "act #"+strtrim(the_act,2)+" mean="+strtrim(mean0,2)+" sdev="+strtrim(sdev0,2)
			wset, tanal_nums.position_num
			plot, tvec/str_mult[str_idx], buffer(*,0), $
				XTIT=xtitle, YTIT=ytitle, ysty=1+2+16, $
				TIT=title, PSYM=psym

			mean1 = (moment(buffer(*,1), SDEV=sdev1))[0]
			title = "act #"+strtrim(the_act,2)+" mean="+strtrim(mean1,2)+" sdev="+strtrim(sdev1,2)
			wset, tanal_nums.current_num
			plot, tvec/str_mult[str_idx], buffer(*,1), $
				XTIT=xtitle, YTIT="Current [counts]", $
				ysty=1+2+16, TITLE=title, PSYM=psym
			wset, -1
		end

		'tanal_plot_fft': begin

			widget_control, tanal_slide_id, GET_VALUE=the_act


			buffer = data[*, *, the_act]

			n_freq = long(ndata2store/2)
			fvec = findgen(n_freq)/(ndata2store*adsec.sampling_time*(decimation+1)); freq [Hz]

			fft_data = fft(buffer(*,0))
			fft_data = abs(fft_data)/total(abs(fft_data)^2)*total(abs(buffer(*,0)^2))
			if voltage_output then ytitle="voltage count/Hz" else ytitle="position count/Hz"
			wset, tanal_nums.position_num
			plot, fvec(1:n_freq-1), fft_data(1:n_freq-1), $
				XTIT="Frequecy [Hz]", YTIT=ytitle, $
				ysty=1+2+16, TIT="act #"+strtrim(the_act), /XLOG

			tot_power = total(abs(fft_data)^2)
			if tot_power ne 0.0 then begin
				fft_data = fft(buffer(*,1))
				fft_data = abs(fft_data)/tot_power*total(abs(buffer(*,1))^2)
			endif else begin
				fft_data = fltarr(n_freq)
			endelse
			wset, tanal_nums.current_num
			plot, fvec(1:n_freq-1), fft_data(1:n_freq-1), $
				XTIT="Frequency [Hz]", YTIT="current bit/Hz", $
				ysty=1+2+16, /XLOG
			wset, -1
		end

		'tanal_save': begin
            default_dir = filepath(ROOT=adsec_path.meas,"")
            default_filename = "hist.sav"
			filename = dialog_pickfile(FILTER="*.sav" $
			    ,PATH=default_dir $
			    ,FILE=default_filename $
			    ,GET_PATH=selected_dir $
			    ,TITLE="Choose the file name to save all data" $
			    ,/WRITE)
			if filename ne "" then begin
				if (findfile(filename))[0] ne "" then begin
					ans = dialog_message(["The file "+filename+" already exists." $
										, "Do you want to overwrite it?"], /QUESTION)
					if strlowcase(ans) eq "no" then return
				endif

				tvec = findgen(ndata2store)*adsec.sampling_time*(decimation+1) ; time [s]
				buffer = data
				save, tvec, buffer, FILENAME=filename
			endif
		end

		'tanal_save_ascii': begin
            widget_control, tanal_slide_id, GET_VALUE=the_act

            default_dir = filepath(ROOT=adsec_path.home,SUB="Dati","")
            default_filename = "prova.txt"
			filename = dialog_pickfile(FILTER="*.txt" $
			    ,PATH=default_dir $
			    ,FILE=default_filename $
			    ,GET_PATH=selected_dir $
			    ,TITLE="Choose the file name to save act data" $
			    ,/WRITE)
			if filename ne "" then begin
				if (findfile(filename))[0] ne "" then begin
					ans = dialog_message(["The file "+filename+" already exists." $
										, "Do you want to overwrite it?"], /QUESTION)
					if strlowcase(ans) eq "no" then return
				endif
                tvec = findgen(ndata2store)*adsec.sampling_time*(decimation+1) ; time [s]
                buffer = data
                	openw, unit, filename, /GET
                	printf, unit, [double(transpose(tvec)),double(transpose(buffer[*,*,the_act]))]
                free_lun, unit
            endif
        end

		'tanal_cancel': begin
			widget_control, event.top, /DESTROY
		end

		'act_number_slider': begin
		end

		else: message, 'User value not handled: '+uvalue
	endcase
end



pro tanal_panel, the_decimation, the_ndata2store, the_sensor_output, the_data, GROUP=group

	common adsec_block, adsec
	common tanal_panel_block, tanal_slide_id, tanal_nums, decimation, $
	       ndata2store, sensor_output, data

	data = the_data
	decimation = the_decimation
	ndata2store = the_ndata2store
	sensor_output = the_sensor_output

	root_tanal_panel = widget_base(TITLE = 'Temporal analisys plots', /COLUMN, /MODAL, GROUP=group)
	tanal_base_panel = widget_base(root_tanal_panel, /COLUMN)
	buttons_tanal_panel = widget_base(root_tanal_panel, /ROW)

	tanal_nums = {	position_num:	0L, $
					current_num:	0L}

    ss = get_screen_size()
    sx = round(ss[0]*0.80) < 800
    sy = round(ss[1]*0.35)

	position_id = WIDGET_DRAW(tanal_base_panel, $
		;/BUTTON_EVENTS, $		;generate events when buttons pressed
		/FRAME, $
		UVALUE = 'position_plot', $
		XSIZE = sx, $
		YSIZE = sy)


	current_id = WIDGET_DRAW(tanal_base_panel, $
		;/BUTTON_EVENTS, $		;generate events when buttons pressed
		/FRAME, $
		UVALUE = 'current_plot', $
		XSIZE = sx, $
		YSIZE = sy)


	tanal_slide_id = cw_lslider(buttons_tanal_panel, $
                        TITLE="Act. number", $
                        /DRAG, $
                        /EDIT, $
                        /FRAME, $
                        SCROLL=5, $
                        MINIMUM=0L, $
                        MAXIMUM=long(adsec.n_actuators-1L), $
                        VALUE=0L, $
                        UVALUE="act_number_slider")

	dummy = widget_button(buttons_tanal_panel, VALUE='Plot time', UVALUE='tanal_plot')
	dummy = widget_button(buttons_tanal_panel, VALUE='Plot fft', UVALUE='tanal_plot_fft')
	dummy = widget_button(buttons_tanal_panel, VALUE='Save all', UVALUE='tanal_save')
	dummy = widget_button(buttons_tanal_panel, VALUE='Save act ascii', UVALUE='tanal_save_ascii')
	dummy = widget_button(buttons_tanal_panel, VALUE='Exit', UVALUE='tanal_cancel')

	WIDGET_CONTROL, root_tanal_panel, /REALIZE

	widget_control, position_id, GET_VALUE=position_num
	tanal_nums.position_num = position_num


	widget_control, current_id, GET_VALUE=current_num
	tanal_nums.current_num = current_num


	xmanager, 'tanal_panel', root_tanal_panel, GROUP_LEADER=group

end
