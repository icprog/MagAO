pro test_linear, data,cl_act, RES=res
@adsec_common
if keyword_set(res) then restore, dialog_pickfile(), /ver

ndata=n_elements(data)
dcommand=transpose(data.dcommand)
position=transpose(data.position)
ctrl_current=transpose(data.ctrl_current)
current=transpose(data.current)

if total(cl_act) eq 0 then cl_act=adsec.act_w_cl

;index for positive and negative movement of the shell
idx_pos = 2*indgen((ndata-2)/2)+1
idx_neg = idx_pos+1

;absolute delta movement calcolus
diff_ctrl_current=ctrl_current[idx_pos,*]+ctrl_current[idx_neg,*]-2*rebin(ctrl_current[0,*], ((ndata-2)/2),adsec.n_actuators)
diff_current=current[idx_pos,*]+current[idx_neg,*]-2*rebin(current[0,*],((ndata-2)/2),adsec.n_actuators)
diff_position=position[idx_pos,*]+position[idx_neg,*]-2*rebin(position[0,*],((ndata-2)/2),adsec.n_actuators)
diff_dcommand=dcommand[idx_pos,*]-dcommand[idx_neg,*]
diff_ctrl_current = diff_ctrl_current[*, cl_act]
diff_current = diff_current[*, cl_act]
diff_position = diff_position[*, cl_act]
diff_dcommand = diff_dcommand[*, cl_act]

id = where(diff_dcommand gt 0)
dposition_pos = position[idx_pos,*] - rebin(position[0,*], n_elements(idx_pos), adsec.n_actuators)
dposition_neg = position[idx_neg,*] - rebin(position[0,*], n_elements(idx_pos), adsec.n_actuators)
dcommand_pos = dcommand[idx_pos,*]
dcommand_neg = dcommand[idx_neg,*]

dposition_pos = dposition_pos[*, cl_act]
dposition_neg = dposition_neg[*, cl_act]
dcommand_pos = dcommand_pos[*, cl_act]
dcommand_neg = dcommand_neg[*, cl_act]
linf_pos = rebin(abs(dposition_pos/dcommand_pos), 1, n_elements(cl_act))
linf_neg = rebin(abs(dposition_neg/dcommand_neg), 1, n_elements(cl_act))


window, /fr
image_show, abs(diff_ctrl_current[*,cl_act]), /SH, TITLE=" delta ctrl current", $
subtitle='"If the measures pattern is not random (machine error) or null, a no linearity is present."'

window, /fr 
image_show, abs((diff_current)[*,cl_act]), /SH, TITLE="delta total current", $
subtitle='"If the measures pattern is not random (machine error) or null, a no linearity is present."'

window, /fr 
image_show, abs(diff_position[*,cl_act]), /SH ,TITLE="delta position", $
subtitle='"If the measures pattern is not random (machine error) or null, a no linearity is present."'

window, /fr 
image_show, abs(diff_dcommand[*,cl_act]), /SH,TITLE="delta command", $
subtitle='"If the measures pattern is not random (machine error) or null, a no linearity is present."'


;delta_ctrl_current=ctrl_current[1:ndata-1]-rebin(ctrl_current[0,*], ((ndata-1)),48)
;delta_bias_current=bias_current[1:ndata-1,*]-rebin(bias_current[0,*], ((ndata-1)),48)
;delta_ff_current=ff_current[1:ndata-1,*]-rebin(ff_current[0,*], ((ndata-1)),48)
;delta_position=position[1:ndata-1,*]-rebin(position[0,*], ((ndata-1)),48)
;delta_command=command[1:ndata-1,*]-rebin(command[0,*], ((ndata-1)),48)

;window, 5 
;image_show, (delta_ctrl_current[0:89,cl_act]), /SH, TITLE="ctrl current"
;window, 6 
;image_show,(delta_bias_current[0:89,cl_act]), /SH, TITLE="bias current"
;window, 7 
;image_show, ((delta_ff_current+delta_ctrl_current+delta_bias_current)[0:89,cl_act]), /SH, TITLE="total current"
;window, 8 
;image_show, (delta_position[0:89,cl_act]), /SH ,TITLE="position"
;window, 9 
;image_show, (delta_command[0:89,cl_act]), /SH,TITLE="command"

end
