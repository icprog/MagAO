; $Id: xcurr_setup.pro,v 1.6 2008/02/05 11:03:13 labot Exp $
;+
; HISTORY
;   21 Jul 2004: Armando Riccardi (AR)
;    modifications to match LBT data types.
;  
;   01 Dec 2004: Marco Xompero (MX)
;    wait added to permit the preshaping.
;-



pro xcurr_setup_event, event

    @adsec_common 
    common slider_block, slider_ids, act_vector, same_at_all

    widget_control, event.id, GET_UVALUE = uvalue

    case uvalue of

        'curr_setup_ok': begin

            if (same_at_all) then begin
                nact = adsec.n_actuators
                act_vector=lindgen(nact)
                disp_mess, "Stopping the control and setting the same current on all actuators:"
                widget_control, slider_ids(0), GET_VALUE=value
                sys_status.current = value
            endif else begin
                nact = n_elements(slider_ids)
                str0 = "#"+strtrim(act_vector,2)+" "
                str = ""
                for i=0,nact-1 do str = str+str0[i]
                disp_mess, "Stopping the control and setting currents on actuators:"
                disp_mess, str, /APPEND

                for i=0,nact-1 do begin
                    widget_control, slider_ids(i), GET_VALUE=value
                    sys_status.current(act_vector(i)) = value
                endfor
            endelse
            
            disp_mess, "Stopping the control...", /APPEND
            err = stop_control(act_vector)
            if (err ne 0L) then begin
                disp_mess, "Error in stopping control (code #"+strtrim(err)+")!", /APPEND
                return
            endif
            disp_mess, "...done.", /APPEND

            old_ovs = rtr.oversampling_time
            err = set_diagnostic(over=0.0, MASTER=0)
            if err ne adsec_error.ok then begin
                message, "ERROR ON DIAGNOSTIC SETTING (error code: "+strtrim(err,2)+").", $
                         CONT=(sc.debug eq 0B)
                return
            endif
            wait, old_ovs > 0.01

            disp_mess, "Setting the currents...", /APPEND
            err = write_bias_curr(act_vector, sys_status.current[act_vector])
            if (err ne 0L) then begin
                disp_mess, "Error writing current value (code #"+strtrim(err)+")!", /APPEND
                return
            endif

            if old_ovs gt 0 then begin
                err = set_diagnostic(over=old_ovs, /MASTER)
                if err ne adsec_error.ok then begin
                    disp_mess, 'TEMPORAL ANALISYS FAILED, MIRROR UNPROTECTED!', /APPEND
                    return
                endif
            endif

            dum = time_preshaper(FF=tff) 
            wait,max(tff)
            disp_mess, "...done.", /APPEND

            sys_status.closed_loop(act_vector) = 0B
            disp_mess, "All done!", /APPEND

            widget_control, event.top, /DESTROY
            update_panels
        end

        'curr_setup_cancel': begin
            widget_control, event.top, /DESTROY
        end

        else: begin
        end
    endcase

end



pro xcurr_setup, actuators, GROUP=group, SAME=same

    common system_block, sys_status
    common adsec_block, adsec
    common slider_block, slider_ids, act_vector, same_at_all

    same_at_all = keyword_set(same)
    if (same_at_all) then begin
        act_vector=[0L]
    endif else begin
        actuators = actuators(sort(actuators))
        act_vector = actuators
    end

    root_curr_setup = widget_base(TITLE = 'Current setup', /COLUMN, /MODAL, GROUP=group)
    sliders_base = widget_base(root_curr_setup, /COLUMN)
    buttons_curr_setup = widget_base(root_curr_setup, /ROW)

    slider_ids = def_sliders(sliders_base, act_vector, sys_status.current(act_vector))

    dummy = widget_button(buttons_curr_setup, VALUE='Stop control and set bias current', UVALUE='curr_setup_ok')
    dummy = widget_button(buttons_curr_setup, VALUE='Cancel', $
            UVALUE='curr_setup_cancel')

    widget_control, root_curr_setup, /REALIZE

    xmanager, 'xcurr_setup', root_curr_setup, GROUP_LEADER=group

end

