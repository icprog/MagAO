; $Id: disp_mess.pro,v 1.5 2009/12/21 10:55:08 marco Exp $

pro disp_mess, messag, APPEND=append, NOXADSEC=noxadsec

	@xadsec_common

    noxadsec =  xregistered('xadsec') eq 0
    if noxadsec then begin
		log_print, messag
        return
    endif

	if (n_elements(ids) eq 0) or keyword_set(noxadsec) then begin
		mf_id=0L
	endif else begin
		mf_id=ids.message_field_id
	endelse
	widget_control, mf_id, BAD_ID=bad_id
	if mf_id eq bad_id then begin
		; xadsec is not active. The messages are printed in the log window
		log_print, messag
	endif else begin
		; xadsec is active. The messages are printed in the xadsec message box
		ttl = widget_info(mf_id, /TEXT_TOP_LINE)
		widget_control, mf_id, SET_VALUE=messag, $
					    APPEND=keyword_set(append)
		if n_elements(messag) eq 1 then log_print, messag
;		widget_control, mf_id, SET_TEXT_TOP_LINE=ttl+n_elements(message)
	endelse
end
