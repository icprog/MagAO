
; modal_noise
;
; Generates a modal frame history.
;
; Push-pull mode:
; Each mode is modulated with a square wave with the given amplitude and frequency.
;
; Sinusoidal mode:
; Each mode is modulated in a sine wave with the given amplitude and frequency.
;
; Frequency is specified as the number of periods that will fit into a frame sequence (4000 frames).
;
; E.g. specifying 10 periods when the loop is going at 250 Hz will result in a
; 250*(10/4952) = 0.5 Hz modulation
;
; Digital noise is saved as a .fits file ready for uploading to the AdSec software
;
; Input parameters:
;
; outfile     :  (string) filename to use for the resulting FITS file.
; NOISE       :  (keyword) if set, adds a random noise to the output
; PUSHPULL    : (keyword) modulate modes with a square wave for push-pull acquisition
; SINUS       : (keyword) modulate modes with a sinusoid for lock-in acquisition
; ENVELOPE    : amplitude envelope file. If not set, defaults to "amp_envelope.fits"
;
; PATH_PREFIX : (string) base path for all filenames, will be joined with a path separator. Do not set if you use absolute path names.
;
;
; SEQ_LEN     : sequence len. If not set, defaults to full sequence (4952 frames)
; 
;
; AMPVECTOR    :   fltarr(672) with the amplitude of each mode, from 0 to 1. 
;                 Amplitude will be normalized by the standard amp_envelope file.
; PERIODSVECTOR:   fltarr(672) with the number of periods of each mode in the
;                 full frame sequence (490 frames).
; - OR - 
;
; AMPFILE     :   fits file with 672 amplitude values
; PERIODSFILE :   fits file with 672 period values
;



; Example:
; modulate mode #1 with 100 push-pulls in the whole disturb history, with 0.5 amplitude:
;
; a = fltarr(672)    
; a[1]=0.5
; p = replicate(100,672)
; m = modal_history( ampvector=a, per=p, /PUSHPULL)


function modal_history, PATH_PREFIX = PATH_PREFIX, OUTFILE = OUTFILE, AMPVECTOR = AMPVECTOR, PERIODSVECTOR = PERIODSVECTOR, NOISE = NOISE, SEED=SEED, SINUS = SINUS, PUSHPULL = PUSHPULL, ENVELOPE = ENVELOPE, ENVELOPE_PASSED = ENVELOPE_PASSED, SEQ_LEN = SEQ_LEN

    if not keyword_set(PATH_PREFIX) then PATH_PREFIX=""

    if keyword_set(AMPVECTOR) then amp = AMPVECTOR
    if keyword_set(PERIODSVECTOR) then periods = PERIODSVECTOR

    if keyword_set(ENVELOPE_PASSED) then amp_envelope = ENVELOPE_PASSED $
    else begin
        if keyword_set(ENVELOPE) then amp_envelope = readfits( PATH_PREFIX + path_sep() + ENVELOPE) $
        else amp_envelope = readfits('amp_envelope.fits')
    endelse

    if not keyword_set(SEQ_LEN) then SEQ_LEN = 4000

    if not keyword_set(amp) then begin
        print,'No amplitude specified'
        return,0
    endif
    if not keyword_set(periods) then begin
        print,'No periods specified'
        return, 0
    endif

    n_modes = 672

    env2 = fltarr(n_modes)
    env2[0] = amp_envelope
    amp = amp * env2

    modes      = fltarr(n_modes, SEQ_LEN)

    atm = modes
    if keyword_set(SINUS) then begin
        for i=0, n_modes-1 do begin
            for f=0, SEQ_LEN-1 do begin
                modes[i,f] = amp[i] * sin( (float(f)/SEQ_LEN)*periods[i]*2*!pi)
            endfor
        endfor
    endif

    if keyword_set(PUSHPULL) then begin
        for i=0, n_modes-1 do begin
            period_len = SEQ_LEN/periods[i]
            for f=0, SEQ_LEN-1 do begin
                if f mod period_len lt period_len/2 then $
                    modes[i,f] = amp[i] $
                else $
                    modes[i,f] = -amp[i]
            endfor
        endfor
    endif

;    if keyword_set(NOISE) then begin
;         atm[i,*] = gen_noise(SEED, SEQ_LEN)*maxamp[i] *2.0
;         modes[i,*] = modes[i,*] + atm[i,*]
;      endif
;    endfor

    ; Matrix must be [time, modes]
    modes = transpose(modes)

    if keyword_set(OUTFILE) then begin
        hdr = CREATE_STRUCT("TYPE", "MODALHIST")
        writefits, PATH_PREFIX + path_sep() + OUTFILE, modes, hdr
        print,'Written '+OUTFILE
    endif

    return, modes

end

