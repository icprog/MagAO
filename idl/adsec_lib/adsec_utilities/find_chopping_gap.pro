file_shell_ts3 ='/mnt/work/AO2.0/idl/adsec672a/conf/shell_TS3.txt' 
;file_ss_ts3 = '/mnt/work/AO2.0/idl/adsec672a/meas/status/2008_01_23/Status_60deg_p0.09_d0.3.sav'
;file_ss_ts3 = '/mnt/work/AO2.0/idl/adsec672a/meas/status_save/2008_01_28/status_save_80um_0.03p.sav'
file_ss_ts3 = '/mnt/work/AO2.0/idl/adsec672a/status_save_70um_generated.sav'
file_ss_ts3= '/mnt/work/AO2.0/idl/adsec672a/meas/status_save/2008_06_16/status_save_TS3_70um_INTp0.09d0.25_EXTp0.09d0.25.sav'
;file_ss_ts1 = '/mnt/work/AO2.0/idl/adsec672a/meas/status_save/2008_04_24/status_save_TS1_pist_calibration_b.sav'
file_ss_ts1 = '/mnt/work/AO2.0/idl/adsec672a/meas/status_save/2008_04_24/status_save_TS1_70um_pist_calibration_b.sav'
file_ss_ts1 = '/mnt/work/AO2.0/idl/adsec672a/meas/status_save/2008_04_29/status_save_TS1_70um_pist_calibration_d_zp.sav'
shell_ts3 = read_ascii_structure(file_shell_ts3)
restore, file_ss_ts3, /ver
adsec_ts3 = status_save.adsec
ss3 = status_save

;Zernike FIT
dim = 256
shell_x = mk_vector(dim, -1, 1) 
shell_xx = rebin(shell_x, dim, dim)

shell_y = transpose(mk_vector(dim, -1, 1)) 
shell_yy = rebin(shell_y, dim, dim)
shell_rr = sqrt(shell_xx^2+shell_yy^2)
pupil =(shell_rr gt shell_ts3.in_radius/shell_ts3.out_radius ) * (shell_rr lt 1)
idx = where(pupil, np)
tilt_img = fltarr(long(dim)*dim)

tiltm = 2
for i=1L, np-1 do tilt_img[idx[i]] = zern(tiltm, shell_xx[idx[i]], shell_yy[idx[i]])
tilt_img = reform(tilt_img, dim, dim) / max(tilt_img)

;40 [um] al bordo (2.5 arcsec + 1.5 arcsec per AO e FS)
tilt_img *= 40e-6
pxs = 1./2./(shell_ts3.out_radius)*float(dim)
pys = 1./2./(shell_ts3.out_radius)*float(dim)
xx_in =  (ss3.adsec.act_coordinates[0,*] + shell_ts3.out_radius)*pxs
yy_in =  (ss3.adsec.act_coordinates[1,*] + shell_ts3.out_radius)*pys
tilt_on_act = interpolate(tilt_img, xx_in, yy_in)
restore, file_ss_ts1, /ver
ss1 = status_save

newpos = (ss1.sys_status.position+tilt_on_act)
window, /free
display, newpos[ss1.adsec.act_w_cl], ss1.adsec.act_w_cl, /sh, TITLE='TS1:TILT#1: final position theorical'

sat_pos = ss1.adsec.d0/(2.-ss1.adsec.c_ratio)
newpos_diff = (ss1.sys_status.position+tilt_on_act-sat_pos)
idd = where(newpos_diff gt 0)
;newpos_diff[idd] = 0
window, /free
display, (newpos_diff[ss1.adsec.act_w_cl]) , ss1.adsec.act_w_cl, /sh, TITLE='TS1:TILT#1: distance from capsens sat'

newpos_n = (ss1.sys_status.position-tilt_on_act)
window, /free
display, newpos_n[ss1.adsec.act_w_cl], ss1.adsec.act_w_cl, /sh, TITLE='TS1:TILT#2: final position theorical'

newpos_diff_n = (ss1.sys_status.position-tilt_on_act-sat_pos)
idd = where(newpos_diff_n gt 0)
;newpos_diff_n[idd] = 0

window, /free
display, (newpos_diff_n[ss1.adsec.act_w_cl]) , ss1.adsec.act_w_cl, /sh, TITLE='TS1:TILT#2, distance from capsens sat'
print, minmax(newpos[ss1.adsec.act_w_pos])
print, minmax(newpos_n[ss1.adsec.act_w_pos])




sat_pos3 = ss3.adsec.d0/(2.-ss3.adsec.c_ratio)
newpos = (ss3.sys_status.position+tilt_on_act)
window, /free
display, newpos[ss3.adsec.act_w_cl], ss3.adsec.act_w_cl, adsec_save = ss3.adsec, adsec_shell_save = shell_ts3,/sh, TITLE='TS3:TILT#1: final position theorical'

newpos_diff = (ss3.sys_status.position+tilt_on_act-sat_pos3)
idd = where(newpos_diff gt 0)
;newpos_diff[idd] = 0
window, /free
display, (newpos_diff[ss3.adsec.act_w_cl]) , ss3.adsec.act_w_cl, adsec_save = ss3.adsec, adsec_shell_save = shell_ts3,/sh, TITLE='TS3:TILT#1: distance from capsens sat'

newpos_n = (ss3.sys_status.position-tilt_on_act)
window, /free
display, newpos_n[ss3.adsec.act_w_cl], ss3.adsec.act_w_cl, adsec_save = ss3.adsec, adsec_shell_save = shell_ts3, /sh, TITLE='TS3:TILT#2: final position theorical'

newpos_diff_n = (ss3.sys_status.position-tilt_on_act-sat_pos3)
idd = where(newpos_diff_n gt 0)
;newpos_diff_n[idd] = 0

window, /free
display, (newpos_diff_n[ss3.adsec.act_w_cl]) , ss3.adsec.act_w_cl, adsec_save = ss3.adsec, adsec_shell_save = shell_ts3, /sh, TITLE='TS3:TILT#2, distance from capsens sat'
print, minmax(newpos[ss3.adsec.act_w_pos])
print, minmax(newpos_n[ss3.adsec.act_w_pos])


end




