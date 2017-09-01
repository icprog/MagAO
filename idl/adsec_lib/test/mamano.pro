;e' da testare la moltiplicazione tra matrici, ma la matrice vettore funziona
;REVERSE da testare.

function mamano, m_in, n_in, REVERSE=reverse

m=reform(m_in)
n=reform(n_in)
sm = size(m, /dim)
sn = size(n, /dim)

if sm[0] ne sn[n_elements(sn)-1] then begin
    print, 'Uncoerent dimensions.'
    return , -1
endif else begin
    if not keyword_set(REVERSE) then begin
        if n_elements(sn) gt 1 then begin
        ;;;;;;;;;;;;;;;;da testare
            array = fltarr(sm[1],sn[0])
            for k=0, sn[0]-1 do begin
                for j=0, sm[1]-1 do begin
                    tmp =0 
                    for i=0,sm[0]-1 do tmp=tmp+ m[i,j]*n[k,i]
                    array[k,j] = tmp
                endfor
            endfor
        endif else begin
            array = fltarr(1,sm[1])
            for j=0, sm[1]-1 do begin
                tmp =0 
                for i=0,sm[0]-1 do tmp=tmp+ m[i,j]*n[i]
                array[j] = tmp
            endfor
        endelse
        
    endif else begin
    ;;;;;;;;;;;;;;;;; REVERSED MODE
        if n_elements(sn) gt 1 then begin
        ;;;;;;;;;;;;;;;;da testare
            array = fltarr(sm[1],sn[0])
            for k=0, sn[0]-1 do begin
                for j=0, sm[1]-1 do begin
                    tmp =0 
                    for i=0,sm[0]-1 do tmp=tmp+ m[sm[0]-1-i,j]*n[k,sm[0]-1-i]
                    array[k,j] = tmp
                endfor
            endfor
        endif else begin
            array = fltarr(1,sm[1])
            for j=0, sm[1]-1 do begin
                tmp =0 
                for i=0,sm[0]-1 do tmp=tmp+ m[sm[0]-1-i,j]*n[sm[0]-1-i]
                array[j] = tmp
            endfor
        endelse
        
    endelse
endelse
return, array
end
