;
; IDL Event Callback Procedures
; xsl_comm_gui_eventcb
;
; Generated on:	08/10/2005 15:38.54
;
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
pro xls_comm_on_timer, Event

@adsec_common

common xsl_comm_gui_block, xsl_counter

ext = ".fits"
xls_gui_start_btn_id=widget_info(Event.top, FIND_BY_UNAME="START_STOP_BTN")
widget_control, xls_gui_start_btn_id, GET_VALUE=button_label

       ;;;;;; QUI CODICE PER INTERPRETARE I COMANDI
xls_message_fld_id=widget_info(Event.top, FIND_BY_UNAME="MESSAGE_FLD")

root_dir = filepath(ROOT=adsec_path.home, SUB=["xsl_comm_dir"],"")
fits_file_in = findfile(filepath(ROOT=root_dir,"*_in"+ext))
if fits_file_in[0] eq "" then n_files=0 else n_files = n_elements(fits_file_in)
if n_elements(xsl_counter) eq 0 then xsl_counter=0UL
xsl_counter += 1UL
lf = string(sc.lf)
message_str = strtrim(xsl_counter,2)+lf

case n_files of
    0: begin
    end

    1: begin
        fits_file_in = fits_file_in[0]
        basename = file_basename(fits_file_in,"_in"+ext)
        message_str = [temporary(message_str),"Found FITS: "+basename+lf]
        widget_control, xls_message_fld_id, SET_VALUE=message_str
        case basename of
            "apply_mode_vector": begin
                
                mode_vector = readfits(fits_file_in)
                if test_type(mode_vector, /FLOAT, DIM=n_dim, N_EL=n_el) then begin
                    message_str = [temporary(message_str), "ERROR: mode_vector must be FLOAT."+lf]
                    widget_control, xls_message_fld_id, SET_VALUE=message_str
                    break;
                endif
                if n_dim[0] ne 1 then begin
                    message_str = [temporary(message_str), "ERROR: mode_vector must be a VECTOR."+lf]
                    widget_control, xls_message_fld_id, SET_VALUE=message_str
                    break;
                endif
                if n_el ne adsec.n_actuators then begin
                    message_str = [temporary(message_str) $
                                   , "ERROR: mode_vector must have " $
                                   +strtrim(adsec.n_actuators,2)+" elements."+lf]
                    widget_control, xls_message_fld_id, SET_VALUE=message_str
                    break;
                endif

                file_delete, fits_file_in
                ;;; MANDA LO SLOPE VECTOR
                print, mode_vector
                err = apply_mode_vector(mode_vector)
                ;err =0L
                if err eq adsec_error.ok then begin
                    message_str = [temporary(message_str) $
                                   , "Input FITS file removed and slope vector APPLIED."+lf]
                endif else if err eq 12345L then begin
                    message_str = [temporary(message_str) $
                                   , "TOO LARGE CURRENT, MODES NOT APPLIED. Input FITS file removed."+lf]
                endif else begin
                    message_str = [temporary(message_str) $
                                   , "ERROR in appling slopes. Input FITS file removed."+lf]
                endelse
                widget_control, xls_message_fld_id, SET_VALUE=message_str
                

                ;;; COSTRUISCE RISPOSTA
                fits_file_out = filepath(ROOT=root_dir, basename+"_out"+ext)
                temp_file_out = filepath(ROOT=root_dir,"temp_out.fits")
                writefits, temp_file_out, [err]
                file_move, temp_file_out, fits_file_out, /OVER, /VERB
                message_str = [temporary(message_str) $
                               , "Answer in file: "+basename+"_out"+ext+lf]
                widget_control, xls_message_fld_id, SET_VALUE=message_str
            end

            else: begin
                message_str = [temporary(message_str) $
                               , "Unexpected file name: "+basename+"_in"+ext+lf]
                widget_control, xls_message_fld_id, SET_VALUE=message_str
            end
        endcase
    end
    
    else: begin
        message_str = [temporary(message_str) $
                       , "Unexpected file name: "+basename+"_in"+ext+lf]
        widget_control, xls_message_fld_id, SET_VALUE=message_str
    end
endcase

if button_label eq "Stop" then begin
    xsl_comm_refresh_time_id=widget_info(Event.top, FIND_BY_UNAME="REFRESH_TIME_FLD")
    widget_control, xsl_comm_refresh_time_id, GET_VALUE=updating_time
    widget_control, Event.top, TIMER=float(updating_time[0])
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
pro xsc_comm_start_btn_on_press, Event
	widget_control, Event.ID, GET_VALUE=button_label

	case button_label of

		"Start": begin
			xsl_comm_gui_refresh_time_id=widget_info(Event.top, FIND_BY_UNAME="REFRESH_TIME_FLD")
			widget_control, xsl_comm_gui_refresh_time_id, GET_VALUE=updating_time
			widget_control, Event.top, TIMER=float(updating_time[0])
			widget_control, Event.id, SET_VALUE="Stop"
		end

		"Stop": begin
			widget_control, Event.ID, SET_VALUE="Start"
		end

		else: begin
			dummy = dialog_message("Unexpected internal error", /ERROR)
		end
	endcase

end
;
; Empty stub procedure used for autoloading.
;
pro xsl_comm_gui_eventcb
end
