
pro force_shape, F, mode_vector,  MAX=maxF, NOSHOW=noshow, NACT=nact, POS=pos
;+
; NAME:
;   force_shape;
; 
; PURPOSE:
;   Compute the force matrix for a given shell shape.
;
; SYNTAX:
;   force_shape, F, mode_vector, MAX= max, NACT=nact, /NOSHOW, /POS
; 
; INPUTS:
;   mode_vector: (modal/spatial) vector amplitudes (m rms) [0:nmodes-1] 
;  
; OUTPUTS:
;   F        : matrix of forces (N)
;
; KEYWORDS:
;   MAX      : return the maximum absolute value of force [N]
;   NACT     : return the actuator that exerts the maximum absolute value of force
;   NOSHOW   : don't show force pattern
;   POS      : mode_vector is interpreted as positions of the actuators 
;              
; 
; HISTORY:
;  16 May 2006: (lb) completely rewritten for new stress matrix in s_k_global.sav
; 
;-
if (N_PARAMS() ne 2) then begin
    print, 'Usage: force_shape, F, mode_vec,  MAX=max, NACT=nact, /NOSHOW, /POS' 
    print, '  F       (out): force matrix [N]'
    print, '  mode_vec(in) : modal amplitudes (m rms) [0:n_modes-1] '
    print, '  MAX     (out): return the maximum abs(force)'
    print, '  NACT    (out): return the actuator that exerts the maximum abs(force)'
    print, '  POS          : mode_vec is interpreted as actuators positions'
    return
endif

@sigma_common
@adsec_common

n_modes = n_elements(stress.w_k_mat)
pupilsz = n_elements(stress.idxpupil)

if (n_elements(mode_vector) ne n_modes) then begin
    message, "Wrong size for mode_vector. Need "+strtrim(n_modes,2)+" elements"     
endif

if keyword_set(POS) then pos_vector = mode_vector $
else pos_vector = stress.v_k_mat ## mode_vector * sqrt(n_modes)

F = stress.k_mat ## pos_vector

;minP = min(P, Imin)
maxF = max(abs(F), nact)

if not keyword_set(noshow) then begin
    display , F[adsec.act2mir_act], /SH, /AS, /SMO, TITLE='Force pattern [N]'  ;,$
        ;xax=[mingrid, maxgrid], yax=[mingrid, maxgrid]
endif

print, 'Max force: '+strtrim(maxF,2)+' on act '+strtrim(nAct,2)

end

