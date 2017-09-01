
; Visualizzazione autogain

pro visualizza
common autogain, gains, first

window,1,retain=2
np = 20
mn = min(gains[2,*])
mx = max(gains[2,*])
step = float(mx-mn)/np
device,decomposed=0
loadct,10
contour,reform(gains[2,*]),reform(gains[0,*]),reform(gains[1,*]),/irregular,/fill,/iso,levels=findgen(np)*step+mn

end


function meritfunc, v
common autogain, gains, first

if v[0] lt 0 then v[0]=0
if v[0] gt 1 then v[0]=1

if v[1] lt 0 then v[1]=0
if v[1] gt 1 then v[1]=1

print,'Gain: ',v

gainv = fltarr(672)
gainv[0:1] += v[0]
gainv[2:*] += v[1]
filename = '/tmp/gain_tt'+strtrim(v[0],2)+'_ho'+strtrim(v[1],2)+".fits"
print,filename
writefits,filename,gainv
errcode = set_ao_gain(filename)

wait,12

errcode = read_var_wrap('optloopdiag.R.SLOPERMS@M_FLAOWFS', vv, 1000)
vv[0] = fix(vv[0]*1000)/1000.0
print,'Score: ',vv[0]

g = [v,vv[0]]
if first eq 1 then gains = g $
else gains = [[gains],[g]]

visualizza
return, vv[0]
end

pro autogain, CLEARHIST = CLEARHIST
common autogain, gains, first

gainvector = [0.3, 0.3]
nmax = 500
ftol = 0.001
scale = [0.05,0.05]

if (n_elements(gains) eq 0) or (keyword_set(CLEARHIST)) then first = 1


r = amoeba( ftol, FUNCTION_NAME = 'meritfunc', $
              FUNCTION_VALUE =  vv, $
              NCALLS = ncalls, $
              NMAX = nmax, $
              P0 = gainvector, $
              SCALE = scale)

if n_elements(r) eq 1 then message,'Amoeba failed to converge'

print,'minimum=',r

end


