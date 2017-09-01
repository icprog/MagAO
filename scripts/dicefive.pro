
pro dicefive

;date = '20100622'
;tracknums = ['111807', '112009', '112107', '112137', '112209', '112237', '112311', '112342', '112411', '112437', '112503', '112541', '112652', '112724', '112749', '112826', '112904', '112944', '113051', '113120', '113153', '113228', '113335']


;date = '20101026'
;a = file_search('/towerdata/adsec_data/'+date+'/Data_'+date+'_*')
;nn = long(strmid(a,45,6))
;tracknums = strtrim(nn[ where((nn gt 114141) and (nn lt 121549))],2)

;=======
;date = '20101026'
;a = file_search('/towerdata/adsec_data/'+date+'/Data_'+date+'_*')
;nn = long(strmid(a,45,6))
;tracknums = strtrim(nn[ where((nn gt 114141) and (nn lt 121549)  and (nn ne 115004))],2); and (nn ne 120911) and (nn ne 120123))],2)

;>>>>>>> .r946

;115046 120258 120346 

date = '20101123'
a = file_search('/towerdata/adsec_data/'+date+'/Data_'+date+'_*')
nn = long(strmid(a,45,6))
tracknums = strtrim(nn[ where((nn ge 115046) and (nn le 120258))],2)



n = n_elements(tracknums)

center = fltarr(n,2)
jitter = fltarr(n,2)

for i=0,n-1 do begin

    tracknum = date+'_'+tracknums[i]
    a = getaoelab(tracknum)
    cc = (a->irtc())->centroid()
    dev_x = stddev(cc[*,0])
    dev_y = stddev(cc[*,1])
    jitter[i,*] = [dev_x, dev_y]
    center[i,*] = ((a->irtc())->gaussfit())->center() * (a->irtc())->pixelscale()
endfor

window,0,retain=2, xsize=900, ysize=600
plot,center[*,0], center[*,1], psym=4, xtitle='X position (arcsec)', ytitle='Y position (arcsec)', charsize=2, charthick=2
for i=0,n-1 do xyouts,center[i,0]+0.02, center[i,1]+0.02, strtrim(i,2),color=255L,charsize=1.5

print,''
print,''
;;;; Clusterizza i punti

d_cluster = 0.5  ; cluster size in arcsec
clusters=intarr(n,n)-1
for i=0,n-1 do begin
    for j=i+1,n-1 do begin
        w = where(clusters eq j, count)
        if count gt 0 then continue

        c1 = center[i,*]
        c2 = center[j,*]
        d = sqrt((c1[0]-c2[0])^2 + (c1[1]-c2[1])^2)
        if d le d_cluster then begin
            w= where(clusters eq i, count)
            if count gt 0 then begin
                c = w[0] mod n
                w1 = where(clusters[c,*] ne -1)
                clusters[c, max(w1)+1] = j
            endif else begin
                w2 = where(clusters[*,0] eq -1)
                clusters[min(w2),0] = i
                clusters[min(w2),1] = j
            endelse
        endif
    endfor
endfor


cls = where(clusters[*,0] ne -1)

elong = strarr(n_elements(cls))
dpoint = fltarr(n_elements(cls))
doffset = fltarr(n_elements(cls))
for c=0,n_elements(cls)-1 do begin
    cl = clusters[cls[c],*]
    cl = cl[ where(cl ne -1)]
    mmx = minmax(center[cl,0])
    mmy = minmax(center[cl,1])
    rmsx = stddev(center[cl,0])
    rmsy = stddev(center[cl,1])
    p2vx = mmx[1]-mmx[0]
    p2vy = mmy[1]-mmy[0]
    if p2vx gt p2vy then begin
        elong[c]='X'
        cc = center[cl,0]
        rms = rmsx
    endif else begin
        elong[c]='Y'
        cc = center[cl,1]
        rms = rmsy
    endelse
    mm = minmax(cc)
    p2v = mm[1]-mm[0]
    dpoint[c] = p2v/n_elements(cl)
    doffset[c] = p2v/n_elements(cl)/4 ; there are 4 offset for each up/down repetition
    print, 'Cluster ',c,' Elong:',elong[c],' npoints',n_elements(cl),' p2v:',p2v,' rms:',rms,' disp/point:',dpoint[c],' disp/offset',doffset[c]
    print,'Coords: ',mean(center[cl,0]),mean(center[cl,1])
endfor
        




end
