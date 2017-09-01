; $Id: init_cap.pro,v 1.4 2004/11/02 15:25:00 zanotti Exp $
;
;   2 Nov 2004: by D.Zanotti
;               adsec: in_cap_radius, out_cap_radius are changed in adsec_shell: in_cap_radius, out_cap_radius
;
;
function chi2, p

@adsec_common 
common chi2_xy, x, y
common chi2_act, act_list

xg=[adsec.act_coordinates(0,act_list)]
yg=[adsec.act_coordinates(1,act_list)]
xp=p(0)+p(2)*(cos(p(4))*xg-sin(p(4))*yg)
yp=p(1)+p(3)*(sin(p(4))*xg+cos(p(4))*yg)
return, total((x-xp)^2+(y-yp)^2)
end

pro init_cap, list, x_c ,y_c

@adsec_common 
common chi2_xy, x, y
common chi2_act, act_list
common cap_block, actuator_list, x_coord, y_coord, index, cap

if test_type(list,/INT,N_EL=n_el) then $
	message,"Actuator list must be integer"
if (n_el ne 3) or (min(list) lt 0) or (max(list) gt adsec.n_actuators-1) then $
	message,"Actuator list must contain three integers between 0 and "+strtrim(adsec.n_actuators-1,2)
if test_type(x_c,/INT,N_EL=n_el) then $
	message,"X coordinates must be integer"
if (n_el ne 3) or (min(x_c) lt 0) or (max(x_c) gt wyko.n_pix_x) then $
	message,"Actuator list must contain three integers between 0 and "+strtrim(wyko.n_pix_x-1,2)
 if test_type(y_c,/INT,N_EL=n_el) then $
	message,"Y coordinates must be integer"
if (n_el ne 3) or (min(y_c) lt 0) or (max(y_c) gt wyko.n_pix_y) then $
	message,"Actuator list must contain three integers between 0 and "+strtrim(wyko.n_pix_y-1,2)

act_list=list
y=y_c
x=x_c

;act_list=[0,4,8]
;y=[167,82,97]
;x=[200,231,128]

p=amoeba(1e-6,function_name='chi2',p0=[200d0,200d0,1d0,1d0,0d0],scale=1)
wyko.offset=p(0:1)														; center of the mirror location in interferometer coord. (IC)
wyko.scaling=p(2:3)														; scaling factor from mirror coordinates (MC), measured in
																		; mm to IC, measured in pixels.
wyko.ang_offset=p(4)													; angle of rotation about the center of the mirror
																		; to transform from MC to IC                                       IC)
wyko.transf=[[p(2)*[cos(p(4)),-sin(p(4))]],[p(3)*[sin(p(4)),cos(p(4))]]]	; transformation matrix to pass from mirror coordinates (mm)
																		; to wyko coordinates (pix), to obtain the total transformation
																		; wyko.offset must be added to the result

amc = adsec.act_coordinates			;actuator coordinates in MC
aic = rebin(wyko.offset,2,adsec.n_actuators)+transpose(wyko.transf ## transpose(amc))	;actuator coordinates in IC


wyko.act_coordinates(0,*)=aic[0,*]
wyko.act_coordinates(1,*)=aic[1,*]

t = wyko.ang_offset
xscale = wyko.scaling[0]
yscale = wyko.scaling[1]
xc = wyko.offset[0]
yc = wyko.offset[1]

x = rebin(indgen(wyko.n_pix_x),wyko.n_pix_x,wyko.n_pix_y)
y = rebin(transpose(indgen(wyko.n_pix_y)),wyko.n_pix_x,wyko.n_pix_y)

pix_coord = fltarr(2,wyko.n_pix_x,wyko.n_pix_y)                      ;pixel locations in (MC, mm)
pix_coord(0,*,*) = cos(t)*(x-xc)/xscale + sin(t)*(y-yc)/yscale
pix_coord(1,*,*) = -sin(t)*(x-xc)/xscale + cos(t)*(y-yc)/yscale

index=lonarr(adsec.n_actuators+1) & index(0)=0

rmin=adsec_shell.in_cap_radius			;inner and outer radii of the capacitive sensor armatures in mm
rmax=adsec_shell.out_cap_radius

for i=0,adsec.n_actuators-1 do begin
d2=(pix_coord(0,*,*)-amc(0,i))^2+(pix_coord(1,*,*)-amc(1,i))^2
if i eq 0 then begin
	 dummy=where(d2 le rmax^2 and d2 gt rmin^2,count)
endif else begin
	dummy=[dummy,where(d2 le rmax^2 and d2 gt rmin^2,count)]
endelse
if(count le 0) then stop
index(i+1)=index(i)+count
endfor

cap = lonarr(wyko.n_pix_x*wyko.n_pix_y)
cap[0] = dummy
wyko.index = index
wyko.cap = cap

offset = wyko.offset
scaling = wyko.scaling
ang_offset = wyko.ang_offset
transf = wyko.transf
opt_coordinates	= wyko.act_coordinates
save,FILE=filepath(root=adsec_path.data,"optical.sav"),offset,scaling,ang_offset,transf,opt_coordinates,index,cap

actuator_list=list
x_coord=x_c
y_coord=y_c

end
