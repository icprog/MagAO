;'/home/labot/work/AO/releases/trunk/conf/adsec/672b/'

Pro addtn, path

    @adsec_common
    path  = getenv('ADOPT_SOURCE')+'/conf/adsec/585m/'
    spawn , 'find ' + path +' -name "*.txt"', txtfiles0
    spawn , 'find ' + path+'processConf/fastdiagn/' +' -name "*.fits"', fitsfiles0
;    path1 = getenv('ADOPT_SOURCE')+'/conf/adsec/commons/'
;    spawn , 'find ' + path1 +' -name "*.txt"', txtfiles1



    tn0 = '20110101_010101'
    tn = ';;+& '+tn0+' &-;;';
    out = '/towerdata/adsec_calib/configuration/'
    ;txtfiles = [txtfiles0, txtfiles1]
    txtfiles = txtfiles0

    for i=0, n_elements(txtfiles)-1 do begin

        if (file_info(txtfiles[i])).size ne 0 then begin
            openr, unit, txtfiles[i], /GET
            text = ''
            readf, unit, text
            currtn = stregex(text, '[0-9]+_[0-9]+',/EX) 
            free_lun, unit

            filetext = read_binary(txtfiles[i])
        endif else begin
            filetext = bytarr(25)
        endelse
        if strlen(currtn) eq 15 then begin

            print, "TN is present in : "+file_basename(txtfiles[i])
            filetext[0:24] = byte(tn)
            print, 'currTN: '+currtn+' substituted with '+ stregex(tn, '[0-9]+_[0-9]+',/EX)
        endif else begin
            print, 'adding TN '+stregex(tn, '[0-9]+_[0-9]+',/EX)+' for: ' + file_basename(txtfiles[i])
            filetext = [byte(tn),byte(string(10B)), filetext]
        endelse

        openw, unit, txtfiles[i], /GET
        writeu, unit, filetext
        free_lun, unit
        tmp = strmid(file_basename(txtfiles[i]), 0, strlen(file_basename(txtfiles[i]))-4)
        outdir = out + file_dirname(strmid(txtfiles[i], strlen(path))) + '/' + tmp + '/' +  tn0
        file_mkdir, outdir
        file_copy, txtfiles[i], outdir, /OVER
    

    endfor
    
    
     path2  = getenv('ADOPT_SOURCE')+'/calib/adsec/585m/'

    spawn , 'find ' + path2 +' -name "ff*.fits"', fitsfiles

    for i=0, n_elements(fitsfiles)-1 do begin
        
        tmpfits = readfits(fitsfiles[i])
        mkhdr, hdr, tmpfits, /EXTEND
        sxaddpar, hdr, "TRACKNUM", '20101027_193318'
        writefits, fitsfiles[i], tmpfits, hdr

    endfor

    spawn , 'find ' + path2 +' -name "pbgainmatrix.fits"', fitsfiles
    tmpfits = readfits(fitsfiles[0])
    mkhdr, hdr, tmpfits, /EXTEND
    sxaddpar, hdr, "TRACKNUM", tn0
    writefits, fitsfiles[0], tmpfits, hdr
    
    
End

