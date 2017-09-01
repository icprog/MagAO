file=dialog_pickfile()
print, file
restore, file, /ver

res = df-ff_matrix##dx
ncl = n_elements(cl_act)
vv = fltarr(ncl)
nit = nmeas/ncl
for i=0,nit-1 do vv += (extract_diagonal(res[i*ncl:(i+1)*ncl-1,*]))^2/nit^2
vv /= sqrt(nit)
ids = (reverse(sort(vv)))[0:10]

window, /free
display, vv, cl_act, TITLE='Mean RMS residue on diagonal', BAR_TITLE='[N]', /SH, /NO_N
window, /free
plot, cl_act, vv, psym=-4, title = 'Mean RMS residue on diagonal', YTITLE='[N]', XTITLE='ACT'
plots,cl_act[ids], vv[ids], psym=4, col=255L
print, [transpose(vv[ids]), transpose(cl_act[ids])]
End
