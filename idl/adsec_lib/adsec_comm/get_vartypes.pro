;$Id: get_vartypes.pro,v 1.1 2005/11/29 18:23:12 labot Exp $

;+
;   NAME:
;     GET_VARTYPE
; 
;   PURPOSE:
;     This function get the types code of MsgRTDB variables.
;
;   CALLING SEQUENCE:
;     err = get_vartype(types)
;
;   OUTPUTS:
;     types: int sequence of variable types to use. see Supervisor/lib/rtdblib.h.
;
;
; HISTORY:
;
;   Created by  Marco Xompero on 29 Jul 2005
;   <marco@arcetri.astro.it>
;
;-
Function get_vartypes, types

    @adsec_common

    len = 7
    types = lonarr(len)
    err = call_external(sc.dll_name, 'get_vartypes', len, types)
    return, err

End
