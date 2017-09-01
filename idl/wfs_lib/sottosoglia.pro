;***************************************************
; SOTTOSOGLIA
; Mette al valore di soglia tutti i pix dentro il raggio trovato
;***************************************************
pro sottosoglia,s,imm_soglia,r,centro, pixel, cerchio, cerchio_count
common sottosoglia, done_alloc, xi, yi, old_centrox, old_centroy, old_h, old_k

    start = systime(1)

    soglia_remp = .1
    imm_elem=size(imm_soglia)
    h=imm_elem[1]
    k=imm_elem[2]

    r2 = r^2

    if keyword_set(done_alloc) then begin
        if (h ne old_h) or (k ne old_k) then done_alloc=0
    endif

    if not keyword_set(done_alloc) then begin
    	xi = (indgen(h)-centro[0]) ## replicate(1,k)
    	xi = transpose(xi)
    	yi = (indgen(k)-centro[1]) ## replicate(1,h)
	    done_alloc=1
	    old_centrox = centro[0]
	    old_centroy = centro[1]
	    old_h = h
	    old_k = k
    endif else begin
    	xi -= (centro[0] - old_centrox)
	    yi -= (centro[1] - old_centroy)
	    old_centrox = centro[0]
	    old_centroy = centro[1]
    endelse

    d2 = xi^2+yi^2
    sotto =  where(d2 lt (r-soglia_remp)^2, count)
    changed = where(imm_soglia[sotto] ne s, count_changed)
    if count_changed gt 0 then begin
        imm_soglia[ sotto] = s
        pixel = count
    endif else begin
        pixel = 0
    endelse

; Not used on W
;    cerchio = where(abs(d2 - r2) lt 10, cerchio_count)

    fine = systime(1)
    ;print,'Sottosoglia:',fine-start

end
