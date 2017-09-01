

pro check_magnification

;track = '20090917_144441'
;track = '20090917_150157'
;track = '20090917_151218'
track = '20090917_152251'

pup = '20090916-144036'

slopepath = '/towerdata/adsec_data/20090917/'
puppath = getenv('ADOPT_ROOT')+'/calib/'+format_side(getenv('ADOPT_SIDE'))+'/WFS/ccd39/LUTs/bin1/'

slopefile = slopepath +'Data_'+track+'/Slopes_'+track+'.fits'
pupfile = puppath + pup + '/slopex'

s = readfits(slopefile)
table = read_ascii(pupfile)
t = table.field1
idx = where( t ne -1)

n_frames = n_elements(s[0,*])

frame = fltarr(80,80)
frame_tot = fltarr(80,80,n_frames)
slope = fltarr(1600)

for i=0,n_frames-1 do begin
    slope = s[*,i]
    frame[idx] = slope[t[idx]] 
    frame_tot[*,*,i] = frame
endfor

frame_avg = total(frame_tot,3)/n_frames

mag = 911/30. 


stop
end
