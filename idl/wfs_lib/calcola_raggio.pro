; *******************
; Calcola_raggio
; Calcola il raggio assumendo come area il numero dei pixel sopra la soglia
; *******************
function calcola_raggio,imm_soglia,s

    i=where(imm_soglia ge s )
    ;per contare gli elementi
    e=n_elements(i)
    ;l'area sarà il numero di px totali
    dim_imm=e/!pi

    return, sqrt(dim_imm)

end
