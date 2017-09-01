;+
;   NAME:
;    INIT_ADSEC_GR
;
;   PURPOSE:
;   Definitions for displaying the actuator pattern and initialization of the structure "gr"
;
;   USAGE:
;    err = init_adsec_gr(filename)
;
;   INPUT:
;    filename: name of the configuration file with the dimensions of the position and 
;               current panels (pixels)(default file for LBT672). 
;
;   OUTPUT:
;    Error code.
;
;   KEYWORD:
;    None.
;  COMMON BLOCKS:
;   GR                  :  graphical common block will be filled.
;
;  PACKAGE:
;   LBT Adsec Libraries.
;
;  HISTORY
;   Created on 26 Oct 2004 by Daniela Zanotti (DZ).
;   01 Dec 2004: Marco Xompero (MX)
;     modified default configuration filename
;
;   NOTE:
;    None.
;-

function init_adsec_gr, filename

@adsec_common

if n_elements(filename) eq 0 then filename=filepath(Root=adsec_path.conf, "graph.txt")



if file_search(filename) ne filename then begin
    message,'The configuraton file "'+filename+'" does not exist.' , CONT = (sc.debug eq 0B)
    return, adsec_error.input_type
endif 

dim = read_ascii_structure(filename, DEBUG=0)
if n_elements(dim) eq 0 then begin
    message, 'The configuration file "'+filename+'" does not exist or has a wrong format.', CONT = (sc.debug eq 0B)
    return, adsec_error.input_type
endif

tr = dim.theta_tv/180.0*!PI
rot_mat = [[cos(tr), -sin(tr)], $
           [sin(tr),cos(tr)]]
if dim.x_reflect_tv then rot_mat[*,0] = -rot_mat[*,0]
                                                                                                                             
; scale on the graphic panel [pix/mm]
tv_scale = (min([dim.x_tv, dim.y_tv]-10))/(2*adsec_shell.out_radius)
                                                                                                                             
; # of pixels per sensing zone
npix_cap = tv_scale*2*adsec_shell.out_cap_radius
npix_cap = fix(npix_cap/2)*2+1 > 3 ; odd is better
; # of pixel of the inner diameter of the capsens
npix_in_cap = fix(tv_scale*2*adsec_shell.in_cap_radius)
                                                                                                                             
; pixmap mask (0B/1B) of the single sensing zone
capsens = shift(dist(npix_cap), npix_cap/2, npix_cap/2)
capsens = (capsens lt npix_cap/2) - (capsens lt npix_in_cap/2)

rot_act_coord = rot_mat ## transpose(adsec.act_coordinates);?geometrica elettronica
rot_act_coord = transpose(temporary(rot_act_coord))
act_coord = fix(round(tv_scale*rot_act_coord))
                                                                                                                             
act_coord = act_coord + rebin([dim.x_tv/2,dim.y_tv/2], 2, adsec.n_actuators, /SAMP)
num_coord = act_coord
num_coord[1,*] = num_coord[1,*]-!D.Y_CH_SIZE/4 ; correction for label verical centering
                                                                                                                             
tv_pos_type_list = ["Positions", "Commands", "Max-min position", "Position error", "Capsens out."]
tv_pos_unit_list = ['m', "um", "nm"]
tv_sens_unit_list = ['adcbit', "mV", "V"]
                                                                                                                             
tv_curr_type_list = ["Total Currents", "Bias currents", "Control currents", "Feed-foreward currents"]
tv_curr_unit_list = ['N', "mA", "mN"]
                                                                                                                             
extra_ct = 0.0 ; after color table optimization:
               ; top_color_value = max_value+extra_ct*(max_value-min_value)/2
               ; bottom_color_value = min_value-extra_ct*(max_value-min_value)/2
gr = $
  { $
    x_tv             :            dim.x_tv, $ ; x-dim of position and current panel [pix]
    y_tv             :            dim.y_tv, $ ; y-dim of position, scale and current panel [pix]
    x_scale_tv       :             100, $ ; x-dim of scale panel [pix]
    curr_cuts        :   [-1,1]*5000.0, $ ; color cuts in current panel
    pos_cuts         :[2.0^15,-10000.0], $ ; color cuts in position panel
    extra_ct         :        extra_ct, $ ; see above definition of extra_ct
    sub_pos_offset   :              0B, $ ; 1B: pos-offset is displayed. 0B pos is displayed
    sub_curr_offset  :              0B, $ ; 1B: curr-offset is displayed. 0B curr is displayed
    cl_on_color      :long([255B,0B,0B,0B],0), $ ; color of closed loop on (red)
    cl_off_color     :long([0B,255B,0B,0B],0), $ ; color of closed loop off (green)
    n_act_color      :   !D.TABLE_SIZE, $ ; number of colors for the actuators
    theta_tv         :        dim.theta_tv, $ ; angle of rotation of adsec_shell.act_coordinates pattern 4 displaing
    x_reflect_tv     :    dim.x_reflect_tv, $ ; reflection (1B) or not (0B) of the rotated pattern 4 displaing
    act_coord        :       act_coord, $ ; tv coordinates of act's [pix]
    tv_scale         :        tv_scale, $ ; pix/mm on the panels
    tv_pos_type_list :tv_pos_type_list, $ ; list of the data type to display in the position panel
    tv_curr_type_list:tv_curr_type_list, $ ; list of the data type to display in the position panel
    tv_pos_type_idx  :              0L, $ ; index of the currently active item of tv_pos_type_list
    tv_curr_type_idx :              0L, $ ; index of the currently active item of tv_curr_type_list
    tv_pos_unit_list :tv_pos_unit_list, $ ; list of position units to display in the position panel
    tv_sens_unit_list:tv_sens_unit_list, $ ; list of cap sensor output units to display in the position panel
    tv_curr_unit_list:tv_curr_unit_list, $ ; list of the data type to display in the position panel
    tv_pos_unit_idx  :              0L, $ ; index of the currently active item of tv_pos_type_list
    tv_curr_unit_idx :              0L, $ ; index of the currently active item of tv_curr_type_list
    tv_pos_smooth    :              0B, $ ; do I smooth the position pattern?
    tv_curr_smooth   :              0B, $ ; do I smooth the current pattern?
    num_coord        :       num_coord, $ ; tv coordinates of act num's [pix]
    num_type         :               0, $ ; 0=internal repr.,1=mechanic repr.,2=warren
    charsize         :        dim.charsize, $ ; normalized size of the chars used to label the acts
    position         :bytarr(dim.x_tv,dim.y_tv), $ ; bitmap of position panel
    current          :bytarr(dim.x_tv,dim.y_tv), $ ; bitmap of current panel
    npix_cap         :        npix_cap, $ ; dim of the capsens image
    capsens          :         capsens  $ ; image of a capsens
  }
;
; End of: definitions for displaying the actuator pattern
return, adsec_error.ok
end
