Pro find_cal_zo_zern, dz
path = "~/idl/adsec672a/meas/status_save/"
;pathd = path+'2008_04_23/'
pathd = path+'2008_04_28/'
restore, pathd+"status_save_TS1_70um_pist_calibration_c_pre_zp.sav", /VERB
restore, path+"bad_calib_act.sav", /VERB
do_save = 1B

adsec_save=status_save.adsec
act_w_pos = status_save.adsec.act_w_pos

act_w_pos_save = act_w_pos
bad_calib_act_save = bad_calib_act

if bad_calib_act[0] ne -1 then begin
        print, intersection(bad_calib_act, adsec.true_act , true_bad_calib_act) 
        print, intersection(true_bad_calib_act, act_w_pos , bad_calib_act_w_pos) 
        print, complement(bad_calib_act_w_pos,act_w_pos, pp)
	act_w_pos = pp
endif
pos = status_save.sys_status.position

window,0
display, pos[act_w_pos], act_w_pos, /SH, /NO_NUM, /SM

pp = fltarr(adsec.n_actuators)
x = adsec.act_coordinates[0,act_w_pos]
y = adsec.act_coordinates[1,act_w_pos]
radial_ave_pos = fltarr(adsec_shell.n_rings)
for i=0,adsec_shell.n_rings-1 do begin
	dr = (adsec_shell.ring_radius[1]-adsec_shell.ring_radius[0])/2
	idx = where(abs(sqrt(x^2+y^2)-adsec_shell.ring_radius[i]) lt dr)
	act_list = act_w_pos[idx]
	pp[act_list] = i
	radial_ave_pos[i] = mean(pos[act_list])
endfor

window,1
plot, adsec_shell.ring_radius, radial_ave_pos

z = pos[act_w_pos]
xn=x/adsec_shell.out_radius
yn=y/adsec_shell.out_radius
xfn = adsec.act_coordinates[0,act_w_pos_save]/adsec_shell.out_radius
yfn = adsec.act_coordinates[1,act_w_pos_save]/adsec_shell.out_radius
nrad = 5
j_list = indgen((nrad+1)*(nrad+2)/2+1)+1
fitf = surf_fit(xn, yn, z, j_list, /ZERN, COEFF=coeff, ZOUT=[xfn,yfn])
fit_all = fltarr(adsec.n_actuators)
fit_all[act_w_pos_save]=fitf
fit = fit_all[act_w_pos]
res = pos[act_w_pos]-fit
window, 2
display, res, act_w_pos, /SH, /NO_NUM

window, 3
display, fitf, act_w_pos_save, /SH, /NO_NUM, /SM

window, 4
plothist, pos[act_w_pos]-fit, BIN=1e-6

idx = where(res gt 7e-6, count)
if count ne 0 then begin
	bad_calib_act = act_w_pos[idx]

	pp = fltarr(adsec.n_actuators)
	pp[bad_calib_act]=1
	window, 5
	display, pp, /NO_NUM
endif

if bad_calib_act_save[0] eq -1 then count_save=0 else count_save=n_elements(bad_calib_act_save)

print,"Number of bad actuators:", count_save
print,"Number of bad actuators to add:", count
if count ne 0 and do_save then begin
	bad_calib_act = [bad_calib_act,bad_calib_act_save]
	bad_calib_act = bad_calib_act[sort(bad_calib_act)]
   save, bad_calib_act, file=pathd+'bad_calib_act.sav"
   print, "New list of bad actuators saved"
endif

d0_new = adsec_save.d0
dz = fltarr(adsec.n_actuators)
for i=0, n_elements(bad_calib_act)-1 do begin
    dummy = where(adsec.act_wo_pos eq bad_calib_act[i], cc)
    if cc gt 1 then message, "ERROR"
    if cc ne 1 then begin
        d0_new[bad_calib_act[i]] = adsec_save.d0[bad_calib_act[i]]*fit_all[bad_calib_act[i]]/pos[bad_calib_act[i]]
        dz[bad_calib_act[i]] = pos[bad_calib_act[i]]-fit_all[bad_calib_act[i]]
        endif
endfor
pm = meas_path('calibration')
save, dz, d0_new, status_save, bad_calib_act, file=pathd+"d0_zern_calib.sav"
openw, 1, pm+"d0_zern_calib.txt"
printf, 1, "float "+strtrim(n_elements(d0_new),2)
printf, 1, transpose(d0_new)
close, 1
return
end
