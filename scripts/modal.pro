
; modal, '20110704_pureKLs_01/pureKLs_TTf_plusTTmodes_idx2.sav', 'MMmatrix_noslaves_idx2.sav', 'idx2.fits', 'mm2c_MMmatrix_noslaves_20110704.fits', wuv_file='MMmatrix_noslaves_idx2_wuv.sav', mmminv_file='MMmatrix_noslaves_idx2_mmminv.sav'

; modal, '20110704_pureKLs_01/pureKLs_TTf_plusTTmodes_idx2.sav', 'idx2.fits', '4d/IFmatrix.sav','4d/idx.fits','mm2c_MMmatrix_noslaves_20110704.fits', wuv_file='4d/IFmatrix_wuv.sav',tokeep=findgen(571), mmminv_file='4d/IFmatrix_inv.sav'

pro  modal, klmodes_file, klidx_file, mmmodes_file, idx_file, mm2c_file, wuv_file=wuv_file, mmminv_file =mmminv_file, tokeep=tokeep, cmd

restore, klmodes_file, /v
restore, mmmodes_file, /v
idx = readfits(idx_file)
klidx = readfits(klidx_file)
mm2c = readfits(mm2c_file)

if keyword_set(ifmatrix) then mmmatrix = ifmatrix

modo = modemat[4,*]
x = klidx mod 217
y = klidx / 217
res = tri_surf(modo, x, y, nx=217, ny=217)

modo = res[idx]

if keyword_set(wuv_file) then begin
	 restore, wuv_file, /v
endif else begin
	la_svd, mmmatrix, w, u, v
endelse
stop

u2 = u[ tokeep,*]
c = u2 # modo
wf = c # u2     
f5 =fltarr(217,217)
f5[idx] = wf
tvscl,rebin(f5,217*3,217*3,/sam)

if keyword_set(mmminv_file) then begin
	restore, mmminv_file, /v
endif else begin
	mmminv = pseudo_invert(mmmatrix, n_modes_to_drop=1)
endelse

cc = wf # mmminv

;mm2c_fixed = mm2c[0:563,*]
mm2c_fixed = mm2c[ tokeep,*]

cmd = cc # mm2c_fixed

stop
end
