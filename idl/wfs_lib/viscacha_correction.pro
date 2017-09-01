
function viscacha_correction, intmat

  soglia_tip = 0.2e5
  soglia_tilt = -0.2e5

  tip  = intmat[0,*]
  tilt = intmat[1,*]

  sx = tip[0:*:2]
  sy = tilt[1:*:2]

  window,10
  plot, sx, title='SX'
  oplot, [0,1600], [soglia_tip, soglia_tip], col=255L

  window,11
  plot, sy, title='SY'
  oplot, [0,1600], [soglia_tilt, soglia_tilt], col=255L

  idx_tip = where(sx gt soglia_tip)
  idx_tilt= where(sy lt soglia_tilt)

  ;; Deinterlace
  idx_tip *=2 
  idx_tilt  = idx_tilt *2 +1

  intmat[*, idx_tip] *= -1
  intmat[*, idx_tilt] *= -1

  return, intmat
end
