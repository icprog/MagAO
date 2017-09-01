pro distanza_centri, nome, IM_TYPE=IM_TYPE, m, w, diagonale_lato, media_d, QUIET = QUIET, OUTFILE = OUTFILE, SPLIT = SPLIT, TH1 = TH1, TH2 = TH2, DISPFACTOR = DISPFACTOR

;******************************************
; nome      = stringa con il nome del file
;          o array con l'immagine
; IM_TYPE   = TIFF, FITS   tipo di file
;   se non selezionata e' atteso un array
; m        = array con i centri e i raggi
;          delle 4 pup
; w         = misura dei lati del quadrato
;******************************************


;Caricamento immagine
if keyword_set(IM_TYPE) then begin
    if IM_TYPE EQ 'TIFF' then a=read_tiff(nome)
    if IM_TYPE EQ 'FITS' then a=readfits(nome)
endif else a=nome

; SE IL FRAME HA DIMENSIONI DISPARI LE AGGIUSTA IN PARI
new_size=intarr(2)
dimensioni=size(a)
for i=0,1 do begin
	if dimensioni[i+1]/2. GT dimensioni[i+1]/2 then begin
		new_size[i]=dimensioni[i+1]+1
		print, 'dimensione',i+1,' variata:', dimensioni[i+1], '->',new_size[i]
	endif else new_size[i]=dimensioni[i+1]
endfor
dummy_a=fltarr(new_size[0],new_size[1])
dummy_a[0:dimensioni[1]-1,0:dimensioni[2]-1]=a
a=dummy_a


analizza, a , m, SOGL = TH1, S2_P2V = TH2, SPLIT = SPLIT, QUIET = QUIET, /NOSAVE, DISPFACTOR = DISPFACTOR

w=replicate(1.,4)
media_d=fltarr(1.,4)

if keyword_set(SPLIT) then begin

for i=0,3 do begin
    ;il mod serve per bloccarmi il ciclo, lavora con il resto della divisione
    ;ad es. 3+1=4 -->4/4 ha resto zero , quindi mi porta l'indice j=0

    j=(i+1) mod 4

    d_a=sqrt((m(j,1)-m(i,1))^2  + (m(j,2)-m(i,2))^2)


    if i EQ 1 OR i EQ 3 then begin

       if not keyword_set(QUIET) then print,'diag',i,'->',j, '(pix) =', d_a

    endif else begin

       if not keyword_set(QUIET) then print, 'lato ',i,'->',j ,'(pix) =', d_a

       w[i]=d_a
    endelse

endfor

for h=0,1 do begin

    k=(h+2) mod 4

    d_b=sqrt((m(k,1)-m(h,1))^2  + (m(k,2)-m(h,2))^2)

    if not keyword_set(QUIET) then print, 'lato ',h,'->',k ,'(pix) =', d_b

    w[h*2+1]=d_b

endfor
for pup=0,3 do begin

if not keyword_set(QUIET) then print, 'Pupilla n.',pup+1,'   diam=', m(pup,0)*2,'pix'

media_d[pup]=m(pup,0)*2

endfor

;stop

media_diam=mean(media_d)

if not keyword_set(QUIET) then print, 'diametro medio', media_diam

media=mean(w)

if not keyword_set(QUIET) then print, 'media_lato =' , media

diagonale_lato=sqrt(2)*media

if not keyword_set(QUIET) then print, 'diagonale =' , diagonale_lato

rapporto=media_diam/media

if not keyword_set(QUIET) then print, 'rapporto =', rapporto

endif else begin  ; keyword SPLIT

media_d[0]=m(0,0)*2

endelse

; Text output in this format:
;
; d1, x1, y1, l1
; d2, x2, y2, l2
; d3, x3, y3, l3
; d4, x4, y4, l4

print, media_d
print, m

n_pup = 3
if not keyword_set(SPLIT) then n_pup =0
if keyword_set(OUTFILE) then begin
    openw, unit, OUTFILE, /GET_LUN
    for pup=0,n_pup do printf, unit, media_d[pup], m[pup,1], m[pup,2], w[pup]
    close, unit
    free_lun, unit
endif

end



