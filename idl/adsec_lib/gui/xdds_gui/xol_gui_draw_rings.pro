pro xol_gui_draw_rings, ima, act_color, SMOOTH=smooth, LIST=list

    @adsec_common
	common xol_gui_block, frame, slope, dm, xol_data
	common xol_gui_draw_block, capsens

    if n_elements(list) eq 0 then list = indgen(adsec.n_actuators)

    if keyword_set(smooth) then begin
    	common draw_rings_block, x_in, y_in, idx
	    zoom_fac = 15
        if n_elements(idx) eq 0 then begin
            r = shift(dist(gr.x_tv, gr.y_tv), gr.x_tv/2, gr.y_tv/2)
            idx = where((r le gr.tv_scale*adsec.out_radius) and $
                        (r ge gr.tv_scale*adsec.in_radius))
            x_in = xol_data.act_coord[0,*]*(1.0+1e-4*randomn(seed,adsec.n_actuators))
            y_in = xol_data.act_coord[1,*]*(1.0+1e-4*randomn(seed,adsec.n_actuators))
        endif
        sub_ima = min_curve_surf(float(act_color[list]), x_in[list], y_in[list], $
                                 GS=zoom_fac*[1,1], BOUNDS=[0,0,gr.x_tv-1,gr.y_tv-1], /TPS)

        sub_ima = interpolate(sub_ima, findgen(gr.x_tv)/zoom_fac, findgen(gr.y_tv)/zoom_fac, /GRID)
        sub_ima = ((round(sub_ima) > 0) < 255)
        ima[idx] = sub_ima[idx]
    endif else begin
    	;npix_cap = 15
        factnp = 720/adsec.n_actuators
        npix_cap = (ceil(factnp) > 2)
    	;if n_elements(capsens) eq 0 then $
    	capsens = shift(dist(npix_cap),npix_cap/2,npix_cap/2) le npix_cap/2
    	act_coord=transpose(xol_data.act_coord)/(adsec_shell.out_radius*2)+rebin([0.5,0.5],adsec.n_actuators,2)
    	s = size(ima)
    	act_coord=act_coord*(min(s[1:2])-npix_cap)+npix_cap/2
    	n_true_act = n_elements(adsec.true_act)
        for ii=0,n_true_act-1 do begin
            i=adsec.true_act[ii]
            x_pos = round(act_coord[0,i]-npix_cap/2)
            y_pos = round(act_coord[1,i]-npix_cap/2)
            ima[x_pos,y_pos] = act_color[i]*capsens
        endfor
    endelse
end

