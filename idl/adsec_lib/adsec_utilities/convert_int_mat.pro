;SCOPE: interaction matrix conversion after a change on FF matrix (actuator lost, new fittings, ....)
;IM1 = IM ## transpose(V) ## [[I];[P]] ## (V1)


;sample paramenters:
;w_int_mat   = '/towerdata/adsec_calib/M2C/KL/RECs/Intmat_20091005_111109.fits'
;old_im_file = '/towerdata/adsec_calib/flat/2009_08_17/653modes_2009_08_10.sav'
;old_ff_file = '/home/adopt/work/AO/Supervisor/conf/adsec/672a/data/ff_matrix_OAT_calibrated.sav'
;old_kl_file = '/home/adopt/work/AO/Supervisor/conf/adsec/672a/data/kl_04.fits'
;new_ff_file = '/towerdata/adsec_calib/ff_matrix/2009_10_29/ff_matrix_+6.005e-05_3.400e-06m_c.sav'
Function convert_int_mat , old_ff_file, new_ff_file, old_im_file  

    @adsec_common
    restore, old_ff_file, /ver
    cl0 = act_w_ff
    U0 = u
    W0 = w
    V0 = v
    FF0 = ff_matrix

    restore, new_ff_file, /ver
    cl1 = act_w_ff
    U1 = u
    W1 = w
    V1 = v
    FF1 = ff_matrix

    restore, old_im_file, /ver
    IM0 = int_mat

    res = intersection(cl0, cl1, cl_int)
    if res eq 1 then begin
        log_print, "ERROR on the provided files: no conversion is possible."
        return, adsec_error.input_type
    endif
    if n_elements(cl0) eq n_elements(cl1) then begin

        if n_elements(cl_int) ne n_elements(cl0) then begin
            log_print, "ERROR: no conversion is possible. CL act no matching in old FF and old INT MAT"
            return, adsec_error.input_type
        endif
     
        IM1 = IM0 ## transpose(V0) ## (V1)
    
    endif else begin        
        
        res = complement(cl_int, cl0, act_removed)
        if res eq 1 then begin
            log_print, "ERROR on the provided files: no conversion is possible. Wrong CL act matching."
            return, adsec_error.input_type
        endif
            
        index_list = act_removed
            other_index_list = cl_int
        k00 = ff1[index_list,*]
        k00 = k00[*,index_list]

        k01 = ff1[other_index_list,*]
        k01 = k01[*, index_list]

        k10 = ff1[*,other_index_list]
        k10 = k10[index_list, *]

        k11 = ff1[*,other_index_list]
        k11 = k11[other_index_list, *]
        

        P = (-pseudo_invert(K00) ## K01)
        RM = dblarr(n_elements(cl1), n_elements(cl0))
        RM[cl_int, cl_int] = 1d
        RM[act_removed, *] = P
    
        IM1 = IM0 ## transpose(V0) ## (V1)
    endelse

    mask2flatten = mask2flatten[x0:x1, y0:y1]
    dummy= fltarr(n_pix_x, n_pix_y)
    dummy[0,0] = mask2flatten
    r_weight2flatten = rebin(dummy, n_meta_pix_x, n_meta_pix_y)
    r_mask2flatten = r_weight2flatten ge 0.6

    opd2flatten = opd2flatten[x0:x1, y0:y1]
    dummy = fltarr(n_pix_x, n_pix_y)
    dummy[0,0] = opd2flatten
    r_opd2flatten = rebin(dummy, n_meta_pix_x, n_meta_pix_y)
    idx = where(r_weight2flatten gt 0)
    r_opd2flatten[idx]=r_opd2flatten[idx]/r_weight2flatten[idx]


    r_xx = xx[x0:x1, y0:y1]
    dummy = fltarr(n_pix_x, n_pix_y)
    dummy[0,0] = r_xx
    r_xx = rebin(dummy, n_meta_pix_x, n_meta_pix_y)
    
    r_yy = yy[x0:x1, y0:y1]
    dummy = fltarr(n_pix_x, n_pix_y)
    dummy[0,0] = r_yy
    r_yy = rebin(dummy, n_meta_pix_x, n_meta_pix_y)
 
    outfile = getenv('ADOPT_MEAS')+'/'+'im_reduced_from_'+file_basename(old_ff_file)+ $
                '-'+file_basename(new_ff_file)+'-'+file_basename(old_im_file)

        int_mat = im1
        save, FILE=outfile , N_MODES, INT_MAT,PUPIL_XYRE,N_META_PIX_X, N_META_PIX_Y, CL_INT, INTERSECT_MASK_IDX $
                           , r_xx, r_yy


    return, adsec_error.ok

End
