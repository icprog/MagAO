
pro sigma_shape, P, mode_vector, out_img,  MAX=XY, STRESS=type, NOSHOW=noshow, $
                 NACT=nact, DMIN=dmin, POS=pos
;+
; NAME:
;   sigma_shape;
; 
; PURPOSE:
;   Compute the stress matrix for a given shell shape.
;
; SYNTAX:
;   sigma_shape, P, mode_vector, STRESS=type, MAX=XY, /POS, /NOSHOW, NACT=nact, DMIN=dmin
; 
; EXAMPLE:
;   > sigma_shape, P, mode_vector, STRESS=1
;
; INPUTS:
;   mode_vector: stiffness modes vector amplitudes (m rms) [0:nmodes-1] or
;                actuator position (m) [0:nmodes-1] if POS is set
;  
; OUTPUTS:
;   P        : matrix of stresses (Pa)
;
; KEYWORDS:
;   MAX      : return the coordinates of the point of maximum stress (m)
;   STRESS   : which kind of stress to retrieve 
;              1=Sx_top, 2=Sx_bottom, 3=Sy_top, 4=Sy_bottom, 0=Max of all
;   DMIN     : return the distance from the point of maximum tensile stress to 
;              the nearest actuator (needs ACT_XYZ restored from 'fea.sav' or 
;              'act_xyz.sav'. If ACT_XYZ is not defined it tries to restore 
;              'act_xyz.sav' from the current directory)
;   NACT     : return the nearest actuator to point of maximum tensile stress
;   NOSHOW   : don't show stress shape
;   POS      : mode_vector is a command vector
;              
; 
; HISTORY:
;  21 Dec 2005: Created by L. Busoni lbusoni@arcetri.astro.it
;  04 May 2006: (lb) DMIN and NACT keyword added, mode_vector in meters          
;  16 May 2006: (lb) completely rewritten for new stress matrix in s_k_global.sav
; 
;-
if (N_PARAMS() ne 2) then begin
    print, 'Usage: sigma_shape, P, mode_vec, STRESS=type, MAX=XY, DMIN=dmin, NACT=nact, /POS /NOSHOW' 
    print, '  P       (out): stress matrix [Pa]'
    print, '  mode_vec(in) : modal amplitudes (m rms) [0:n_modes-1] '
    print, '  type    (in) : 1=Sx_top, 2=Sx_bottom, 3=Sy_top, 4=Sy_bottom, 0=max of all'
    print, '  XY      (out): coords of the point of maximum stress (m)'
    print, '  DMIN    (out): distance from the point of maximum stress to the nearest actuator' 
    print, '  NACT    (out): nearest actuator to the point of maximum tensile stress'
    return
endif

@sigma_common
@adsec_common

;if n_elements(v_k_mat eq 0) then restore, basedir+'/k_mat.sav', /v
;restore, basedir+'/s_k_global.sav', /v
;restore, basedir+'/adsec.sav', /v

n_modes = n_elements(stress.w_k_mat)
pupilsz = n_elements(stress.idxpupil)

if (n_elements(mode_vector) ne n_modes) then begin
    message, "Wrong size for mode_vector. Need "+strtrim(n_modes,2)+" elements"     
endif

if keyword_set(POS) then pos_vector = mode_vector $
else pos_vector = stress.v_k_mat ## mode_vector * sqrt(n_modes)

if test_type(c_s_k_mat, /float) eq 0 then pos_vector = float(pos_vector)

stress_vector = c_s_k_mat ## pos_vector
sss = transpose(reform(stress_vector, pupilsz, stress.stress_n))

if (n_elements(type) eq 0) then begin 
    type = 0
    
endif
case type of
    0: sss =  max(temporary(sss), dim=1) 
    1: sss =  temporary(sss[0,*])       
    2: sss =  temporary(sss[1,*])      
    3: sss =  temporary(sss[2,*])     
    4: sss =  temporary(sss[3,*])    
endcase

if test_type(c_s_k_mat, /float) eq 0 then P = fltarr(stress.ngrid, stress.ngrid) $
else P = dblarr(stress.ngrid,stress.ngrid)

P[stress.idxpupil] = sss
;P = rotate(temporary(P),4)

minP = min(P)
maxP = max(P,Imax)
mingrid = -adsec_shell.out_radius / 1000.0
maxgrid =  adsec_shell.out_radius / 1000.0

if not keyword_set(noshow) then begin
    ; create the title is the most difficult thing
    case type of
        0: strtype='all'
        1: strtype='xt'
        2: strtype='xb'
        3: strtype='yt'
        4: strtype='yb'
    endcase
    title = STRING(FORMAT='(%"S%s")',strtype)

    xshow , abs(P), /SH, /AS, /LAB, /ZOOM, TITLE=title[0], $
        xax=[mingrid, maxgrid], yax=[mingrid, maxgrid]
endif 

    

XY = fltarr(2)
XY[0] = (Imax mod stress.ngrid)*(maxgrid-mingrid)/stress.ngrid  + mingrid
XY[1] = (Imax / stress.ngrid)*(maxgrid-mingrid)/stress.ngrid  + mingrid
D      = sqrt( (stress.ACT_GLUE[0,*] - XY[0])^2 + (stress.ACT_GLUE[1,*] - XY[1])^2)
dmin   = min(D,nAct)
print, 'Max: '+strtrim(maxP,2)+' in: ['+strtrim(xy[0],2)+','+strtrim(xy[1],2)+$
      '] - distance from act ['+strtrim(nAct,2)+']: '+strtrim(dmin,2) + ' - Imax: '+strtrim(Imax,2)
end

