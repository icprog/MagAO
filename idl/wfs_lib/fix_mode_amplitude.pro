
pro fix_mode_amplitude, PATH_PREFIX = PATH_PREFIX, DATA_FILE = DATA_FILE, INTMAT_FILE = INTMAT_FILE, AMP_FILE = AMP_FILE, OUTPUT_FILE = OUTPUT_FILE, WANTED_AMP = WANTED_AMP, ACCEPTED_ERROR = ACCEPTED_ERROR

    if not keyword_set(PATH_PREFIX) then PATH_PREFIX = ""

    if not KEYWORD_SET(WANTED_AMP) then $
        wanted_amp = 0.5             ; Ampiezza delle slopes voluta (picco-valle max-min)

    if not KEYWORD_SET(ACCEPTED_ERROR) then $
        accepted_error = 0.1         ; Errore permesso sulle ampiezze (in percentuale +/- da 0 a 1)


    restore, PATH_PREFIX + path_sep() + DATA_FILE, /V
    intmat = readfits( PATH_PREFIX + path_sep() + INTMAT_FILE)



    p2v = maxs-mins
    factor = wanted_amp/p2v

    dummy = where(factor gt (1.0+accepted_error), count1)
    dummy = where(factor lt (1.0-accepted_error), count2)

    if count1+count2 eq 0 then begin
        print,' Amplitude is OK'
    endif else begin
        cmd = readfits(PATH_PREFIX + path_sep() + AMP_FILE)
        cmd *= factor
        cmd[ where( finite(cmd) eq 0)] = 0
        if keyword_set(OUTPUT_FILE) then writefits, PATH_PREFIX + path_sep() + OUTPUT_FILE, cmd
    endelse


end
