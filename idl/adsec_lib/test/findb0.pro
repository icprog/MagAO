;This function computes the Recontruction Matrix c=Rs
;
Function findb0, cv, sv

    tsv = transpose(sv)
    svdc, sv ## tsv, w, u, v, /double
    wz = where(w le (machar()).eps*max(w), COMPLEMENT=wnz)
    winv = fltarr(n_elements(w))
    winv[wnz] = 1./w[wnz]
    inv_mat = v ## diagonal_matrix(winv) ## transpose(u)

    return, float(cv ## tsv ## inv_mat)

End
