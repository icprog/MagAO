pro test_fast, M2C=mm, block=block
@adsec_common
if not keyword_set(block) then block=0
err = clear_dl()
err= write_same_ch(-1, dsp_map.dist_accumulator,0l)
err= write_same_ch(-1, dsp_map.curr_accumulator, 0l)
;err= write_same_ch(-1, dsp_map.bias_command, 0.05)
gs = randomn(seed,adsec.n_actuators)*0+1
err = set_g_gain(sc.all, gs, block=block)

;bi = randomn(seed,rtr.n_slope, adsec.n_actuators, rtr.n_slope_delay)
bi = fltarr(rtr.n_slope, adsec.n_actuators, rtr.n_slope_delay)
err = set_b_delay_matrix(bi, block=block)

                                ;set matrici ai casuali
;ai = randomn(seed,adsec.n_actuators, adsec.n_actuators, rtr.n_slope_delay)
ai = fltarr(adsec.n_actuators, adsec.n_actuators, rtr.n_modes_delay)
err = set_a_delay_matrix(ai)

b0 = [identity(48.0),fltarr(rtr.n_slope-48, adsec.n_actuators)]
err = set_b0_matrix(b0, block=block)

;mm = identity(48.0)*2
modo = adsec.ff_p_svec[0,*]
sl = findgen(1,rtr.n_slope)
mf = diagonal_matrix(gs) ## (b0 ## sl)
tmf = transpose(mf)
svdc, (mf ## tmf), wm, um, vm, /double
id = where(wm le (machar()).eps*max(wm), cc, complement=comp)
inv = fltarr(adsec.n_actuators)
inv[comp]=1/wm[comp]
inv_mat = vm ## diagonal_matrix(inv) ## transpose(um)

mm = float(modo ## transpose(mf) ## inv_mat )
mm = mm * 1e-6
err = set_m2c_matrix(mm)
;err = set_cmd2bias()

end
