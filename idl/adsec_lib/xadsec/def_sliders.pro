; $Id: def_sliders.pro,v 1.3 2004/05/19 15:58:33 labot Exp $

function def_sliders, base, act_vector, act_value

	n_acts = n_elements(act_vector)

	sliders_per_row = 6L
    n_row = n_acts/sliders_per_row

    sliders_setup = lonarr(n_row+1)
    for i=0,n_row do begin
        sliders_setup(i) = widget_base(base, /ROW)
    endfor

    slider_ids = lonarr(n_acts)

    for i=0,n_acts-1 do begin
        act_num_str = strtrim(act_vector(i), 2)
        slider_label = 'act.#'+act_num_str
        uvalue = 'curr_setup_slider_'+act_num_str
        slider_ids(i) = cw_fslider(sliders_setup(i/sliders_per_row), $
                        TITLE=slider_label, $
                        /DRAG, $
                        /EDIT, $
                        /FRAME, $
                        MINIMUM=-32768.0, $
                        MAXIMUM=32767.0, $
                        VALUE=act_value(i), $
                        UVALUE=uvalue)
    endfor

	return, slider_ids
end


