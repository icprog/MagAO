; $Id: proc_pos_click.pro,v 1.2 2002/03/14 18:36:05 riccardi Exp $

function proc_pos_click, event

common num_block, nums, current_num
common graph_block, gr
common system_block, sys_is_open
common generic_block, lf, cr, selected_acts
common adsec_block, adsec

click_point = [event.x, event.y]
on_act = float(gr.act_coord-rebin(click_point,2,adsec.n_actuators))
on_act = where(on_act(0,*)^2+on_act(1,*)^2 lt (gr.npix_cap/2)^2)
no_selected = (selected_acts(0) lt 0)
is_on_act = (on_act(0) ge 0)

                                ; mouse-left-button (set values of selected actuators)
if (event.press eq 1) then begin

                                ; click outside actuators
    if (not is_on_act) then begin
        if (no_selected) then begin
            disp_mess, 'Please, click on an actuator to set its value'
        endif else begin
            selected_acts = [-1L]
            disp_mess, 'All actuators have been deselected'
        endelse

                                ; click inside actuators
    endif else begin
        if (no_selected) then begin
            selected_acts = on_act
        endif else begin
            already_selected_act = where(selected_acts eq on_act(0))
            if (already_selected_act(0) lt 0) then $
              selected_acts = [selected_acts, on_act]
        endelse
        return, 1B
    endelse

                                ; mouse-right-button (select actuator)
endif else begin
    if (event.press gt 1) then begin

                                ; click outside actuators
        if (not is_on_act) then begin
            disp_mess, 'Please, click on an actuator to select or deselect it'
        endif else begin
            if (no_selected) then begin
                selected_acts = on_act
            endif else begin
                already_selected_act = where(selected_acts eq on_act(0))
                if (already_selected_act(0) lt 0) then begin
                    selected_acts = [selected_acts, on_act]
                endif else begin
                    selected_idxs = where(selected_acts ne on_act(0))
                    if selected_idxs(0) lt 0 then selected_acts = [-1L] $
                    else selected_acts = $
                      selected_acts(selected_idxs)
                endelse
            endelse
            disp_mess, 'Selected actuators:'
            disp_mess, '#'+strtrim(selected_acts,2), /APPEND
        endelse
    endif
endelse
return, 0B
end

