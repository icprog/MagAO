function gen_noise, seed, n_points, amp=amp

if keyword_set(amp) then begin
    amp1 = amp/(1.+dist(n_points,1)^2)
endif else begin
    amp1 = 1./(1+dist(n_points,1)^2)
endelse
arg = (randomu(seed, n_points)-0.5)*2*!pi

noise = float(fft(amp1 * complex(cos(arg), sin(arg)),-1))*n_points/2

return, noise
end