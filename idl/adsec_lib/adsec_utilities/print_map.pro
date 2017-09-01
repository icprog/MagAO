;xs, ys size in inches

pro print_map

    ;Print actuator map in dsp mode and mirror mode
    
    xs = 1440
    ys = 1440

    window, 0, xs=xs, ys=ys, /PIX
    display, fltarr(672)+'ffffff'xl, num_type=1,  min_v='fffffe'xl, /no_b, num_charthick=1.5, num_charsize=1, TITLE='MIRROR MAP', back='ffffff'xl, col='ffffff'xl
    write_jpeg, 'lbt672_mirror_map.jpg', tvrd(true=3), quality=100, true=3

    display, fltarr(672)+'ffffff'xl,  num_type=0, min_v='fffffe'xl, /no_b, num_charthick=1.5, num_charsize=1, TITLE='DSP MAP', back='ffffff'xl, col='ffffff'xl
    write_jpeg, 'lbt672_dsp_map.jpg', tvrd(true=3), quality=100, true=3
    wdelete, 0


end
