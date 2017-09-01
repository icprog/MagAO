

pro m2c, mode=mode, inputfile=inputfile, listmodes=listmodes, nonorm=nonorm

if not keyword_set(inputfile) then inputfile ='/towerdata/adsec_calib/M2C/KL_UPDATED/m2c.fits'

m2c_mag = readfits(inputfile)
m2c_lbt = readfits('/home/aosup/m2c_KL_v7.fits')

window,0
window,1
window,2, xsize=255, ysize=255
window,3, xsize=255, ysize=255
window,4
window,5
window,6
window,7

if n_elements(mode) eq 0 then mode=0
for mymode=mode,599 do begin

   m1 = m2c_mag[mymode,*]
   m2 = m2c_lbt[mymode,*]

   wset,2
   display, m1, pos=pos1, xx=xx, yy=yy
   wset,3
   display, m2, pos=pos2

   wset,1
   plot,pos2[128,*], psym=-4, title='LBT mode '+strtrim(mymode,2), yrange=minmax(pos2)
   oplot,pos2[125,*], psym=-4

   ; Fitting
   x =xx-127.5
   y =yy-127.5
   d = sqrt(x*x+y*y)
   slaved = where(d lt 44, complement=good)
   dummy = where(m1[good] eq 0, complement=active)
   doublegood = good[active]
   f = fltarr(255,255)
   f[ xx[doublegood], yy[doublegood]] = m1[doublegood]+5
   f[ xx[slaved], yy[slaved]] = -5

   wset,4
   tvscl,rebin(f,510,510)   

   wset,5
   res = min_curve_surf( m1[doublegood], xx[doublegood], yy[doublegood], /tps, nx=255, ny=255)
   tvscl,res

   wset,0
   !p.multi=[0,2,1]
   plot,pos1[128,*], psym=-4, title='Magellan mode '+strtrim(mymode,2), yrange=minmax([pos1, res])
   oplot, findgen(172-85+1)+85, pos1[128,85:172], psym=-4, color=255L
   oplot,pos1[125,*], psym=-4
   oplot, findgen(172-85+1)+85, pos1[125,85:172], psym=-4, color=255L
   oplot,res[128,*]

   plot,pos1[*,128], psym=-4, title='Magellan mode '+strtrim(mymode,2), yrange=minmax([pos1, res])
   oplot, findgen(172-85+1)+85, pos1[85:172,128], psym=-4, color=255L
   oplot,pos1[*,125], psym=-4
   oplot, findgen(172-85+1)+85, pos1[85:172,125], psym=-4, color=255L
   oplot,res[*,125]
   !p.multi=0



   wset,6
   plot, d, m1, psym=4, title='Magellan mode '+strtrim(mymode,2), yrange=minmax([pos1, res])
   x = rebin(findgen(255),255,255) - 127.5
   y = transpose(x)
   d2 = sqrt(x*x+y*y)
   plot, d2, res

   newslave = res[xx[slaved], yy[slaved]]

   bb = fltarr(672)
   bb[ where(xx eq 126)] +=1
   bb[ where(yy eq 126)] +=1
   fake = where(bb eq 2)

   m_new = m1
   for i=0,n_elements(slaved)-1 do begin
       dummy = where(fake eq slaved[i], count)
       if count gt 0 then continue

       m_new[slaved[i]] =  res[xx[slaved[i]], yy[slaved[i]]]
   endfor

   wset,7
   display, m_new, pos=pos1, xx=xx, yy=yy
   !p.multi=[0,2,1]
   plot,pos1[128,*], psym=-4, title='Magellan mode '+strtrim(mymode,2), yrange=minmax([pos1, res])
   oplot, findgen(172-85+1)+85, pos1[128,85:172], psym=-4, color=255L
   oplot,pos1[125,*], psym=-4
   oplot, findgen(172-85+1)+85, pos1[125,85:172], psym=-4, color=255L
   oplot,res[128,*]

   plot,pos1[*,128], psym=-4, title='Magellan mode '+strtrim(mymode,2), yrange=minmax([pos1, res])
   oplot, findgen(172-85+1)+85, pos1[85:172,128], psym=-4, color=255L
   oplot,pos1[*,125], psym=-4
   oplot, findgen(172-85+1)+85, pos1[85:172,125], psym=-4, color=255L
   oplot,res[*,128]
   !p.multi=0

   pippo=''
   read, pippo, prompt="Mode "+strtrim(mymode,2)+'.Enter "w" to write the mode vector'
   if pippo eq 'w' then begin
      filename = '/tmp/m'+strtrim(mymode,2)+'.fits'
	  if not keyword_set(nonorm) then begin
		print,'Mode renormalization'
      	target_rms = 0.75
	  	m_new *= target_rms / stddev(m_new) 
      endif else begin
        print, 'Mode not normalized'
      endelse
      writefits, filename, m_new
	  print,'Written ',filename
   endif

   stop
  
endfor


end
