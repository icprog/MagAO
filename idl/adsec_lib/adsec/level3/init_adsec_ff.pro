;+
;   NAME:
;    INIT_ADSEC_FF
;
;   PURPOSE:
;       Restore the feedforward matrix
;
;   USAGE:
;    err = init_adsec_ff(FF_FILENAME=ff_filename, STRUCT=struct, NOFILL=nofill)
;
;   INPUT:
;       None.
;   OUTPUT:
;    Error code.
;
;   KEYWORD:
;
;    FF_FILENAME : path of file where is saved the ff_matrix
;    STRUCT      : structure creates in the file in which the feedforward data are saved.
;    NOFIL       : If it is set, the adsec feedforward fields are filled up. 
;
;  COMMON BLOCKS:
;   ADSEC               : secondary adaptive base common block. A part of it will be filled.
;
;  PACKAGE:
;   LBT Adsec Libraries.
;
;  HISTORY
;   Created on 28 Oct 2004 by Daniela Zanotti (DZ).
;
;   NOTE:
;    None.
;-

function init_adsec_ff, FF_FILENAME=ff_filename, STRUCT=struct, NOFILL=nofill

@adsec_common

; Restoring the feedforward matrix
;
n_acts = adsec.n_actuators


if (keyword_set(ff_filename)) then begin

    act_ff = file_dirname(ff_filename)+'/ff_list.fits'    
    u_ff = file_dirname(ff_filename)+'/ff_f_svec.fits'    
    v_ff = file_dirname(ff_filename)+'/ff_p_svec.fits'    
    w_ff = file_dirname(ff_filename)+'/ff_sval.fits'    
    check = (file_info(ff_filename)).exists and (file_info(ff_filename)).regular 
    check1 = (file_info(act_ff)).exists and (file_info(act_ff)).regular 
    check2 = (file_info(u_ff)).exists and (file_info(u_ff)).regular 
    check3 = (file_info(v_ff)).exists and (file_info(v_ff)).regular 
    check4 = (file_info(w_ff)).exists and (file_info(w_ff)).regular 
    
    if total([check[0], check1[0],check2[0],check3[0], check4[0]])  eq 0  then begin
        log_print, log_lev=!AO_CONST.log_lev_error, 'The file '+ff_filename+ " containing the feedforward matrix doesn't exist"
        log_print,log_lev=!AO_CONST.log_lev_warning, 'A zero matrix will be used as feedforward matrix'
        meas_ff_used = 0B
        ff_matrix = fltarr(n_acts,n_acts)
        u = -(identity(n_acts))
        v = -u
        w = replicate(0.0, n_acts)
        ff_filename = ""
    endif else begin
    
        found_error=0
        ff_matrix =  readfits(ff_filename, /SILENT) 
        !AO_STATUS.ff_matrix = ff_filename
        u = readfits(u_ff, /SILENT)
        w = readfits(w_ff, /SILENT) 
        v = readfits(v_ff, /SILENT)
        act_w_ff=readfits(act_ff, /SILENT)
;        restore, ff_filename
        meas_ff_used = 1B
        n_act_w_ff = n_elements(act_w_ff)
                                                                                                                             
        if test_type(ff_matrix, /REAL, DIM=dim) then begin
            log_print, log_lev=!AO_CONST.log_lev_error, "The feedforward matrix must be real"
            found_error+=1
        endif
        if dim[0] ne 2 then begin
            log_print, log_lev=!AO_CONST.log_lev_error, "The feedforward must be a 2-D matrix"
            found_error+=1
        endif
        if total(dim eq [2, n_act_w_ff, n_act_w_ff]) ne 3 then begin
            log_print, log_lev=!AO_CONST.log_lev_error, "The feedforward matrix must be a " +strtrim(n_act_w_ff,2)+"x"+strtrim(n_act_w_ff,2)+" matrix."
            found_error+=1
        endif
                                                                                                                             
        if test_type(u, /REAL, DIM=dim) then begin
            log_print, log_lev=!AO_CONST.log_lev_error, "The u matrix must be real"
            found_error+=1
        endif
        if dim[0] ne 2 then begin
            log_print, log_lev=!AO_CONST.log_lev_error, "The u must be a 2-D matrix"
            found_error+=1
        endif
        if total(dim eq [2, n_act_w_ff, n_act_w_ff]) ne 3 then begin
            log_print, log_lev=!AO_CONST.log_lev_error, "The u matrix must be a " +strtrim(n_act_w_ff,2)+"x"+strtrim(n_act_w_ff,2)+" matrix."
            found_error+=1
        endif
                                                                                                                             
        if test_type(v, /REAL, DIM=dim) then begin
            log_print, log_lev=!AO_CONST.log_lev_error, "The v matrix must be real"
            found_error+=1
        endif
        if dim[0] ne 2 then begin
            log_print, log_lev=!AO_CONST.log_lev_error, "The v must be a 2-D matrix"
            found_error+=1
        endif
        if total(dim eq [2, n_act_w_ff, n_act_w_ff]) ne 3 then begin
            log_print, log_lev=!AO_CONST.log_lev_error, "The v matrix must be a " +strtrim(n_act_w_ff,2)+"x"+strtrim(n_act_w_ff,2)+" matrix."
            found_error+=1
        endif
                                                                                                                             
        if test_type(w, /REAL, DIM=dim) then begin
            log_print, log_lev=!AO_CONST.log_lev_error, "The v matrix must be real"
            found_error+=1
        endif
        if dim[0] ne 1 then begin
            log_print, log_lev=!AO_CONST.log_lev_error, "The w must be a vector"
            found_error+=1
        endif
        if total(dim eq [1, n_act_w_ff]) ne 2 then begin
            log_print, log_lev=!AO_CONST.log_lev_error, "The w must be a " +strtrim(n_act_w_ff,2)+" elements vector."
            found_error+=1
        endif
                                                                                                                             
        if n_elements(act_w_ff) lt n_elements(adsec.act_w_cl) then  begin
            log_print, log_lev=!AO_CONST.log_lev_error, "The # of act in closed loop cannot be greater than the # of act with FF or wrong ff_matrix loaded"
            found_error+=1
        endif
                                                                                                                             
        is_not_included = complement(adsec.act_w_cl, act_w_ff, act_wo_ff, n_act_wo_ff)
        if is_not_included then begin
            log_print, log_lev=!AO_CONST.log_lev_error, "the list of act with closed loop must be included the list of act with FF"
            found_error+=1
        endif
        if found_error gt 0 then begin
            log_print, "At least one error on feed forward matrix found: using null matrix."
            log_print,log_lev=!AO_CONST.log_lev_warning, 'A zero matrix will be used as feedforward matrix'
            meas_ff_used = 0B
            ncl = n_elements(adsec.act_w_cl)
            ff_matrix = fltarr(ncl, ncl)
            u = -(identity(ncl))
            v = -u
            w = fltarr(ncl)
            ff_filename = ""
            if n_elements(n_act_wo_ff) eq 0 then n_act_wo_ff = 0
        endif

        if n_act_wo_ff ne 0 then begin
            log_print, log_lev=!AO_CONST.log_lev_warning, "The actuators in closed loop are less then the FF ones"
            log_print, log_lev=!AO_CONST.log_lev_warning, "The FF matrix will be reduced."
            idx_list = lonarr(n_act_wo_ff)
            for i=0,n_act_wo_ff-1 do idx_list[i]=where(act_w_ff eq act_wo_ff[i])
            ff_matrix = ff_matrix_reduction(ff_matrix, idx_list)
            act_w_ff = adsec.act_w_cl
            svdc, ff_matrix, w, u, v, /DOUBLE
                                                                                                                             
            idx = sort(w)
                                                                                                                             
         ;; the singular vector are ordered from the less to the most "stiff"
            u = u[idx, *]
            v = v[idx, *]
            w = w[idx]
        endif
                                                                                                                             
        dummy_ff_matrix = fltarr(n_acts,n_acts)
        for i=0,n_elements(adsec.act_w_cl)-1 do begin
            dummy_ff_matrix[adsec.act_w_cl,adsec.act_w_cl[i]] = ff_matrix[*,i]
        endfor
        ff_matrix=dummy_ff_matrix
                                                                                                                             
        dummy = dblarr(n_acts,n_acts)
        dummy[0:n_elements(adsec.act_w_cl)-1,adsec.act_w_cl]=u
        u=dummy
                                                                                                                             
        dummy = dblarr(n_acts,n_acts)
        dummy[0:n_elements(adsec.act_w_cl)-1,adsec.act_w_cl]=v
        v=dummy
                                                                                                                             
        dummy=dblarr(n_acts)
        dummy[0:n_elements(adsec.act_w_cl)-1]=w
        w=dummy
                                                                                                                             
    endelse
    !AO_STATUS.FF_MATRIX=ff_filename
endif else begin
        log_print, "No file containing ff_matrix was selected "
        log_print, 'A zero matrix will be used as feedforward matrix'
        !AO_STATUS.FF_MATRIX=''
        meas_ff_used = 0B
        ff_matrix = fltarr(n_acts,n_acts)
        u = -(identity(n_acts))
        v = -u
        w = replicate(0.0, n_acts)
endelse
;
; End of: Restoring the feedforward matrix
;
;===================================================================================================
;
struct = $
  { $
    meas_ff_used : meas_ff_used ,$  ; 1B: a k matrix stored in data dir has been used (0B: eitherwise)
    ff_matrix : ff_matrix ,$ ; feedforward matrix: n_act X n_act (ccount/pcount)
    ff_f_svec : u ,$ ; force singular vectors (per columns)
    ff_p_svec : v ,$ ; position singular vectors (per columns)
    ff_sval : w $ ; singular values (ordered from the lowest to highest) ccount/pcount
 }
;
;==================================================================================================
;
if not(keyword_set(nofill)) then begin
    adsec.meas_ff_used = meas_ff_used ; 1B: a k matrix stored in data dir has been used (0B: eitherwise)
    adsec.ff_matrix = ff_matrix ; feedforward matrix: n_act X n_act (ccount/pcount)
    adsec.ff_f_svec = u ; force singular vectors (per columns)
    adsec.ff_p_svec = v ; position singular vectors (per columns)
    adsec.ff_sval = w ; singular values (ordered from the lowest to highest) ccount/pcount
endif


return, adsec_error.ok
end
