Function getconftn, struct

    @adsec_common
    
    path  = adsec_path.conf
    spawn , 'find ' + path +!AO_CONST.SHELL+'/'+' -name "*.txt"', txtfiles0
    spawn , 'find ' + path+'processConf/fastdiagn/' +' -name "*.fits"', fitsfiles0
    path1 = adsec_path.commons+!AO_CONST.SHELL+'/'
    spawn , 'find ' + path1 +' -name "*.txt"', txtfiles1
    path2 = adsec_path.data
    spawn , 'find ' + path2 +' -name "ff*.fits"', fitsfiles2
    spawn , 'find ' + path2 +' -name "pm*.fits"', fitsfiles3

    txtfiles = [txtfiles0, txtfiles1]
    fitsfiles = [fitsfiles0, fitsfiles2, fitsfiles3]
    idnv = where(fitsfiles eq "", compl=idv)
    if idnv[0] gt 0 then fitsfiles=fitsfiles[idv]
    

    for i=0, n_elements(txtfiles)-1 do begin
        line=""
        openr, unit, txtfiles[i], /GET
        READF, unit, line
        free_lun, unit
        currtn = stregex(line, '[0-9]+_[0-9]+',/EX)
        base = file_basename(txtfiles[i], '.txt')

        if i eq 0 then struct = create_struct(base, currtn[0])         $
               else struct = create_struct(base, currtn[0], struct)
    endfor
    for i=0, n_elements(fitsfiles)-1 do begin
        base = file_basename(fitsfiles[i], '.fits')
        dummy = readfits(fitsfiles[i], h, /SILENT)
        id = where(transpose(strmid(h, 0, 8)) eq 'TRACKNUM')
        if id[0] eq -1 then begin
            print, "WARNING: not found tracking number in "+fitsfiles[i]
            currtn = 'UNKNOW'
        endif else begin
            currtn = stregex(h[id], '[0-9]+_[0-9]+',/EX)
        endelse
        struct = create_struct(base, currtn[0], struct)
    endfor
        
    return, adsec_error.ok

End
        


