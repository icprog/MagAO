; *******************
; CALCOLA_BARICENTRO
; Calcola il baricentro della pupilla
; *******************
function calcola_baricentro, imm_pupil
common baricentro, old_p, old_q, m_y, m_x, imm_selected

    start = systime(1)

    imm_bar=size(imm_pupil)
    p=imm_bar[1]
    q=imm_bar[2]

    ;prodotto fra matrici
    recalc=0
    if not keyword_set(m_y) then recalc=1
    if not keyword_set(old_p) then recalc=1

    if keyword_set(old_p) then begin
        if (old_p ne p) or (old_q ne q) then recalc=1
    endif


    if recalc eq 1 then begin
        m_y=findgen(q)##replicate(1,p)
        m_x=replicate(1,q)##findgen(p)
        old_p = p
        old_q = q
    endif

    ; modificato per funzionare con pupu apodizzate
    ;t = total(imm_pupil)
    ;y_b=total(m_y*imm_pupil) / t
    ;x_b=total(m_x*imm_pupil) / t


    indx = where(imm_pupil gt 0,count)

;    y_b = total(indx / p) / n_elements(indx)
;    x_b = total(indx mod p) / n_elements(indx)
;
;    	fine = systime(1)
;	print,'Calcola baricentro: ',fine-start
;
;    return,[x_b, y_b]

    imm_selected = fltarr(p,q)
    if count gt 0 then begin
        imm_selected[indx] = 1

        y_b= total(m_y*imm_selected)/count
        x_b= total(m_x*imm_selected)/count

        ; -------------------------------------------
        ; AGGIUSTINO
        ; ----------------------------------
        ; Aggiusta le coordinate perche' un pixel
        ; completamente illuminato ha il centro a 0.5 e non a 0.

        ;coord_bar=[x_b+0.5, y_b+0.5]
        coord_bar=[x_b, y_b]

    	fine = systime(1)
	;print,'Calcola baricentro: ',fine-start
       return, coord_bar
    endif else begin
        return,[0,0]
    endelse



end
