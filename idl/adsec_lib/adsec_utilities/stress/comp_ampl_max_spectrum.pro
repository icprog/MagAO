pro comp_ampl_max_spectrum, stress_ampl_max, force_ampl_max, int_force_ampl_max, stroke_ampl_max, $
    stress_max_XY, stress_dist_min, stress_near_act, max_force_act, $
    STRESS_THRESHOLD=threshold, FORCE_THRESHOLD=fthreshold, INT_FORCE_THRESHOLD=ifthreshold, $
    STROKE_THRESHOLD=sthreshold

;+
; NAME:
;   comp_ampl_max_spectrum
;
; PURPOSE:
;   Compute the spectrum of the maximum modal amplitude for:
;   1) (stress_ampl_max) a given stress threshold,  
;   2) (force_ampl_max) an actuator force threshold (F on i-th actuator < force_threshold for each i)
;   3) (int_force_ampl_max) a given integral force ( Sum(Fi) < int_force_threshold )
;   4) (stroke_ampl_max) a given actuator stroke (Stroke-ith < stroke_threshold for each i)
;
; SYNTAX:
;   pro comp_ampl_max_spectrum, stress_ampl_max, force_ampl_max, int_force_ampl_max, stroke_ampl_max, $
;    stress_max_XY, stress_dist_min, stress_near_act, max_force_act, $
;    STRESS_THRESHOLD=threshold, FORCE_THRESHOLD=fthreshold, INT_FORCE_THRESHOLD=ifthreshold, $
;    STROKE_THRESHOLD=sthreshold, _EXTRA=e
;
; INPUTS:
;
; OUTPUTS:
;   stress_ampl_max     : spectrum of max modal rms amplitude (m) for a given tensile stress threshold
;   force_ampl_max      : spectrum of max modal rms amplitude (m) for a given force threshold
;                         on a single actuator
;   int_force_ampl_max  : spectrum of max modal rms amplitude (m) for a given integral force threshold
;   stroke_ampl_max     : spectrum of max modal rms amplitude (m) for a given stroke threshold 
;                         on a single actuator
; 
;   stress_max_XY       : spectrum of positions of max tensile stress on the shell [m,m]
;   stress_dist_min     : spectrum of distance between the point of max tensile stress and its nearest
;                         actuator.
;   stress_near_act     : spectrum of index [0:3*n_act] of the nearest actuator
;                         to the point of maximum stress. See HISTORY (20 may 2005)
;                         for a better description.
;   max_force_act:      : spectrum of the actuators the exerts the max abs(force)
;
;
;
; KEYWORDS:
;   STRESS_THRESHOLD:       Stress threshold [Pa]. Default is stress.stress_thre.
;   FORCE_THRESHOLD:        Force threshold of a single actuator [N]. Default is stress.force_act_thre.
;   INT_FORCE_THRESHOLD:    Force threshold [N]. Default is stress.int_force_thre.
;   STROKE_THRESHOLD:       Stroke threshold [m]. Default is stress.stroke_thre.
;
; HISTORY:
;    apr 2005 created by lorenzo busoni
; 13 may 2005 l.b. Added XYmax, Dmin, NearAct
; 20 may 2005 l.b. Dmin is now the distance to the nearest droplet of glue
;                  and NearAct is a [0:2015] vector. A NearAct value
;                  in [0:671] means that the maximum stress is on the North 
;                  droplet of glue. A NearAct value in [672:1343] means that
;                  the maximum stress is on the SE droplet, while in the 
;                  range [1344:2015] the maximum stress is nearest to the SW
;                  droplet of glue  
; 20 dec 2005 l.b. Improved help and added keyword THRESHOLD and SAVEIT
; 16 may 2006 l.b. deep revision for new stress matrix in s_k_global.sav
; 18 jun 2007 l.b. Improved help
;
;-
@sigma_common

stress_thre    = stress.stress_thre
force_thre     = stress.force_act_thre
int_force_thre = stress.int_force_thre
stroke_thre    = stress.stroke_thre

if keyword_set(threshold) then begin
    stress_thre=threshold
endif

if keyword_set(fthreshold) then begin
    force_thre=fthreshold
endif

if keyword_set(ifthreshold) then begin
    int_force_thre=ifthreshold
endif

if keyword_set(sthreshold) then begin
    stroke_thre=sthreshold
endif

nmodes = n_elements(stress.w_k_mat)
stress_ampl_max      = dblarr(nmodes)
force_ampl_max       = dblarr(nmodes)
int_force_ampl_max   = dblarr(nmodes)
stroke_ampl_max      = dblarr(nmodes)
stress_max_xy        = dblarr(nmodes,2) 
stress_dist_min      = dblarr(nmodes) 
stress_near_act      = lonarr(nmodes)
max_force_act        = lonarr(nmodes)
for modo=0,nmodes-1 do begin
    mode_vector  = dblarr(nmodes)
    mode_vector[modo] = 1

    ; stress
    sigma_shape, S, mode_vector, MAX=XY, DMIN=D, NACT=nAct, /NOSHOW
	stress_ampl_max[modo]   = stress_thre/max(S)
	stress_max_xy[modo,*]   = XY
	stress_dist_min[modo]   = D
	stress_near_act[modo]   = nAct

    ; force
	force_shape, S, mode_vector, MAX=max, NACT=nAct, /NOSHOW
    int_force_ampl_max[modo] = int_force_thre/max( [total(S[where(S gt 0)]), -total(S[where(S gt 0)])])
	force_ampl_max[modo]     = force_thre/max
	max_force_act[modo]      = nAct
    
    ; stroke
    deltan = stress.v_k_mat ## mode_vector * sqrt(nmodes)
    stroke_ampl_max[modo] = stroke_thre/max(abs(deltan))
endfor

save, stress_ampl_max, force_ampl_max, int_force_ampl_max, stroke_ampl_max, $
      stress_max_xy, stress_dist_min, stress_near_act, max_force_act, $
      stress_thre, force_thre, int_force_thre, stroke_thre, $
      file=stress.basedir+'/ampl_max_spectrum.sav'

; plot spectra
p_back = !p
!p.color=0
!p.background='ffffffff'xul
!P.THICK = 3.0
!X.THICK = 2.5
!Y.THICK = 2.5
!P.CHARSIZE = 2.5
!P.CHARTHICK = 1.5
!P.FONT = 1
window, /free, xs=800, ys=600, retain=2


data = [ stress_ampl_max, force_ampl_max, int_force_ampl_max, stroke_ampl_max ] 

plot, [0, n_elements(stress_ampl_max)] , [min(data), max(data)] , /ylog, $
      psym = 3, symsize=0.001, xst=2, yst=18, ytitle='Max modal amplitude [m rms]', xtitle='# mode', $
      title=''
oplot, stress_ampl_max,        psym=-4, symsize=1.0
oplot, force_ampl_max,         psym=-4, symsize=1.0, col=255L
oplot, int_force_ampl_max,     psym=-4, symsize=1.0, col=255L*256
oplot, stroke_ampl_max,        psym=-4, symsize=1.0, col=255L*256*256

xyouts, 20,1000, STRING(FORMAT= '(%"Stress < %g MPa\n")',stress_thre/1e6) 
xyouts, 20,100,  STRING(FORMAT= '(%"Force < %g N\n")',force_thre), col=255L
xyouts, 20,10,   STRING(FORMAT= '(%"Integrated force < %g N\n")',int_force_thre), col=255L*256
xyouts, 20,1,    STRING(FORMAT= '(%"Stroke < %g um\n")',stroke_thre*1e6) , col=255L*256*256

write_jpeg, stress.basedir+'/ampl_max_spectrum.jpeg' , tvrd(true=3), quality=95, true=3

!p = p_back


return 
end




