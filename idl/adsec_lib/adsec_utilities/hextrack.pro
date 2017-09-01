;wv:[tx, ty, focus, cx, cy]
function hextrack, wv, APPLY=apply, PUPIL_XYRE=pupil_xyre, newpos, wv1, wv2,ONLY_TILT=only_tilt

    @adsec_common
    
    verbose=1
    M = transpose([[4.5960,	10.373,0.155,	-0.697,	-2.1660] $
                    ,[-0.897,	-0.581,	2.0490,	0.01150,	-0.0276] $
                    ,[10.74710,	-3.3170,	-0.1320,	-2.1770,	0.70350] $
                    ,[4.698,	20.1450,	-0.00250,	-0.03880,	-0.105600] $
                    ,[-20.9092,	6.8090,	0.00860,	0.0580,-0.03360]])
    V  = transpose([[0,500,0,-50,0], [0,0,30,0,0], [500,0,0,0,50], [0,0,0,3,0],[0,0,0,0,3]])


M1=transpose([[   -3.8697812, -5.6334329     , 0.27591824     ,-0.061868779    , -0.028874575] $
    ,[-0.73317451    , -0.33301707    ,-0.0024143259   , -0.019180468   ,  0.071952116] $ 
    ,[-0.52078392    ,-0.016812523    ,0.018393563     ,0.0078195205    ,-0.011719705]  $
    ,[ 0.22244518    ,  -1.1060650    ,-0.0049812205   ,  0.031248066   , 0.0031771983] $
    ,[-0.31425937    ,  0.51857302    , 0.057193910    ,-0.019917420    ,0.0010890614]  $
    ,[-0.11865190    , -0.43852722    ,-0.0089586769   , -0.012495520   , 0.0049653239] $
    ,[ 0.35938721    , 0.096030767    ,0.0016866963    , 0.034989882    , 0.015261012]  $
    ,[-0.51094730    , -0.14537202    ,-0.018337000    ,-0.00036217360  ,  -0.018442897] $
    ,[ 0.39498719    , 0.036082815    , 0.013376949    ,-0.014342107    ,0.0027768197] $
    ,[ 0.57915805    , -0.30238029    ,-0.033424935    ,0.0071796971    ,-0.016885422] $
    ,[-1.1069660     , 0.66502843     ,-0.011605620    ,-0.0041833628   ,-0.0041242032]])

M1 = M1[indgen(9)+1,*]-M1[indgen(9),*]
 
V1 = transpose([[12.633852       ,18.994577      ,-4.1067708     ,-2.9331890      ,1.6679777]     $
       ,[9.3513049       ,-13.598568     ,0.12827041     , 1.3254434      ,1.0429464]     $
       ,[-2.1326811      ,2.1839187      ,-0.29058883    , -0.25243028    , -0.14864544]  $
       ,[-6.6426326      ,-1.5655279     ,0.056276975    , 0.019196210    , -0.74983276]  $
       ,[4.4399890       , 0.75332069    , -0.82616448   , -0.023749069   ,   0.51890838] $
       ,[3.0275067       ,0.32025683     , 0.14013155    ,-0.098710687    ,  0.31394920]  $
       ,[-6.6041314      ,-5.8850793     ,-0.020033987   , 0.63758813     ,-0.72899175]   $
       ,[-0.80672497     ,4.2369468      ,0.24280986     ,-0.47974770     ,-0.017899018]  $
       ,[2.9375010       ,0.28973111     ,-0.18380918    ,-0.0080169206   , 0.25201658]   $
       ,[-3.0059122      ,3.2681844      ,0.46664383     ,-0.35127320     ,-0.39614370]   $
       ,[1.4270501       ,0.60312820     ,0.16595834     ,-0.019545866    ,0.31517391]])
 
V1 = V1[indgen(9), *]

    acquire4D = n_elements(wv) eq 0
    if acquire4d then begin
        err = idl_4d_init()
        name_radix = "hex"
        err = get_m4d(name_radix, opd2flatten, mask2flatten)
        if err ne adsec_error.ok then message, "ERROR!!"
        err = idl_4d_finalize()


        sopd = size(opd2flatten, /DIM)
        idx_m = where(mask2flatten)

        ;print, "Start pupil fitting..."
        ;pupil_xyre = pupilfit(mask2flatten, /CIRCLE)
        ;print, "End pupil fitting: ", pupil_xyre
        if n_elements(pupil_xyre) eq 0 then pupil_xyre = [497.011, 520.275, 461.568, -0.102177]
        xx = rebin((dindgen(sopd[0])-pupil_xyre[0])/(pupil_xyre[2]), sopd)
        yy = rebin((dindgen(1,sopd[1])-pupil_xyre[1])/(pupil_xyre[2]), sopd)
        fit = surf_fit(xx[idx_m], yy[idx_m], double(opd2flatten[idx_m]), indgen(10)+1, COEFF=coeff, /ZERN)
        ;normalization of Noll coefficients to 4D coefficients
        coeff = coeff[indgen(10)]
        norm_v = [1, 2, 2, sqrt(3),sqrt(6), sqrt(6), sqrt(8), sqrt(8), sqrt(8), sqrt(8)]
        lambda = 632.79999999999995000000d
        cnorm = (coeff*norm_v)*1d9/lambda *2 ;in lambda, wavefront, 4D normalized
        ;normalized coefficient reordering to match the 4D order
        cnorm_r = cnorm[[0,1,2,3,5,4,7,6,9,8]]
        wv1 = transpose(cnorm_r[[1,2,3,6,7]])
        wv = wv1
        if keyword_set(only_tilt) then wv[2:4]=0.
        if keyword_set(VERBOSE) then print, "WAVES", wv
        
        
    endif


    Mplus = pseudo_invert(M)
    tf_mat = V ## Mplus
;    dpar = -(tf_mat ## transpose(wv))
    alpha= atan(3.95/38.6)+!pi
    wvc = -wv
    wvc[2] = -wvc[2]
    dpar = -(tf_mat ## transpose(wvc))

;   M1plus = pseudo_invert(M1)
;    tf_mat = V1 ## M1plus
;    dpar = -(tf_mat ## transpose(wv))


    if keyword_set(VERBOSE) then print, "HXP MOVEMENT", dpar

    if keyword_set(APPLY) then begin
        err = hexgetpos(actual_pos)
        if err ne adsec_error.ok then return, adsec_error.generic_error
        newpos = dpar+actual_pos
        if total(abs(newpos[[0,1, 2]]) gt [5000, 5000, 5000]) gt 0 then begin
            return, adsec_error.generic_error
        endif
        if total(abs(newpos[[3, 4]]) gt [900, 900]) gt 0 then begin
            return, adsec_error.generic_error
        endif
        
        err = hexmoveby([dpar[0], dpar[1], dpar[2], dpar[3], dpar[4], 0.0])
        if err ne 0 then return, err
    endif

 
    return, dpar

end



      
