; $Id: draw_rings.pro,v 1.5 2004/11/02 15:36:14 zanotti Exp $
;
;
;   2 Nov 2004 D.Zanotti(DZ)
;       adsec: in_radius, out_radius are changed in adsec_shell: in_radius, out_radius
;
pro draw_rings, ima, act_color, SMOOTH=smooth, LIST=list

    @adsec_common
    common draw_rings_block, x_in, y_in, idx

    if n_elements(list) eq 0 then list = indgen(adsec.n_actuators)

    zoom_fac = 15
    if keyword_set(smooth) then begin
        if n_elements(idx) eq 0 then begin
            r = shift(dist(gr.x_tv, gr.y_tv), gr.x_tv/2, gr.y_tv/2)
            idx = where((r le gr.tv_scale*adsec_shell.out_radius) and $
                        (r ge gr.tv_scale*adsec_shell.in_radius))
            x_in = gr.act_coord[0,*]*(1.0+1e-4*randomn(seed,adsec.n_actuators))
            y_in = gr.act_coord[1,*]*(1.0+1e-4*randomn(seed,adsec.n_actuators))
        endif

		dummy=act_color[list]
		if total(dummy ne dummy[0]) gt 0 then begin
	        sub_ima = min_curve_surf(float(dummy), x_in[list], y_in[list], $
	                                 GS=zoom_fac*[1,1], BOUNDS=[0,0,gr.x_tv-1,gr.y_tv-1], /TPS)
	    endif else begin
	    	dx = gr.x_tv/zoom_fac
	    	dy = gr.y_tv/zoom_fac
	    	sub_ima = replicate(float(dummy[0]), dx, dy)
	    endelse

        sub_ima = interpolate(sub_ima, findgen(gr.x_tv)/zoom_fac, findgen(gr.y_tv)/zoom_fac, /GRID)
        sub_ima = ((round(sub_ima) > 0) < 255)
        ima[idx] = sub_ima[idx]
    endif else begin
;        for i=0,adsec.n_actuators-1 do begin
;            x_pos1 = gr.act_coord(0,i)-gr.npix_cap/2
;            y_pos1 = gr.act_coord(1,i)-gr.npix_cap/2
;            ;x_pos2 = x_pos1+gr.npix_act-1
;            ;y_pos2 = y_pos1+gr.npix_act-1
;            ima(x_pos1,y_pos1) = act_color(i)*gr.capsens
;        endfor
        for i=0,n_elements(list)-1 do begin
            x_pos1 = gr.act_coord(0,list[i])-gr.npix_cap/2
            y_pos1 = gr.act_coord(1,list[i])-gr.npix_cap/2
            ;x_pos2 = x_pos1+gr.npix_act-1
            ;y_pos2 = y_pos1+gr.npix_act-1
            ima(x_pos1,y_pos1) = act_color[list[i]]*gr.capsens
        endfor
    endelse
end
