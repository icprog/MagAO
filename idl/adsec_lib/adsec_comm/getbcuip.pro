; $Id: getbcuip.pro,v 1.1 2007/04/13 12:19:33 marco Exp $$
;
;+
;   NAME:
;    GetBcuIp
;
;   PURPOSE:
;    Get the bcu IP addressed by MirrorCtrl
;
;   USAGE:
;    err = GetBcuIp([/SWITCH,] id)
;
;   INPUT:
;    None.
;
;   OUTPUT:
;    id:    array of IP. The len of array depends on the number of BCUs.
;    err:   error code.
;
; KEYWORDS
;
;   SWITCH: address the SWITCH BCU. 
;
; HISTORY:
; 
;  Modified 19 Apr 2009 by Alfio Puglisi (AP) to not use BcuMaps
;  Created on Feb 5, 2007 by Marco Xompero (MX) 
;  Osservatorio Astrofisico di Arcetri, ITALY
;  <marco@riccardi.arcetri.astro.it>
;
;-

Function getBcuIp, id, SWITCHB=switchb

    @adsec_common

    ;st = strupcase(!AO_CONST.telescope_side,0,1)
    ;if st eq 'RIGHT' then suffix = 'dx' $
    ;else if st qt 'LEFT' then suffix = 'sx' $
    ;else return, adsec_error.input_type
   

    if keyword_set(SWITCHB) then begin
        name = "BCU_SWITCH" 
        idd = getBcuIp_wrap(name)
        if idd eq -1 then begin

            message, "READING ERROR BCU", CONT=(sc.debug eq 0B)
            return, adsec_error.input_type

        endif 
        id = idd

    endif else begin
    
        id = strarr(adsec.n_crates)
        for i=0, adsec.n_crates-1 do begin
            name="BCU_MIRROR_"+strtrim(i,2)
            idd = getBcuIp_wrap(name)
            if idd eq -1 then  return, adsec_error.input_type
            id[i] = idd
        endfor
    endelse
    
    return, adsec_error.ok

end
