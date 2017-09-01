; sinusmode_generate
;
; Generates a disturbance file for the automatic gain procedure.
;
; Paramaters:
;
; mode     : mode to modulate (0..672)
; amp      : semi-amplitude in meters
; freq     : requested modulation frequency
; loopfreq : expected loop frequency 
; modalbase: modal basis string (e.g. 'KL_v7')
;
; Keyword parameters:
; 
; EVEN_PERIODS : change frequency to have an integer no. of periods to allow the disturbance to be looped.
; ADD_FILE : add to the disturbance another disturbance file (in mirror command)
;
; Return value:
;
; (string) disturbance tracknum
; EFFECTIVE_FREQ (float): real frequency in case the EVEN_PERIODS keyword was specified



function sinusmode_generate, mode, amp, freq, loopfreq, modalbasis, EVEN_PERIODS = EVEN_PERIODS, EFFECTIVE_FREQ = EFFECTIVE_FREQ, ADD_FILE=ADD_FILE

if not keyword_set(ADD_FILE) then ADD_FILE=""

a = fltarr(672)
a[mode] = amp
amp_envelope = replicate(1.0, 672)

; Calculate no. of periods in disturbance
per = (4000.0/loopfreq)*freq

if (fix(per) ne per) and (keyword_set(EVEN_PERIODS)) then per = fix(per)+1
EFFECTIVE_FREQ = per/(4000.0/loopfreq)
p = fltarr(672)
p[mode] = per

; Generate modal disturbance
modes = float(modal_history( ampvector = a, periodsvector= p, ENVELOPE_PASSED = amp_envelope, /SINUS))

sep = path_sep()
prefix = '/towerdata/adsec_calib/M2C'
path = prefix + sep + modalbasis

m2c = readfits(path + sep + 'm2c.fits')
commands = float(m2c ## modes)

tracknum = get_tracknum()
modes_filename = path + sep + 'disturb' + sep + 'ModalDisturb_' + tracknum + '.fits'
cmd_filename   = path + sep + 'disturb' + sep + 'CmdDisturb_' + tracknum + '.fits'

even=0
if (per eq fix(per)) then even=1

h = [ "TYPE", "SINUSMODE", $
        "MODE", strtrim(mode,2), $
        "FREQ", strtrim(freq,2), $
        "FREQEFF", strtrim(EFFECTIVE_FREQ), $
        "LOOPFREQ", strtrim(loopfreq,2), $
        "AMP", strtrim(amp,2), $
        "M2C", modalbasis, $
        "ADD_FILE", ADD_FILE, $
        "EVEN", strtrim(even,2) ]

writefits, modes_filename, modes, make_fits_header( modes, HDR= h, FILETYPE='MODAL_HISTORY')

if ADD_FILE ne "" then begin
    atmo = readfits(ADD_FILE)
    commands += atmo
endif
writefits, cmd_filename, commands, make_fits_header( modes, HDR= h, FILETYPE='COMMAND_HISTORY')

return, tracknum

end
