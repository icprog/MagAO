pro effenumero, x, y, ris, errori, m, nome, IM_TYPE=IM_TYPE, n_i, n_f

;*************************************
; nome      = stringa con il nome del file
;          o array con l'immagine
; IM_TYPE   = TIFF, FITS   tipo di file
;   se non selezionata e' atteso un array
; m        = array con i centri e i raggi
;          delle 4 pup
;x = distanza lineare
;y = raggio pupilla
;ris =  risultato del fit
;errori= errori del fit
;***********************************

;imposto il percorso dell'immagine
;cd,'C:\Documents and Settings\Francesca\Documenti\IDL\Francesca\france'

;cd,'C:\utenti\Francesca\france\31-1-06'

;n_i=2 ;indice iniziale delle immagini
;n_f=6 ;indice finale delle immagini
;METTERE LE DIMENSIONI GIUSTE DEL PIXEL DEL CCD
pix_size=7.4e-3 ;mm
dist_lin=25.0    ;mm

n_im=n_f-n_i+1
y=replicate(1.,n_im)
x = replicate(1.,n_im)

x_c=replicate(1.,n_im)
y_c=replicate(1.,n_im)


for im_index=0,n_im-1 do begin

    x[im_index]=dist_lin*im_index

    if keyword_set(IM_TYPE) then begin
        if IM_TYPE EQ 'TIFF' then a=read_tiff(nome+strtrim(im_index+n_i,2)+'.tif')
        if IM_TYPE EQ 'FITS' then a=readfits(nome+strtrim(im_index+n_i,2)+'.fit')
    ;   if
    ;   if
    endif else begin
        print, 'SELEZIONA UN TIPO DI FILE!!'
        stop
    endelse

    analizza, a , m

    y[im_index] = m(0,0)   ; Raggio pupilla
    x_c[im_index] = m(0,1) ; Coord x centro
    y_c[im_index] = m(0,2) ; Coord y centro

endfor
;stop
;per controllare la quota
;controlla le dimensioni del pixel del ccd

ris_xc=LINFIT(x,x_c,sigma=errori_xc)
mx_rad=ris_xc[1]*pix_size
mx_min=mx_rad*180*60./!pi
delta_mx_min= mx_min*(errori_xc[1]/ris_xc[1])
print, 'Pendenza x ',mx_min,' min +/-', delta_mx_min
;plot, x, x_c, psym=-4



ris_yc=LINFIT(x,y_c,sigma=errori_yc)
my_rad=ris_yc[1]*pix_size
my_min=my_rad*180*60./!pi
delta_my_min= my_min*(errori_yc[1]/ris_yc[1])
print, 'Pendenza y ',my_min,' min +/-', delta_my_min



;per ricavare m e q, con i vari r e centri
ris=LINFIT(x,y,sigma=errori)
;inserire i punti
;propagazione dell'errore
f_num= 1./(ris[1]*2*pix_size)
delta_f_num=  f_num*(errori[1]/ris[1])
print, 'F number  = ', f_num ,'  +/- ', delta_f_num

print, 'Centro medio -> x=', mean(x_c), '; y=',mean(y_c)

if n_im gt 2 then begin
window,1
plot, x,y, psym=4
y_fit= x*ris[1]+ris[0]
oplot, x,y_fit
endif

end