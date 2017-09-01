;
; Write a sinusoidal disturbance for the transfer function characterization.
; Each mode is modulated at each frequency separately into a standard 4000 frames buffer.
; Each buffer starts with 100 empty frames, then a marker with an oscillating tip-tilt, then up to 3800 frames
; of sinusoidal modulation. Length is then padded to 4000 frames.
;
; For each mode/freq combination two files are generated:
;
; ModalDisturb_YYYYMMDD_MMMFFF
; CmdDisturb_YYYYMMDD_MMMFFF
;
; where:  YYYYMMDD = timestamp
; MMM = mode number
; FFF = frequency in Hz
;
; If the M2C matrix is not given, the CmdDisturb file will not be written. M2C is a filename.
;
; Parameters:
;
; MODES = array of modes to modulate
; AMPS  = array of amplitudes (one for each mode)
; FREQS = array of frequencies (will be replicated for each mode)
; DISTLEN = length of disturbance in frames <= 3800
; LOOPFREQ = assumed loop speed (default = 1000Hz)
; M2C   = m2c matrix to generate the command disturbance file
; CONCAT = concatenate all disturbance into one long file instead of separate fils.

pro disturb_sin, MODES=modes, AMPS=amps, FREQS=freqs, DISTLEN=distlen, LOOPFREQ=loopfreq, M2C=M2C, CONCAT=CONCAT

nmodes = 672
marker_pos = 100
marker_amp = 1e-6
marker_mode = 0
sinus_pos = 200
totlen = 4000

if not keyword_set(LOOPFREQ) then LOOPFREQ = 1000.0
if not keyword_set(DISTLEN) then DISTLEN = 3800
if keyword_set(M2C) then M2C = readfits(M2C)

loopfreq = float(loopfreq)
first =1

for m=0,n_elements(modes)-1 do begin
	mode = modes[m]
	amp  = amps[m]
	for f=0,n_elements(freqs)-1 do begin
		freq = freqs[f]

		modaldist = fltarr(sinus_pos+distlen,nmodes)

		; Marker
		modaldist[marker_pos+0:marker_pos+2, marker_mode] = marker_amp
		modaldist[marker_pos+3:marker_pos+5, marker_mode] = -marker_amp
		modaldist[marker_pos+6:marker_pos+8, marker_mode] = marker_amp
		modaldist[marker_pos+9:marker_pos+11, marker_mode] = -marker_amp


		for i=0,distlen-1 do begin

			period = loopfreq/freq
			modaldist[ sinus_pos+i, mode] = sin(i/period*3.1415*2)*amp

		endfor

		len = n_elements(modaldist[*,0])
		if len lt totlen then begin
			mm = fltarr(totlen-len, nmodes)
			modaldist = [modaldist, mm]
		endif

		caldat, systime(/julian, /UTC),mm,dd,yyyy
		if not keyword_set(CONCAT) then begin
			timestamp = string(yyyy,format='(I4.4)')+string(mm,format='(I2.2)')+string(dd,format='(I2.2)')
			timestamp += '_'+string(mode,format='(I3.3)')+string(fix(freq),format='(I3.3)')+'.fits'
			filename = 'ModalDisturb_' + timestamp
			print,filename
			writefits, filename, modaldist

			if keyword_set(M2C) then begin
				filename = 'CmdDisturb_'+ timestamp
				print, filename
				writefits, filename, m2c ## modaldist
			endif

		endif else begin
			if first eq 1 then begin
				totaldisturb = modaldist
				first =0
			endif $
			else totaldisturb = [totaldisturb, modaldist]
		endelse
	endfor
endfor

if keyword_set(CONCAT) then  begin
	timestamp = string(yyyy,format='(I4.4)')+string(mm,format='(I2.2)')+string(dd,format='(I2.2)')
	timestamp += '_'+string(modes[0],format='(I3.3)')+string(fix(freqs[0]),format='(I3.3)')+'.fits'
	filename = 'ModalDisturb_' + timestamp
	print,filename
	writefits, filename, totaldisturb

	if keyword_set(M2C) then begin
		filename = 'CmdDisturb_'+ timestamp
		print, filename
		writefits, filename, m2c ## totaldisturb 
	endif
endif


end
