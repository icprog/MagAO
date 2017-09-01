
; modal_noise
;
; Generates a digital noise frame sequence. Each mode is modulated in a sine wave
; with the given amplitude and frequency. Frequency is specified as the number
; of periods that will fit into a frame sequence (490 frames).
;
; E.g. specifying 10 periods when the loop is going at 250 Hz will result in a
; 490/250*10 = 19.6 Hz modulation.
;
; Modes amplitude may be cut if the mode combination results in too high forces
; on the mirror.
;
; Digital noise is saved as a .fits file ready for downloading into the slope computer.
;
; Input parameters:
;
; outfile     :   (string) filename to use for the resulting FITS file.
; M2C_FILE    :   (string) filename of the m2c matrix FITS file
; FF_FILE     :   (string) filename of the ff matrix SAV file
; MAXAMP_FILE :   (string) filename of the max_modal_amplitude FITS file
; NOISE       :   (keyword) if set, adds a random noise to the output
; 
;
; AMPVECTOR    :   fltarr(48) with the amplitude of each mode, from 0 to 1. 
;                 Amplitude will be normalized by the standard maxamp file.
; PERIODSVECTOR:   fltarr(48) with the number of periods of each mode in the
;                 full frame sequence (490 frames).
; - OR - 
;
; AMPFILE     :   fits file with 48 amplitude values
; PERIODSFILE :   fits file with 48 period values
;


pro modal_noise, outfile, AMPVECTOR = AMPVECTOR, PERIODSVECTOR = PERIODSVECTOR, AMPFILE = AMPFILE, PERIODSFILE = PERIODSFILE, M2C_FILE = M2C_FILE, FF_FILE = FF_FILE, MAXAMP_FILE = MAXAMP_FILE, NOISE = NOISE, NUM=NUM

    if not keyword_set(M2C_FILE) then begin
        print,'Missing m2c file'
        exit
    endif
    if not keyword_set(FF_FILE) then begin
        print,'Missing ff file'
        exit
    endif
    if not keyword_set(MAXAMP_FILE) then begin
        print,'Missing maxamp file'
        exit
    endif

    if keyword_set(AMPVECTOR) then amp = AMPVECTOR
    if keyword_set(AMPFILE)   then amp = readfits(AMPFILE)

    if keyword_set(PERIODSVECTOR) then periods = PERIODSVECTOR
    if keyword_set(PERIODSFILE)   then periods = readfits(PERIODSFILE)

    if not keyword_set(amp) then begin
        print,'No amplitude specified'
        exit
    endif
    if not keyword_set(periods) then begin
        print,'No periods specified'
        exit
    endif

    m2c = readfits(m2c_file)

    n_modes = n_elements(m2c[*,0])
    n_act   = n_elements(m2c[0,*])
    n_frames = 490.0

    modes      = fltarr(n_modes, n_frames)

    ; Restore ff matrix and fix dimensions
    restore,ff_file
    temp_ff_matrix = fltarr( n_elements(ff_matrix[*,0]), n_act)
    new_ff_matrix = fltarr( n_act, n_act)

    for n=0,n_elements(act_w_ff)-1 do $ 
        temp_ff_matrix[*,act_w_ff[n]] = ff_matrix[*,n]

    for n=0,n_elements(act_w_ff)-1 do $
        new_ff_matrix[act_w_ff[n],*] = temp_ff_matrix[n,*]
    
    ff_matrix = new_ff_matrix    
   
    maxamp = readfits(maxamp_file)

    ; Normalize amplitudes
    amp = amp * maxamp / 5.0

   seed = NUM
   atm = modes
   for i=0, n_modes-1 do begin
      for f=0, n_frames-1 do begin
        modes[i,f] = amp[i] * sin( (float(f)/n_frames)*periods[i]*2*!pi)
      endfor

     if keyword_set(NOISE) then begin
         atm[i,*] = gen_noise(seed, n_frames)*maxamp[i] *2.0
         modes[i,*] = modes[i,*] + atm[i,*]
      endif
    endfor


;    for i=0, n_modes-1 do begin
;        modes[i,*] = 1/3.0 * randomn( seed, n_frames, /NORMAL)
;    endfor


    ; trasponi temporaneamente la matrice
    modes = transpose(modes)
    atm   = transpose(atm)

    ; Calculate the needed forces and cut the modal amplitude if needed
    help, ff_matrix, m2c, modes
    forces = ff_matrix ## m2c ## modes 
    max_f = max(abs(forces))
    thrf = 0.30   ; max force (newtons?)
    if max_f gt thrf then begin
        modes = modes * (thrf / max_f)
        atm   = atm   * (thrf / max_f)
        print, 'Modes amplitude cut by '+strtrim( 100-(thrf/max_f)*100 ,2)+'%'
    endif

    result = modes
    print, 'Min/max mode: ',minmax(result)

    ; Get differential commands

    temp = result
    for i=0,n_frames-2 do result[i,*] = temp[i+1,*] - temp[i,*]
    result[n_frames-1, *] = temp[0,*] - temp[n_frames-1, *] 

    outfile = outfile+strtrim(NUM,2)+".fits"

    ; Transpose and save
    result = transpose(result)
    hdr = CREATE_STRUCT("TYPE", "DIGITALNOISE")
    writefits, outfile, float(result), hdr

    modesfile = outfile+".modes"+strtrim(NUM,2)+".fits"
    atmfile = outfile+".atm"+strtrim(NUM,2)+".fits"
    writefits, modesfile, float(temp), hdr
    writefits, atmfile, float(atm), hdr

    print,'Written '+outfile
end

