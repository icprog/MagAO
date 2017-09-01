;+
;   NAME:
;    INIT_INTERF_PARAM
;
;   PURPOSE:
;    Load the interferometer parameters.
;
;   USAGE:
;    err = init_interf_param(filename, OPTICAL_FILENAME = optical_filename)
;
;   INPUT:
;    filename: name of the configuration file(default: "params_wyko_RTI4100.txt") 
;               with the data of the interferometer and the communication type (default socket cmunication).
;
;   OUTPUT:
;    Error code.
;
;  COMMON BLOCKS:
;   wyko          :wyko common block was filled with the interferometer parameters.
;
;   KEYWORD:
;    OPTICAL_FILENAME:  file containing the optical calibration constants
;
;   HISTORY
;    Created on 25 Oct 2004 by Daniela Zanotti (DZ).
;                   
;
;   NOTE:
;    None.
;-
                                                                                                                             
function init_interf_param, filename, OPTICAL_FILENAME = optical_filename
                                                                                                                             
@adsec_common
                                                                                                                             
if n_elements(filename) eq 0 then filename="params_wyko_RTI4100.txt"
                                                                                                                             
filename=filepath(Root=adsec_path.conf, filename)
                                                                                                                             
if file_search(filename) ne filename then begin
    message,'The configuraton file "'+filename+'" does not exist.', /INFO
    return, adsec_error.input_type
endif

interf = read_ascii_structure(filename, DEBUG=0)
if n_elements(interf) eq 0 then begin
    message, 'The configuration file "'+filename+'" does not exist or has a wrong format.',/INFO
endif else begin


interf_type = interf.type
interf_model = interf.model

interf_n_pix_x  = interf.n_pix_x
interf_n_pix_y = interf.n_pix_y 
interf_aspect = interf.aspect     ; aspect ratio of the interferometer pixels (y/x)
interf_wl = interf.wl  
interf_com_type = interf.com_type
interf_com_port = { $
    ip_address: interf.ip_address, $
    port:       interf.port}

n_acts = adsec.n_actuators

offset              = fltarr(2)         ;; (0,0) (CCD) pixel coordinates in mm
scaling             = fltarr(2)         ;; x and y scaling factors from pixel to mm
ang_offset          = 0.0               ;; angular offset of mm coord. sys. from pix coord. sys.
transf              = fltarr(2,2)       ;; rotation matrix to transform from mm to pix coord.
opt_coordinates     = fltarr(2,n_acts)  ;; actuators coordinates in pixels
index               = lonarr(n_acts+1)  ;; index of the starting positions of the capacitor locations in the cap array
cap                 = lonarr(interf_n_pix_x*interf_n_pix_y) ;; list (indexed by index) of the wyko pixels within the capacitor plates
opt_flag            = 0B                ;; flag for the initialization of wyko parameters (0B not initialized)
                                                                                                                             
if keyword_set(optical_filename) then begin

check = file_search(optical_filename)
if check[0] eq "" then begin
    message, 'The file '+optical_filename+ $
      " containing the optical data doesn't exist", /INFO
    print, "The variables offset, scaling, ang_offset, rot, act_coordinates have not been initialized"
endif else begin
                                                                                                                             
    restore, optical_filename
                                                                                                                             
    if test_type(offset, /REAL, DIM=dim, N_El=n_el) then $
      message, "offset vector must be real"
    if dim[0] ne 1 then $
      message, "Offset must be a vector"
    if n_el ne 2 then $
      message, "Offset must have two values"
                                                                                                                             
    if test_type(scaling, /REAL, DIM=dim, N_El=n_el) then $
      message, "scaling vector must be real"
    if dim[0] ne 1 then $
      message, "scaling must be a vector"
    if n_el ne 2 then $
      message, "scaling must have two values"
                                                                                                                             
    if test_type(ang_offset, /REAL, N_El=n_el) then $
         message, "ang offset must be real"
    if n_el ne 1 then $
        message, "ang offset must have one value"
                                                                                                                             
    if test_type(transf, /REAL, DIM=dim) then $
      message, "transformation matrix vector must be real"
    if total(dim ne [2,2,2]) ne 0 then $
      message, "transformation must be 2x2 matrix"
                                                                                                                             
    if test_type(index, /LONG, DIM=dim) then $
      message, "index must be long"
    if total(dim ne [1,n_acts+1]) ne 0 then $
      message, "index must be a vector"
                                                                                                                             
   if test_type(cap, /LONG, DIM=dim) then $
      message, "cap must be long"
    if total(dim ne [1,interf_n_pix_x*interf_n_pix_y]) ne 0 then $
      message, "cap must be a vector"
                                                                                                                             
    if test_type(opt_coordinates, /REAL, DIM=dim) then $
      message, "act_coordinates matrix vector must be real"
    if total(dim ne [2,2,n_acts]) ne 0 then $
      message, "act_coordinates must be 2xn_acts matrix"
                                                                                                                             
    opt_flag = 1B
endelse
endif else begin
    print, "The variables offset, scaling, ang_offset, rot, act_coordinates have not been initialized"
endelse
;===========================================================================
; Definition of the interferometer parameters
;
wyko = $
  { $
    type:       interf_type, $    ; interferometer Type
    model:      interf_model, $ ; interferometer Model
    com_type:   interf_com_type, $ ; communication type to the interferometer (socket or serial)
    com_port:   interf_com_port, $ ; connection data used for the serial/socket communication with wyko
    n_pix_x:    interf_n_pix_x, $      ; x-size of wyko CCD in pixels
    n_pix_y:    interf_n_pix_y, $      ; y-size of wyko CCD in pixels
    aspect:     interf_aspect, $       ; apsect ratio of the wyko pixels (y/x)
    wavelength: interf_wl, $   ; laser wavelength in nm
    offset:     offset, $       ; center of the mirror in interferometer coord. (IC, pix)
    scaling:    scaling, $      ; scaling factor from mirror coordinates (MC, mm) to (IC, pix)
    ang_offset: ang_offset, $   ; angle of rotation about the center of the mirror
                                ; to transform from MC to IC
    transf:     transf, $       ; transformation matrix to pass from (MC, mm)
                                ; to (IC, pix). To obtain the total transformation
                                ; wyko.offset must be added to the result
                                ; actuators coordinates in pixels
    act_coordinates: opt_coordinates, $
    index:      index, $
    cap:        cap, $
    flag:       opt_flag $  ;flag to check if the optical calibration paramters have been loaded
  }
;
; END OF: Definition of the interferometer parameters
;===========================================================================
endelse

return, adsec_error.ok
end
