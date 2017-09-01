pro comp_s_k_global
@sigma_common
restore, stress.basedir+"/fea_stress.sav", /VERB

if n_elements(sigma_modal) eq 0 then begin
    sigma = temporary(sigma_zonal)
    delta = temporary(delta_zonal)
endif
if n_elements(sigma_zonal) eq 0 then begin
    sigma = temporary(sigma_modal)
    delta = temporary(delta_modal)
endif

;for i = 0L,  n_elements(sigma_modal[*,0])-1 do begin
;    fr = dblarr(ngrid*ngrid,stress.stress_n)
;    fr[idxpupil,*] = sigma_modal[i,*]
;    fr = reform(fr,ngrid,ngrid,stress.stress_n)
;    fr = transpose(fr,[1,0,2])
;    sigma_modal[i,*] = (reform(fr,ngrid*ngrid,stress.stress_n))[idxpupil,*]
;endfor

if n_elements(sigma_zonal) ne 0 and n_elements(sigma_modal) ne 0 then begin
    sigma = [temporary(sigma_modal),temporary(sigma_zonal)]
    delta = [temporary(delta_modal),temporary(delta_zonal)]
endif

inv_delta = pseudo_invert(delta, w_vec=w, u_mat=u, v_mat=v, INV_W=inv_w,IDX_ZEROS=idx,COUNT_ZEROS=count,/VERBOSE)
s_k_mat = sigma ## inv_delta
;svdc, s_k_mat, w_s_k_mat, u_s_k_mat, v_s_k_mat, /DOUBLE

save, s_k_mat, inv_delta, ngrid, idxpupil, stress_n, FILE=stress.basedir+"/s_k_global.sav"

end
