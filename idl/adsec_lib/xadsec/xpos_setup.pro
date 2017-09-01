; $Id: xpos_setup.pro,v 1.5 2004/12/03 19:11:29 labot Exp $
;+
; HISTORY
;   21 Jul 2004: Armando Riccardi (AR)
;    modifications to match LBT data types.
;  
;   01 Dec 2004: Marco Xompero (MX)
;    wait added to permit the preshaping.
;-

pro xpos_setup_event, event

    common system_block, sys_status
    common adsec_block, adsec
    common slider_block, slider_ids, act_vector, same_at_all

    widget_control, event.id, GET_UVALUE = uvalue

    case uvalue of

        'pos_setup_ok': begin

            if (same_at_all) then begin
                nact = adsec.n_actuators
                act_vector=lindgen(nact)
                disp_mess, "Close the loop and setting the same position on all actuators:"
                widget_control, slider_ids(0), GET_VALUE=value
                sys_status.position = value
            endif else begin
                nact = n_elements(slider_ids)
                str0 = "#"+strtrim(act_vector,2)+" "
                str = ""
                for i=0,nact-1 do str = str+str0[i]
                disp_mess, "Closing the loop and setting position on actuators:"
                disp_mess, str, /APPEND

                for i=0,nact-1 do begin
                    widget_control, slider_ids(i), GET_VALUE=value
                    sys_status.position(act_vector(i)) = value
                endfor
            endelse

            disp_mess, "Setting the new commands...", /APPEND
            err = write_des_pos(act_vector, sys_status.position[act_vector])
            if (err ne 0L) then begin
                disp_mess, "Error writing position value (code #"+strtrim(err)+")!", /APPEND
                return
            endif
            dum = time_preshaper(POS=tpos)
            wait, max(tpos)
            disp_mess, "...done.", /APPEND

            disp_mess, "Enabling the control...", /APPEND
            err = start_control(act_vector)
            if (err ne 0L) then begin
                disp_mess, "Error acting position value (code #"+strtrim(err)+")!", /APPEND
                return
            endif
            disp_mess, "...done.", /APPEND

            sys_status.closed_loop(act_vector) = 1B
            disp_mess, "All done!", /APPEND

            widget_control, event.top, /DESTROY
            update_panels
        end

        'pos_setup_cancel': begin

            widget_control, event.top, /DESTROY
        end

        else: begin
        end
    endcase

end



pro xpos_setup, actuators, GROUP=group, SAME=same

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

    root_pos_setup = widget_base(TITLE = 'Position setup', /COLUMN, /MODAL, GROUP=group)
    sliders_base = widget_base(root_pos_setup, /COLUMN)
    buttons_pos_setup = widget_base(root_pos_setup, /ROW)

    slider_ids = def_sliders(sliders_base, act_vector, sys_status.position(act_vector))

    dummy = widget_button(buttons_pos_setup, VALUE='Set commnad and Start control', UVALUE='pos_setup_ok')
    dummy = widget_button(buttons_pos_setup, VALUE='Cancel', $
            UVALUE='pos_setup_cancel')

    widget_control, root_pos_setup, /REALIZE

    xmanager, 'xpos_setup', root_pos_setup, GROUP_LEADER=group
end

