
function sinusmode_analyse, acq_tracknum, BIN=BIN, MODES = MODES

if n_elements(BIN) eq 0 then BIN = 1.0 ; bin in hz

ee = getaoelab(acq_tracknum)

hdr = (ee->disturb())->header()
mode = fix(get_fits_keyword(hdr, 'MODE'))
freq = float(get_fits_keyword(hdr, 'FREQ'))
eff_freq = float(get_fits_keyword(hdr, 'FREQEFF'))

from = eff_freq-BIN/2.0
to   = eff_freq+BIN/2.0


if keyword_set(MODES) then begin
   power = (ee->modes())->power(mode, from_freq=from, to_freq=to)
endif else begin
   power = (ee->residual_modes())->power(mode, from_freq=from, to_freq=to)
endelse

return, power
end
