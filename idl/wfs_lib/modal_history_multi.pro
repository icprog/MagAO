;@File: modal_history_multi
;
;@Procedure: modal_history_multi
;
; Generates a modal frame history with push-pull of sinusoidal style.
;
; Push-pull mode:
; Each mode is modulated with a square wave with the given amplitude and frequency. Modes are modulated one after the other.
;
; Sinusoidal mode:
; Each mode is modulated in a sine wave with the given amplitude and frequency. Modes are modulated in groups.
;
; Frequency is specified as the number of periods that will fit into a frame sequence (4000 frames).
;
; E.g. specifying 10 periods when the loop is going at 250 Hz will result in a
; 250*(10/4952) = 0.5 Hz modulation
;
; Digital noise is saved in a pair of .fits files ready for uploading to the Adaptive secondary.
;
; Input parameters:
;
; PUSHPULL    : (keyword) modulate modes with a square wave for push-pull acquisition
; SINUS       : (keyword) modulate modes with a sinusoid for lock-in acquisition
;
; PATH_PREFIX : (string) base path for all filenames, will be joined with a path separator. Do not set if you use absolute path names.
;
; MODAL_FILENAME: (string) filename where the modal history will be saved in FITS format. If not specified, no modal history will be saved.
; CMD_FILENAME  : (string) filename where the command history will be saved in FITS format. If not specified, no command history will be saved.
;
; AMPVECTOR    :   fltarr(TOTAL_MODES) with the amplitude of each mode, from 0 to 1. Amplitude will be normalized by the standard amp_envelope file.
; PERIODSVECTOR:   fltarr(TOTAL_MODES) with the number of periods of each mode in the full frame sequence (4000 frames) or in SEQ_LEN if specified
;
; TOTAL_MODES   : (int) total no. of modes to use. If not specified, defaults to 672
; TOTAL_LEN     : (int) if specified, total length of the sequence. If the result is shorter, it will be padded with zeroes to this length
; MULTIPLE_LEN  : (int) if specified, total length of the sequence will be padded as before, to the next multiple of the specified value.
;
; M2C         : (string) filename of the m2c matrix to use (FITS) to generate the command history.
; ENVELOPE    : (string) amplitude envelope file. If not set, defaults to "amp_envelope.fits"
; SEQ_LEN     : (int) sequence len. If not set, defaults to full sequence (4000 frames).
;
; NOISE       : (keyword) if set, adds a random noise to the output
; SEED        : (int) seed to use for random number generator
; 
; HDR         : (strarr) optional header to add to the standard FITS header, ideally generated with fxaddpar.




; Multi version:  each mode is modulated separately and sequences are joined into a single temporal sequence one after the other

; Example:
; modulate the first 100 modes one after the other with a single push-pull of max. amplitude, each 40 frames long:
;
; a = fltarr(672)    
; a[0:99]=0.8
; p = replicate(1,672)
; m = modal_history_multi( amp = a, per = p, /PUSHPULL, SEQ_LEN=40)  


function modal_history_multi, PATH_PREFIX = PATH_PREFIX, MODAL_FILENAME = MODAL_FILENAME, CMD_FILENAME = CMD_FILENAME, AMPVECTOR = AMPVECTOR, PERIODSVECTOR = PERIODSVECTOR, NOISE = NOISE, SEED=SEED, SINUS = SINUS, PUSHPULL = PUSHPULL, ENVELOPE = ENVELOPE, SEQ_LEN = SEQ_LEN, M2C = M2C, HDR = HDR, TOTAL_LEN = TOTAL_LEN, MULTIPLE_LEN=MULTIPLE_LEN, TOTAL_MODES = TOTAL_MODES, ADDFILE = ADDFILE

    if not keyword_set(TOTAL_MODES) then TOTAL_MODES=672
    if not keyword_set(PATH_PREFIX) then PATH_PREFIX=""

    goodmodes = where( AMPVECTOR ne 0)

    ; Preallocate array
    totlen = SEQ_LEN * n_elements(goodmodes)
    modes = fltarr(totlen, TOTAL_MODES)


    ; Read envelope file only once
    if keyword_set(ENVELOPE) then ENVELOPE_PASSED = readfits(PATH_PREFIX + path_sep() + ENVELOPE)

    for n=0,n_elements(goodmodes)-1 do begin

        a = fltarr(TOTAL_MODES)
        a[goodmodes[n]] = AMPVECTOR[goodmodes[n]]
        modes[n*SEQ_LEN : (n+1)*SEQ_LEN-1, *]  = modal_history( AMPVECTOR = a, PERIODSVECTOR = PERIODSVECTOR, NOISE = NOISE, SEED=SEED, SINUS=SINUS, PUSHPULL=PUSHPULL, ENVELOPE_PASSED=ENVELOPE_PASSED, SEQ_LEN = SEQ_LEN)

    endfor

    zeros = fltarr(100,TOTAL_MODES)
    modes = [zeros, modes]

    len = n_elements(modes[*,0])

    ; Pad to a specific length
    if keyword_set(TOTAL_LEN) then begin
        if len lt TOTAL_LEN then begin
            modes = [modes, fltarr(TOTAL_LEN-len, TOTAL_MODES)]
        endif
    endif

    ; Pad to a multiple of a specific length
    if keyword_set(MULTIPLE_LEN) then begin
        if len mod MULTIPLE_LEN ne 0 then begin
            modes = [modes, fltarr(MULTIPLE_LEN - (len mod MULTIPLE_LEN), TOTAL_MODES)]
        endif
    endif


    ; Write out modal history
    if keyword_set(MODAL_FILENAME) then begin
        data = float(modes)
        writefits, PATH_PREFIX + path_sep() + MODAL_FILENAME, data, make_fits_header( data, HDR= HDR, FILETYPE='MODAL_HISTORY')

        ; Modify modal history for noise estimation
        noise_data = data
        noise_data[*,1:*] = 0
        noise_data[ where(data gt 0)] = 1
        noise_data[ where(data lt 0)] = 0
        writefits, PATH_PREFIX + path_sep() + MODAL_FILENAME, noise_data, /APPEND

        print,'Written '+MODAL_FILENAME
    endif

    ; Write out command history
    if keyword_set(CMD_FILENAME) then begin
        m2c_mat = readfits(PATH_PREFIX + path_sep() + M2C)
        data =  float(m2c_mat) ## float(modes)

        if keyword_set(ADDFILE) then begin
            data += readfits(ADDFILE)
        endif

        writefits, PATH_PREFIX + path_sep() + CMD_FILENAME, data, make_fits_header( data, HDR= HDR, FILETYPE='COMMAND_HISTORY')
        print,'Written '+CMD_FILENAME
    endif

    return, modes
end


