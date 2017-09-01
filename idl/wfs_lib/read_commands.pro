
; Read the current modal commands from the AO loop
;
; Returns: fltarr(672)  current modal commands (integrated)

function read_commands

    ;print, 'Read modes result:',;
    buf = fltarr(52372/4)
    modes = fltarr(672)

    counter = shm_readbuf_wrap( 'MASTDIAGN00:ADSECBUF', 1, 1000, buf)
    if counter lt 0 then begin
        print, 'Readbuf error: ',counter
    endif
    for crate=0,5 do for i=0,27 do modes[i*4 + crate*4*28:i*4+3 + crate*4*28] = buf[1673 +i*68 + crate*(68*28+8): 1676 +i*68 + crate*(68*28+8)]
    return, modes

end
