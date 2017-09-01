; $Id: num2color.pro,v 1.2 2002/03/14 18:36:05 riccardi Exp $

function num2color, closed_loop_act

	common graph_block, gr

	color = lonarr(n_elements(closed_loop_act))

	idx = where(closed_loop_act mod 2B, count)
	if count gt 0 then color(idx) = gr.cl_on_color
	idx = where((not closed_loop_act) mod 2B, count)
	if count gt 0 then color(idx) = gr.cl_off_color

	return, color
end
