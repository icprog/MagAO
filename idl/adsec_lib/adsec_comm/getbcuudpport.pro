; $Id: getbcuudpport.pro,v 1.2 2007/05/29 18:15:13 riccardi Exp $$
;
;+
;   NAME:
;    GetBcuUdpPort
;
;   PURPOSE:
;    Get the correct UDP port to address the MasterDiagnostic process.
;
;   USAGE:
;    err = GetBcuUdpPort([/SWITCH,] id)
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

Function getBcuUdpPort, id, SWITCHB=switchb

    @adsec_common

    if keyword_set(SWITCHB) then begin
        name = "BCU_SWITCH"
        if sc.name_comm eq "Dummy" then idd=8000 else begin
	        idd = getBcuMasterUdpPort_wrap(name)
	        if idd eq -1 then begin

	            message, "READING ERROR BCU", CONT=(sc.debug eq 0B)
	            return, adsec_error.input_type

	        endif
		endelse
        id = idd
    endif else begin

        id = intarr(adsec.n_crates)
        for i=0, adsec.n_crates-1 do begin
        	if  sc.name_comm eq "Dummy" then idd=8001+i else begin
		        name="BCU_MIRROR_"+strtrim(i,2)
		        idd = getBcuMasterUdpPort_wrap(name)
		        if idd eq -1 then  return, adsec_error.input_type
		    endelse
	        id[i] = idd
        endfor
    endelse

    return, adsec_error.ok

end
