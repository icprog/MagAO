
pro muovileggirot_plot, prefix, num_points

x = fltarr(num_points,4)
y = fltarr(num_points,4)
order = [3,1,2,0]

for i=0,num_points-1 do begin

    filename = prefix+'/pupils_fr'+strtrim(i,2)+'.txt'
    a = read_ascii(filename)
    x[i,*] = a.field1[0,*]
    y[i,*] = a.field1[1,*]

endfor

window,0,retain=2, xsize=800, ysize=600

; rotate 90" to be the same as the ccd viewer
tempx = x
x = y
y = 80-tempx-1

!p.multi=[0,2,2]

for p=0,3 do begin

	pupil = order[p]

	mmx = minmax(x[*,pupil])
	mmy = minmax(y[*,pupil])
	p2vx = mmx[1]-mmx[0]
	p2vy = mmy[1]-mmy[0]
	dx = p2vx/10
	dy = p2vy/10

	rx = [ mmx[0]-dx , mmx[0] + p2vx + dx]
	ry = [ mmy[0]-dy , mmy[0] + p2vy + dy]

	plot,x[*,pupil],y[*,pupil],title="Pupil movement", xtitle="pixels", ytitle="pixels", psym=-4,xrange=rx, yrange=ry, /xstyle, /ystyle, /iso
	for i=0,n_elements(x[*,pupil])-1 do xyouts, x[i,pupil]+0.1, y[i,pupil], string(format='(%"%5.1f")', i*22.5)

endfor

!p.multi=[0]

end



