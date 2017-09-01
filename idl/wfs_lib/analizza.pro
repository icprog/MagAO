;*****************************************
; ANALIZZA
; immagine  = immagine in ingresso da dare, array 2d
; results   = array con i centri e i raggi delle 4 pup
; SPLIT     = se è settata divide l'immagine nelle 4 pupille
; SOGL      = soglia in percentuale al picco-valle dell'intera
;             immagine, serve a togliere il background
; S2_P2V    = seconda soglia in frazione di max-min. Se non
;             settato e' una mean dei valori sopra la prima soglia
;***************************************
pro analizza, immagine, results, SOGL = SOGL, SPLIT = SPLIT, S2_P2V=S2_P2V, QUIET=QUIET, DOPLOT=DOPLOT, DRAWTO = DRAWTO, DISPFACTOR = DISPFACTOR, NOSAVE = NOSAVE, PLOT_TO = PLOT_TO

if not keyword_set(DISPFACTOR) then dispfactor = 1

start = systime(1)

dim_imm=size(immagine)

immagine[*,0] = 0
immagine[*,dim_imm[2]-1] =0
immagine[0,*] = 0
immagine[dim_imm[1]-1,*] =0

if keyword_set(SPLIT) then begin
    print,'SPLIT attivo'

    cx=fix(dim_imm[1]/2)
    cy=fix(dim_imm[2]/2)
    offset=0
    dim = cx

    ;creo array a tre dimensioni, dove la prima mi indica l'imm. delle quattro selezionata

    ; Fix thorlabs
    if cx eq 512 then begin
        cx = 540
        cy = 358
        dim = 250
    endif


    ;fix ccd39
    reduce = cx/20
    cx += reduce
    dim -= reduce

    k = fltarr(4,dim,dim)

    k[0,*,*] = immagine[cx-dim:cx-1, cy:cy+dim-1]
    k[1,*,*] = immagine[cx:cx+dim-1,   cy:cy+dim-1]
    k[2,*,*] = immagine[cx-dim:cx-1, cy-dim:cy-1]
    k[3,*,*] = immagine[cx:cx+dim-1,   cy-dim:cy-1]

    writefits,'/tmp/pup1.fits',k[0,*,*]
    writefits,'/tmp/pup2.fits',k[1,*,*]
    writefits,'/tmp/pup3.fits',k[2,*,*]
    writefits,'/tmp/pup4.fits',k[3,*,*]

    ;creo array a due dimensioni [la prima mi indica l'imm fra le quattro, la seconda le variabili (r,centro[0],centro[1])]
    results = fltarr(4,3)

endif else begin

    k = fltarr(1,dim_imm[1],dim_imm[2])
    k[0,*,*] = immagine
    results = fltarr(1,3)

endelse

if keyword_set(SPLIT) then n_pup = 4 else n_pup=1

;introduco un ciclo for perchè per ciascuna delle 4 imm devo attuare la procedura per ricavare centro e raggio


soglie = fltarr(n_pup, 2)

if not keyword_set(QUIET) then window,0, xsize=dim_imm[1]*DISPFACTOR,ysize=dim_imm[2]*DISPFACTOR
if not keyword_set(QUIET) then window,1, xsize=dim_imm[1]*DISPFACTOR,ysize=dim_imm[2]*DISPFACTOR

for pup=0,n_pup-1 do begin

    ;per estrarre una imm a due dim da un array a tre dim (si chiama 'cubo')
    imm=reform(k[pup,*,*])

    ;così mi divide la procedura per ciascuna immagine
    if not keyword_set(QUIET) then print,'Analizzo pupilla ',pup

    imm_orig = imm

    ;calcolo la soglia per ciascuna delle quattro pupille

    ; PRIMA SOGLIA
    if not keyword_set(SOGL) then begin
       s1 = median(imm) + (mean(imm)-median(imm))*.05

    endif else begin
       s1 = median(imm) + (mean(imm)-median(imm))* SOGL
    endelse

    imm_soglia = imm
    imm_soglia[ where( imm_soglia lt s1) ]=0

    ; SECONDA SOGLIA
    ;if keyword_set(S2_P2V) then s2=(MAX(imm_soglia)-MIN(imm_soglia))*S2_P2V else s2= mean(imm(where(imm gt s1)))
    if keyword_set(S2_P2V) then s2=mean(imm_soglia)*S2_P2V else s2= mean(imm(where(imm gt s1)))
    print,'Soglia: ',s2

    soglie[pup, 0] = s1
    soglie[pup, 1] = s2

    imm_soglia[ where( imm_soglia lt s2) ]=0



    ;inizia il ciclo per ciascuna imm
    repeat begin

       centro = calcola_baricentro(imm_soglia)
       r  = calcola_raggio(imm_soglia,s2)

    sottosoglia, s2, imm_soglia, r, centro, pixel, cerchio, cerchio_count

       if keyword_set(DRAWTO) then wset, DRAWTO
       if (not keyword_set(QUIET)) or (keyword_set(DRAWTO)) then begin


            m = max(imm_orig)
            i = imm_orig
            ;if cerchio_count gt 0 then i[cerchio] = m
            wset,0
            tvscl, rebin(i+imm_soglia*max(i)/(max(i)/10.0), n_elements(i[*,0])*dispfactor, n_elements(i[0,*])*dispfactor, /SAMPLE),pup
            wset,1
            tvscl, rebin(i, n_elements(i[*,0])*dispfactor, n_elements(i[0,*])*dispfactor, /SAMPLE),pup
        endif

       ;

       ; Correggo l'origine per le varie pupille solo se ho splittato in 4

       if keyword_set(SPLIT) then begin
         if pup eq 0 then centro = centro + [cx-dim,cy]
         if pup eq 1 then centro = centro + [cx,cy]
         if pup eq 2 then centro = centro + [cx-dim,cy-dim]
         if pup eq 3 then centro = centro + [cx,cy-dim]

       endif


       ;per salvare i dati per ciascun ciclo iterativo


    ;; AGGIUSTINO2

       ;results[pup,*]=[r,centro[0], centro[1]]
       results[pup,*]=[r,centro[0] -0.5 ,centro[1] +0.5]

       if not keyword_set(QUIET) then begin
         ;print, '______________________'
            print, 'Pupilla', pup, '        Diametro =', 2.*r ,'       Coord centro =',centro
        endif

    endrep until (pixel eq 0)


    if not keyword_set(NOSAVE) then save, results ,filename='risultati'+ string(long(systime(1))) +'.sav'
    if not keyword_set(QUIET) then print ,'ho finito'

endfor


if keyword_set(DOPLOT) then begin
    if keyword_set(PLOT_TO) then wset, PLOT_TO else window,7
    if keyword_set(SPLIT) then begin
        !p.multi = [0,4,1]
    endif
    for pup=0,n_pup-1 do begin

       ; Correggo l'origine per le varie pupille solo se ho splittato in 4
       sezione_plot = results[pup,2]
       centro_plot  = results[pup,1]
       if keyword_set(SPLIT) then begin
         if pup eq 0 then sezione_plot = sezione_plot - d
         if pup eq 1 then sezione_plot = sezione_plot - d

         if pup eq 1 then centro_plot  = centro_plot - c
         if pup eq 3 then centro_plot  = centro_plot - c
       endif

        plot, k[pup, *,round(sezione_plot)]
        oplot, [centro_plot-r,centro_plot-r],[0,MAX(immagine)], color=255
        oplot, [centro_plot+r,centro_plot+r],[0,MAX(immagine)], color=255

       oplot, [-1,1e6], [soglie[pup,0], soglie[pup,0]], color = 255
       oplot, [-1,1e6], [soglie[pup,1], soglie[pup,1]], color = 255
    endfor
    if keyword_set(SPLIT) then begin
        !p.multi = 0
    endif
 endif

fine = systime(1)

end
