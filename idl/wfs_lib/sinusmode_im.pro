
function sinusmode_im, acq_tracknum, BIN=BIN, MODES = MODES

if n_elements(BIN) eq 0 then BIN = 1.0 ; bin in hz

ee = getaoelab(acq_tracknum)

hdr = (ee->disturb())->header()
fname = (ee->disturb())->fname()
modal_fname = strjoin(strsplit(fname, 'CmdDisturb', /EXTRACT, /REGEX), 'ModalDisturb')
slopes = (ee->slopes())->slopes()

mode = fix(get_fits_keyword(hdr, 'MODE'))
freq = float(get_fits_keyword(hdr, 'FREQ'))
eff_freq = float(get_fits_keyword(hdr, 'FREQEFF'))
loop_freq = float(get_fits_keyword(hdr, 'LOOPFREQ'))
amp  = float(get_fits_keyword(hdr, 'AMP'))
modal_dist = (readfits(modal_fname))[*,mode]

z = modal_dist ## transpose(slopes)
w = modal_dist ## transpose( shift(slopes, (loop_freq/eff_freq)/4, 0))

c =w^2 + z^2

;;;;;;;;;;;;;;;

ss=20
rms = fltarr(ss)
for s=0,ss-1 do rms[s] = stddev( slopes ## shift(modal_dist,s))
best_s = where(rms eq max(rms))
best_s=0
modo = slopes ## shift(modal_dist, best_s)




end
