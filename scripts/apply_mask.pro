
; apply_mask, 'idx2.fits', 'MMmatrix_noslaves_20110704.sav', '20110704_pureKLs_01/pureKLs_TTf_plusTTmodes.sav', 'MMmatrix_noslaves_idx2.sav', '20110704_pureKLs_01/pureKLs_TTf_plusTTmodes_idx2.sav'

pro apply_mask, newmask_file, mmmatrix_file, klmatrix_file, outmmmatrix_file, outklmatrix_file

newmask = readfits(newmask_file)

restore, mmmatrix_file, /v
restore, klmatrix_file, /v

f =fltarr(217,217)
n_modes = n_elements(modemat[*,0])
newmodemat = dblarr(n_modes, n_elements(newmask))

for i=0,n_modes-1 do begin
   f[idx_mask] = modemat[i,*]
   newmodemat[i, *] = f[newmask]
endfor

modemat = newmodemat

save, modemat, file=outklmatrix_file



f =fltarr(217,217)
n_modes = n_elements(mmmatrix[*,0])
newmmmatrix = dblarr(n_modes, n_elements(newmask))

for i=0,n_modes-1 do begin
   f[idx_mask] = mmmatrix[i,*]
   newmmmatrix[i, *] = f[newmask]
endfor

mmmatrix = newmmmatrix

save, mmmatrix, file=outmmmatrix_file

end
