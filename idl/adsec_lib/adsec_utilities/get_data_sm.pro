function get_data_sm, NAME=name,  modes_vec, ff_cmd_vec, slopes_vec, frames

@adsec_common
sized = 8116
type = 1
if keyword_set(name) then filename = name else filename='MIRCTRL50:FRAME'

err=smread(filename, sized, type, data, /ulong)
if err ne adsec_error.ok then return, err

n_data=n_elements(data)
trash = 4

n_frame = data[n_data-trash]
res_data = data[0:(n_data-(trash+1))]
len =n_elements(res_data)
modes_data=res_data[len-adsec.n_actuators:*]
res_data =  res_data[0:(len-(1+adsec.n_actuators+trash))]
len =   n_elements(res_data)
ff_cmd_data = res_data[len-adsec.n_actuators:*]
res_data =  res_data[0:(len-(1+adsec.n_actuators+trash))]
len =   n_elements(res_data)
slopes_data = res_data[len-rtr.n_slope:*]
res_data =  res_data[0:(len-(1+rtr.n_slope))]
len =   n_elements(res_data)
if len ne 6408 then begin
    print, 'Errore nella decomposizione'
    return, err
endif

;modes_vec=float(modes_data,0,adsec.n_actuators)
;ff_cmd_vec=float(ff_cmd_data,0,adsec.n_actuators)
;slopes_vec=float(slopes_data,0,rtr.n_slope)
;frames = float(n_frame,0,1)
frames =n_frame
modes_vec=modes_data
ff_cmd_vec=ff_cmd_data
slopes_vec=slopes_data
return, adsec_error.ok
end
