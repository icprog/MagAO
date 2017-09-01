; $Id: plot_numbers.pro,v 1.6 2009/04/10 14:01:49 labot Exp $

pro plot_numbers, color, NUM_TYPE=num_type, LIST=list
;
;            -1          no numbers
; num_type:  0 (default) internal representation (0,1,2,3,...)
;            1           mechanic representation (1,2,3,4,...)
;            2           Warren representation  (000,001,...,006,100,101,...,112,200,201,...,218,...)

;    common graph_block, gr
;    common adsec_block, adsec
    @adsec_common

    if n_elements(num_type) eq 0 then num_type=0

	if num_type ne -1 then begin
	if n_elements(list) eq 0 then list=indgen(adsec.n_actuators)

    the_nums = sc.act_name[*, num_type]

    xyouts, gr.num_coord(0,list), gr.num_coord(1,list), $
        the_nums[list], $
        ALIGNMENT=0.5, /DEVICE, COLOR=color[list], CHARSIZE=gr.charsize
	endif
    return
end
