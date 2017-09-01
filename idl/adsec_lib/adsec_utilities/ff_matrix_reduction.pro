;$Id: ff_matrix_reduction.pro,v 1.7 2009/09/14 08:27:39 riccardi Exp $
;+
;   NAME:
;    FF_MATRIX_REDUCTION
;
;   PURPOSE:
;    The routine returns the reduced-size feed foerard matrix corresponding to
;    a subset of actuators without remeasuring it.
;
;   USAGE:
;    new_matrix = ff_matrix_reduction(ff_matrix, index_list, KS)
;
;   INPUT:
;    ff_matrix:     original feed forward matrix to reduce.
;    index_list:    list of coloums of ff_matrix corresponding to the actuators
;                   to exclude from the new reduced feed forward matrix.
;
;   OUTPUT:
;    new_matrix:    new reduced feed forward matrix
;    dx:            slave matrix (see below)
;
;   KEYWORD:
;    None.
;
;   NOTE:
;    The actuator passed in the index_list are called slave actuators. The others are called
;    master actuators. The slave actuators keep their force to zero and do not contribute any
;    longer to the application of the FF force. That happens, for instance, when one actuator
;    becomes out of order. The original FF matrix (supposing acts in index_list properly 
;    working and contriubuting to the FF force) has to be recomputed considering the missing
;    actuators in index_list. The new matrix is called reduced matrix and has as input only the
;    master actuators.
;    The procedure for computing the reduced FF matrix is described hereafter.
;    Reordering and defining the input FFmatrix in blocks as follow (done by the code):
;    
;    K = [[K00,K01],$
;         [K10,K11]]
;        
;    where the K11 is acting on the master actuators and K00 acting on the slave
;    actuators (K01 and K10 the matrix with the consequent cross-action).
;    Moreover, setting the corresponding reordered colum vector of position and force:
;                p=transpose([p0,p1]) and f=transpose([f0,f1])
;    where, as for the K marix, the index 0 is for the slave actuators and the index 1 for
;    the master actuators. p1 are known (the desired position), f0=0 (no acting actuators).
;    
;    the linear system:
;    
;    f = K ## p
;    
;    can be rewritten as:
;    
;     0 = k00 ## p0 + k01 ## p1
;    f1 = k10 ## p0 + k11 ## p1
;     
;    then:
;    
;    p0 = (-k00^-1 ## k01) ## p1 = KS ## p1
;    f1 = (k00 ## KS + k01) ## p1 = KR ## p1
;    
;    setting:
;    KS = (-k00^-1 ## k01)
;    KR = (k10 ## KS + k11)
;    
;    we have:
;    p0 = KS ## p1
;    f1 = KR ## p1
;    
;    where KR is the reduced FF matrix (output of the function), and KS is the matrix
;    giving the positions of the slave actuators as a function of the position of master
;    actuators (reurned as optional parameter in the MATRIX_SLAVE keyword) 
;    
; HISTORY
;   28 May 2004 Armando Riccardi (AR)
;    Completely rewritten to match the LBT format.
;   22 Jul 2004 Marco Xompero (MX)
;    Checked for the new LBT specs.
;   08 Sep 2009 AR: extended help.
;-


Function ff_matrix_reduction, ff_matrix, index_list, dx, k00, k01, k10, k11

    
    n_row = n_elements(ff_matrix[*,0])

    is_not_included = complement(index_list, indgen(n_row), other_index_list $
                                , n_good)

    if is_not_included then message, "the index_list contains not valid indexes for ff_matrix"
    if n_good eq 0 then message, "The reduced ff_matrix cannot have null size"

    n_bad = n_elements(index_list)

; sub matrix definitions
; FF=NxN, k00=NBADxNBAD, k11=NGOODxNGOOD, k01 and k10 cross matrixes.


    k00 = ff_matrix[index_list,*]
    k00 = k00[*,index_list]

    k01 = ff_matrix[other_index_list,*]
    k01 = k01[*, index_list]

    k10 = ff_matrix[*,other_index_list]
    k10 = k10[index_list, *]

    k11 = ff_matrix[*,other_index_list]
    k11 = k11[other_index_list, *]

; case more than one row reduction (index_matrix elements great than 1): use svdc for
; defining dx matrix.
    if n_bad gt 1 then begin
        svdc, k00, w, u, v, /DOUBLE
        idx = where(w lt max(w)*(machar()).eps, count)
        if count ne 0 then w[idx]=0d0


        dx = dblarr(n_good,n_bad)
        b  = - k01 ## identity(n_good)
        for i=0,n_good-1 do begin
            dummy = reform(b[i,*])
            dx[i,*] = svsol(u, w, v, dummy)
        endfor
; case one actuator reduction: simple case for defining dx matrix
    endif else begin
        dx = -k01 ## identity(n_good)/k00[0]
    endelse

; effective calculation pf reduced feed forward matrix
    reduced_ff_matrix = k10 ## dx + k11

    return, reduced_ff_matrix

end
