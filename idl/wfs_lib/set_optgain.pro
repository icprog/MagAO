
; Sets the AO loop gain
;
; gainVector: fltarr(672)  modal gain

function set_optgain, gainVector

    filename = '/home/adopt/work/AO/optGain/tmp_gain.fits'
    writefits, filename, gainVector

    res = set_ao_gain(filename)

end
