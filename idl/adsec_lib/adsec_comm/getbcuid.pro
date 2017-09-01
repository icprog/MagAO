; $Id: getbcuid.pro,v 1.3 2007/04/13 12:19:33 marco Exp $$
;
;+
;   NAME:
;    GetBcuID
;
;   PURPOSE:
;    Get the correct ID to address the MirrorCtrl
;
;   USAGE:
;    err = GetBcuID([/SWITCH,] id)
;
;   INPUT:
;    None.
;
;   OUTPUT:
;    id:    array of ID. The len of array depends on the number of BCUs.
;    err:   error code.
;
; KEYWORDS
;
;   SWITCH: address the SWITCH BCU. 
;
; HISTORY:
;
;  Created on Feb 5, 2007 by Marco Xompero (MX) 
;  Osservatorio Astrofisico di Arcetri, ITALY
;  <marco@riccardi.arcetri.astro.it>
;
;-

Function getBcuId, id, SWITCHB=switchb

    @adsec_common

    if keyword_set(SWITCHB) then begin
        name = "BCU_SWITCH" 
        idd = getBcuId_wrap(name)
        if idd eq -1 then begin

            message, "READING ERROR BCU", CONT=(sc.debug eq 0B)
            return, adsec_error.input_type

        endif 
        id = idd

    endif else begin
    
        id = intarr(adsec.n_crates)
        for i=0, adsec.n_crates-1 do begin
            name="BCU_MIRROR_"+strtrim(i,2)
            idd = getBcuId_wrap(name)
            if idd eq -1 then  return, adsec_error.input_type
            id[i] = idd
        endfor
    endelse
    
    return, adsec_error.ok

end
