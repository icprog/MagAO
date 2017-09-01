function map_act_merit, p

    common ablock, out_radius, radius, act_coord, f_coord, xr, yr, xpos, ypos, flattened_mask

    RMat = rot_mat(p[0], 3)
    RMat = RMat[0:1, *]
    RMat = Rmat[*,0:1]
    act_coord = transpose(RMat ## transpose(f_coord)/out_radius*radius*p[1])
    if p[2] gt 0 then act_coord[1,*] = -act_coord[1,*]
    if p[3] gt 0 then act_coord[0,*] = -act_coord[0,*]

    act_coord[0,*] += xr
    act_coord[1,*] += yr

    dx = act_coord[0,*] - xpos
    dy = act_coord[1,*] - ypos

    tvscl, flattened_mask
    plots, act_coord[0,*], act_coord[1,*], /DEV, col = '00FF00'xl, psym=4, thick=2
    plots, xpos, ypos, psym=2, col=255l, /DEV, thick=2
    merit =  total(sqrt(dx^2+dy^2))/n_elements(xpos)
    
    print, [p, merit]

    return, merit 

end

function map_act, act_list

    @adsec_common
    common ablock, out_radius, radius, act_coord, f_coord, xr, yr, xpos, ypos, flattened_mask
    out_radius = adsec_shell.out_radius

   ; file = dialog_pickfile()
    close_flat = '/home/labot/work/AO/idl/adsec672a/meas/flat/2009_03_16/flat_data_60um_000_200b_modes.sav'
    file = '/home/labot/work/AO/idl/adsec672a/meas/calibration/2009_03_24/clockdata.sav'
    restore, close_flat, /ver
    restore, file, /ver
    
    fact = 1d
    
;    feat_fit = pupilfit(fullmask, /DISPLAY)
    feat_fit =  [497.011,520.275,461.568,-0.102177]
    feat_fit /= fact
    xpos = xroi/fact
    ypos = yroi/fact
    print, feat_fit
    
    xr = feat_fit[0]
    yr = feat_fit[1]
    radius = feat_fit[2]
    eps = feat_fit[3]
    f_coord = adsec.act_coordinates[*, act_list]

;    sopd = size(fullmask, /DIM)
;    xx = rebin((dindgen(sopd[0])-feat_fit[0])/(feat_fit[2]), sopd)
;    yy = rebin((dindgen(1,sopd[1])-feat_fit[1])/(feat_fit[2]), sopd)
;    idx_m = where(fullmask)

;    for i=0, n_elements(act_list)-1 do begin
;    
;        curr_frame=frames[i,*,*]
;        fit = surf_fit(xx[idx_m], yy[idx_m], double(curr_frame[idx_m]), indgen(120)+1, COEFF=coeff, /ZERN)
;        result= curr_frame*0.0
;        result[idx_m] = curr_frame[idx_m]-fit
;        if i eq 0 then fullframe = result else fullframe += result
;
;        stop
;    endfor 


    sopd = size(flattened_opd, /dim)
    window, 0, xs = sopd[0], ys = sopd[1]
;    tvscl, flattened_mask
;    plots, f_coord[0,*], f_coord[1,*], /DEV, col = '00FF00'xl, psym=4
;    plots, xpos, ypos, psym=2, col=255l, /DEV


    ;PARAMETER FIT: rotation, distortion, flipx, flipy
    par = [-!dpi/2, 1d, 0d, 0d]
    scale = [!dpi/2, 0.01, 0, 0, 0]
    fit = amoeba(1e-6, FUNCTION_NAME='map_act_merit', NMAX=1000, P0=par, SCALE=scale)

    save, file='tmp.sav', fit


    return, fit


End
