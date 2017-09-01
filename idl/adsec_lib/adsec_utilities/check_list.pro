; $Id: check_list.pro,v 1.4 2007/03/26 17:43:38 labot Exp $
;+
; HYSTORY
;   13 July 2004 written by A. Riccardi
;   02 Nov 2004, MX
;     Adsec.all changed in sc.all.
;   20 Mar 2007, MX & DZ
;     GREATER_EQUAL keyword added
;-

function check_list, list, n_max, NOALL=no_all, GREAT_EQUAL=great_equal

    ; returns 1B if any of the list items is not valid. 0B otherwise.
    @adsec_common 

    if test_type(list, /NOFLOAT, n_el=n) then $
      return, adsec_error.IDL_INPUT_TYPE

    ; if list eq sc.all, list must have only one element
    if (n eq 1 and list[0] eq sc.all) then begin
        if keyword_set(no_all) then $
          return, adsec_error.IDL_INPUT_TYPE $
        else $
          return, adsec_error.ok
    endif

    ; test if board_list is empty or if there are too many channels
    if keyword_set(GREAT_EQUAL) then begin
        if (n eq 0 or n gt n_max) then return, adsec_error.IDL_INPUT_TYPE
    endif else begin
        if (n eq 0 or n ne n_max) then return, adsec_error.IDL_INPUT_TYPE
    endelse


    ; test if any board_list is out of range
    if (min(list) lt 0 or max(list) gt n_max-1L) then $
      return, adsec_error.IDL_INPUT_TYPE

    if n gt 1 then begin
        ; test if any list item is passed twice
        dummy = uniq(list, sort(list))
        if n_elements(dummy) ne n then $
            return, adsec_error.IDL_INPUT_TYPE
    endif

    ; all OK
    return, adsec_error.ok
end
