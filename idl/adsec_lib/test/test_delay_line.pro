;script di test della delay line
;
err = clear_dl()

print, 'Test delay lines.'
sfd = fltarr(rtr.n_slope, rtr.n_slope_delay)
moded = fltarr(adsec.n_actuators, rtr.n_modes_delay)

;funziona se le linee di ritardo sono uguali con un wrap-around
for i=0,rtr.n_slope_delay do begin
    sf = randomn(seed, rtr.n_slope)
    sfd[*,i mod rtr.n_slope_delay] = sf
    err = write_same_dsp(sc.all,rtr_map.slope_vector, sf)
    mode = randomn(seed, adsec.n_actuators)
    moded[*,i mod rtr.n_modes_delay] = mode
    err = write_same_dsp(sc.all,rtr_map.modes_vector, mode)
    
        ;print, 'Setting the nendof to 1UL ...'
        ;err = set_nendof()
        print, 'Starting RTR #'+strtrim(string(i),2)
        err = start_rtr()
        print, 'Starting MM #'+strtrim(string(i),2)
        err = start_mm()
        print, 'Starting FF #'+strtrim(string(i),2)
        err = start_ff()
    wait, 0.5
        
endfor

err = read_seq_dsp(sc.all, rtr_map.modes_delay,adsec.n_actuators*rtr.n_modes_delay , dpd)
print, 'dpd',minmax(dpd)
err = read_seq_dsp(sc.all, rtr_map.slope_delay,rtr.n_slope*rtr.n_slope_delay , svd)
print, 'svd',minmax(dpd)
tmp = rebin(reform(moded,adsec.n_actuators*rtr.n_modes_delay),adsec.n_actuators*rtr.n_modes_delay,adsec.n_dsp_per_board*adsec.n_board_per_bcu, /samp)
uu = total(abs(dpd)-abs(tmp))

tmp = rebin(reform(sfd, rtr.n_slope*rtr.n_slope_delay),rtr.n_slope*rtr.n_slope_delay,adsec.n_dsp_per_board*adsec.n_board_per_bcu, /samp)
xx = total(abs(svd)-abs(tmp))

if (abs(uu)+abs(xx)) eq 0 then print, 'Test delay line ok!' else print, 'Test delay line failed.'

end
