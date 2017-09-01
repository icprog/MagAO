
pro analyze_loop_p45, filename, rms

loop = readfits(filename)
xtable = read_ascii("slopex_80")
ytable = read_ascii("slopey_80")

xtable = xtable.field1
ytable = ytable.field1

xtable[ where(xtable eq -1)] = 0
ytable[ where(ytable eq -1)] = 0

n_frames = n_elements(loop[0,*])
rms = fltarr(n_frames)

n_pixels = 80*80
last_valid_slope = 526*2

for i=0,n_frames-1 do begin
	framex = fltarr(80,80)
	framey = fltarr(80,80)

	slope_frame = reform(loop[*,i])

;	for j=0,n_pixels-1 do begin
;       framex[j] = slope_frame[xtable[j]]
;    framey[j] = slope_frame[ytable[j]]
;   endfor
	
;tvscl, rebin( framex, 160, 160, /SAMPLE),1
;   tvscl, rebin( framey, 160, 160, /SAMPLE),2

    rms[i] = stddev( slope_frame[0:last_valid_slope-1])
endfor

plot,rms

end	