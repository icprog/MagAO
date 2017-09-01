;+
;;;; draft, not finished
;-

function check_vsat, seq, v_seq_peak

os = 82.5;45.0 ; overshoot V/N of voltage driver
sat = 9.0 ; saturation level of voltage driver
g   = 5.06 ; DC gain of voltage driver

s = size(seq, /DIM)
n = s[0]
if n_elements(s) eq 1 then $
  step = shift(seq,1) - seq $
else $
  step = shift(seq,1,0) - seq

vbias = g*seq
vamp = os*step
v_seq_peak = vbias+vamp
idx = where(vamp ne 0.0, count)
ratio = min((sat-vbias[idx,*])/abs(vamp[idx,*]), DIM=1)

return, ratio


end
