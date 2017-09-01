pro fit_atmo_layers, atmofile, n_layer, show=show
;+
; NAME:
;   fit_atmo_layers
;
; PURPOSE:
;   Given a file containing atmospheric profiles (see gen_layers), fit the mirror to the wavefront
;   and save the actuators' position. 
;
; SYNTAX:
;
; INPUTS:
;
; OUTPUTS:
;
; KEYWORDS:
;
; HISTORY:
;    jun 2007 created by lorenzo busoni
;
;-
@sigma_common
@adsec_common

    pupil_magn = tele.r_out / (adsec_shell.out_radius/1000.)
    acts_coord = transpose(stress.act_xy)
    layer = restore_layers(atmofile, 0, tele.lambda, tele.r0, par=paratm);
    pxscale = paratm.length/paratm.dim
    act_pos_px = rebin([paratm.dim/2, paratm.dim/2], 2, adsec_shell.n_true_act) + acts_coord * pupil_magn / pxscale
    
    commands = dblarr(n_layer, adsec_shell.n_true_act)
    for i = 0, n_layer-1 do begin
        print, 'Analyzing layer ',i
        layer = restore_layers(atmofile, i, tele.lambda, tele.r0)
        commands[i,*] =  reform(layer[act_pos_px[0,*], act_pos_px[1,*]]) / (2*!dpi) * tele.lambda / 2.   ; /2 because of reflexion
        commands[i,*] -= mean(commands[i,*])
        if keyword_set(show) then plot, commands[i,*]
    endfor
    
    save, commands, pxscale, act_pos_px, pupil_magn, n_layer, atmofile, file=stress.basedir+'/fitted_commands.sav'
end
