pro test_linear_v2, data,cl_act, bias_current, RES=res, no_number=no_number
@adsec_common
if keyword_set(res) then restore, dialog_pickfile(), /ver

ndata=n_elements(data)
n_cl_act = n_elements(adsec.act_w_cl)
nmeas = (ndata-2)/n_cl_act/2.
position=transpose(data.position)
dcommand = transpose(data.dcommand)
ff_current=transpose(data.ff_current)
ctrl_current = transpose(data.current) - ff_current - rebin(transpose(bias_current), ndata, adsec.n_actuators)
if total(cl_act) eq 0 then cl_act=adsec.act_w_cl

;index for positive and negative movement of the shell
idx_pos = 2*indgen(ndata-2)+1
idx_neg = idx_pos+1

;absolute delta movement calcolus
diff_ctrl_current = abs(ctrl_current[idx_pos,*])-abs(ctrl_current[idx_neg,*]);-2*rebin(ctrl_current[0,*], nmeas*n_cl_act,adsec.n_actuators)
diff_ff_current   = ff_current[idx_pos,*]+ff_current[idx_neg,*]-2*rebin(ff_current[0,*], nmeas*n_cl_act,adsec.n_actuators)
diff_position     = abs(position[idx_pos,*])-abs(position[idx_neg,*])

dctrl = fltarr(n_elements(cl_act))
tmp =  diff_ctrl_current[*,adsec.act_w_cl]
for i=0, n_elements(cl_act)-1 do begin
    dummy = tmp[*,i]
    dctrl[i] = stddev(dummy[indgen(nmeas)*n_cl_act+1+i])
endfor

dpos = fltarr(n_elements(cl_act))
tmp =  diff_position[*,adsec.act_w_cl]
for i=0, n_elements(cl_act)-1 do begin
    dummy = tmp[*,i]
    dpos[i] = stddev(dummy[indgen(nmeas)*n_cl_act+1+i])
endfor

print, "Test DeltaCommand"
result = total(double(dcommand))
print, "Total value: "+ strtrim(result, 2) + " , espected: 0d0"
if result eq 0d0 then print, "Passed." else print, "***********  NOT PASSED!!!"

print, "Test Ctrl Current"
window, /free, xs=1024, ys=768
display, dctrl, adsec.act_w_cl, /SH, TITLE="CtrlCurrent pos-neg step difference", no_n = no_number
print, "Done on window: ", !D.WINDOW

print, "Test Posizione"
window, /free ,  xs=1024, ys=768
display, dpos, adsec.act_w_cl, /SH,TITLE="DeltaPosition pos-neg step difference", no_n = no_number
print, "Done on window: ", !D.WINDOW


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
