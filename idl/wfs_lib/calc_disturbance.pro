
;pro calc_disturbance, result, ff_matrix

    m2c_file = "/home/labot/idl/file2test/adsecP45/m2c.fits"
    outfile = "/home/labot/Supervisor/config/commands/disturbance.fits"
    ff_file = "/home/labot/idl/file2test/adsecP45/ff_matrix.sav"
    maxamp_file = "/home/labot/idl/file2test/adsecP45/max_modal_amplitude.fits"

    m2c = readfits(m2c_file)

    n_modes = 48
    n_act   = 48
    n_frames = 490.0

    amp        = fltarr(n_modes)
    n_periods  = fltarr(n_modes)
    modes      = fltarr(n_modes, n_frames)

fai_seno = 0
fai_random = 0
fai_modi = 1

    if fai_modi eq 1 then begin
        ; Metti un po' di modi

        restore,ff_file
        maxamp = readfits(maxamp_file)

        amp = fltarr(48)
        ;amp[0:43] = randomu(seed, 44)*2-1
        amp[3] = 3
        amp = amp * maxamp

        n_periods = round(randomu(seed,48)*10+1)

        for i=0, n_modes-1 do begin
           for f=0, n_frames-1 do begin
             modes[i,f] = amp[i] * sin( (f/n_frames)*n_periods[i]*2*3.1415)
           endfor
        endfor

        ; trasponi temporaneamente la matrice
        modes = transpose(modes)

        ;modes = modes * rebin(transpose(maxamp), 490, 48, /SAMPLE)
        forces = ff_matrix ## m2c ## modes 
        max_f = max(abs(forces))
        thrf = 0.45
        if max_f gt thrf then modes = modes * (thrf / max_f)

        result = m2c ## modes
        print, minmax(result)

        for i=n_frames-1, 1,-1 do begin
            result[i,*] = result[i,*] - result[i-1,*]
        endfor
        result[0,*] =0

    endif

    if fai_random eq 1 then begin

        max_amp = 5e-8

        amp = randomu(seed, 48, 490)*2 -1
        amp = amp * max_amp

        tot = total(amp,1) / 48.0
        stop
        tot = rebin(tot,490,48,/SAMPLE)
        amp = amp-tot
        result = amp

    endif

    result = transpose(result)
    hdr = CREATE_STRUCT("dummy", "dummy")
    writefits, outfile, float(result), hdr
end
