
pro force, mm2c, ff_matrix, rms, p, NOPLOT=NOPLOT


acts = findgen(672)
p = fltarr(672)
rms = fltarr(672)

n_modes = n_elements(mm2c[*,0])

for modo=0,n_modes-1 do begin
  m = fltarr(n_modes)
  m[modo]=1e-7
  c = m # mm2c
  f = c # ff_matrix
  p[modo] = max(abs(f[acts]))
  rms[modo] = stddev(f[acts])
endfor

if not keyword_set(NOPLOT) then plot,rms
end
