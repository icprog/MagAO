
; Makes a writefits-compatible fits array
;
; Input:
; 
; data:   (array) data that will be written in the fits file
; hdr:    (strarr) string array (name, value pairs) with desired header data
; FILETYPE: (string) optional filetype tag for the header
; APPEND_HDR: (strarr) string array with additional header (already in readfits format)
;
; Output:
;
; header  (strarr) header object for writefits
;
; Example:
;
; hdr = ['name1','value1','name2','value2']
; writefits, 'filename.fits', data, make_fits_header(data, hdr, FILETYPE='INTMAT')

function make_fits_header, data, HDR = HDR, FILETYPE = FILETYPE, APPEND_HDR = APPEND_HDR

mkhdr, header, data, /EXTEND
if keyword_set(FILETYPE) then fxaddpar, header, 'FILETYPE', filetype
if keyword_set(HDR) then begin
    for i=0,n_elements(hdr)/2-1 do begin
        fxaddpar, header, hdr[i*2], hdr[i*2+1]
    endfor
endif

if keyword_set(APPEND_HDR) then begin
    exclude = ['SIMPLE', 'BITPIX', 'NAXIS', 'EXTEND', 'END']
    for i=0, n_elements(APPEND_HDR)-1 do begin
        keep=1
        for j=0,n_elements(exclude)-1 do begin
            if strmid(APPEND_HDR[i],0, strlen(exclude[j])) eq exclude[j] then keep=0
        endfor
        if keep eq 1 then header = [header, APPEND_HDR[i]]
    endfor
endif

; Move END keyword to the last line

newhdr = strarr(1)
newhdr[0] = header[0]
s = ' '
for i=0,78 do s += ' '
for i=1,n_elements(header)-1 do begin
    if (header[i] ne s) and (strmid(header[i],0,8) ne 'END     ') then begin
        newhdr = [newhdr, header[i]]
    endif
endfor

endline = 'END'
for i=0,74 do endline += ' '
newhdr = [newhdr, endline]

return, newhdr
end

