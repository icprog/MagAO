
pro mouseEvent, event
common vars, factor, f, w, dim, diff, pup_tracknum, pupdir, bin

if event.press ne 1 then return

x = event.x / factor
y = event.y / factor

print, x,y
f[x,y] = 1-f[x,y]
pupDisplay, w, f, dim*factor
wait,0.1
end

pro pupDisplay, w, f, totdim
wset, w
tvscl,rebin(f, totdim, totdim, /sample)
end


pro refine_pup, from_tracknum, binning
common vars, factor, f, w, dim, diff, pup_tracknum, pupdir, bin

display_size = 600
pup_tracknum = from_tracknum
bin = binning

pupdir = GETENV('ADOPT_ROOT')+'/calib/wfs/current/ccd39/LUTs/bin'+strtrim(binning,1)+path_sep()
pup1 =readfits( pupdir+from_tracknum+path_sep()+'/pup1.fits')
pup2 =readfits( pupdir+pup_tracknum+path_sep()+'/pup2.fits')
pup3 =readfits( pupdir+pup_tracknum+path_sep()+'/pup3.fits')
pup4 =readfits( pupdir+pup_tracknum+path_sep()+'/pup4.fits')


diff = fltarr(5)
diff[2] = pup2[0]-pup1[0]
diff[3] = pup3[0]-pup1[0]
diff[4] = pup4[0]-pup1[0]

dim = 80/binning
factor = display_size/dim
f= fltarr(dim, dim)
f[pup1] = 1

window,10,retain=2,xsize=display_size, ysize=display_size
f2 = f
f2[pup2]=2
f2[pup3]=3
f2[pup4]=4
pupDisplay, 10, f2, dim*factor

base = widget_base(/row)
disp = widget_draw(base, xsize=display_size, ysize=display_size, retain=2, event_pro='mouseEvent')
widget_control, disp, draw_button_events=1

widget_control, base, /realize

widget_control, disp, GET_VALUE=w
pupDisplay, w, f, dim*factor


xmanager,'pup', base, /no_block

end


pro accept
common vars, factor, f, w, dim, diff, pup_tracknum, pupdir, bin

pup1 = where(f eq 1)
pup2 = pup1+diff[2]
pup3 = pup1+diff[3]
pup4 = pup1+diff[4]

f[pup2]=2
f[pup3]=3
f[pup4]=4

pupDisplay, w, f, dim*factor

pupdatafile = pupdir+pup_tracknum+path_sep()+'pupdata.txt'
outpupdatafile = pupdir+'pupdata.txt'
file_copy, pupdatafile, outpupdatafile, /overwrite
print, outpupdatafile
print, pupdir+'pup1.fits'

writefits, pupdir+'pup1.fits', pup1
writefits, pupdir+'pup2.fits', pup2
writefits, pupdir+'pup3.fits', pup3
writefits, pupdir+'pup4.fits', pup4

openw, unit, 'make.py', /GET_LUN
printf, unit, 'from AdOpt import makePupils'
printf, unit, 'makePupils.makePupils_fromMasks('+strtrim(dim*bin,2)+', '+strtrim(bin,2)+')'
close, unit
free_lun, unit

spawn,'python make.py'
end
