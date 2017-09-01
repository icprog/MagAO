pro test_fit, x

    ;actuators map on the shell frame
    coord = map_act(image)
    data = frames(coord, *)
    
    p=[97.5d, 48d-6, -0.10d, !dpi/2]
    ;;; data_cut non funziona con tutti i max e min
    y = cos_func(double(x),p)
    time_xx = readfits('./time_xx.fits')
    time_yy = time_xx
    time_xx2 = max(time_xx) * (dindgen(100000))/9999d   

    param = data_cut(x, y, time_xx, time_xx)

    
end
