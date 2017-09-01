; $Id: ads_movie.pro,v 1.4 2003/11/18 15:57:52 riccardi Exp $

pro ads_movie, time_hist, time_vec, nin=nin, nfin=nfin, ZOOM_FACTOR=zoom $
       , framedec=framedec, subtract=sub, time_format=time_format, NO_DIFF=no_diff $
       , smooth=smooth, mov=mov, max_frames=max_frames, max_v=max_v, min_v=min_v $
       , ACT2SHOW=act2show
;+
; NAME:
;   ADS_MOVIE
;
; PURPOSE:
;   Display a movie of a certain time history TIME_HIST of the values sampled at the actuator locations (defined in the structure adsec).
;
; CATEGORY:
;   General graphics.
;
; CALLING SEQUENCE:
;	 ads_movie, time_hist, time_vec, NIN=nin, NFIN=nfin, ZOOM_FACTOR=zoom $
;      , FRAMEDEC=framedec, SUBTRACT=sub, TIME_FORMAT=time_format, NO_DIFF=no_diff $
;      , SMOOTH=smooth, MOV=mov, MAX_FRAMES=max_frames, MAX_V=max_v, MIN_V=min_v $
;      , ACT2SHOW=act2show
;
; INPUTS:
;   TIME_HIST:  The time history to use to make the movie (must be ntimes x n_actuators)
;
;	TIME_VEC:	Optional vector of time
;
; KEYWORD PARAMETERS:
;
;	NIN: 	initial frame to be used (in frame number or time depending on the TIME_FORMAT keyword)
;
;	NFIN: 	final frame to be used
;
;	ZOOM_FACTOR: zoom in pixels shown per pixel in input (a value of 1 will not change the image size)
;
;   FRAMEDEC: Decimation used to display the frames
;
;   SUBTRACT: If this keyword is set the average piston and tip-tilt are removed
;
;   TIME_FORMAT: If set TIME_VEC must be set and NIN and NFIN are given in time.
;
;	NO_DIFF: If set the initial frame is not subtracted
;
;   SMOOTH: Generates a smoothed version of the movie.
;
;	MOV: If set to a named variable returns the set of frames used for the movie (useful for later calls to ANIMATION)
;
;	MAX_FRAMES: Sets the maximum number of frames used in the movie
;
;	MIN_V: Sets the minimum value displaied
;
;	MAX_V: Sets the maximum value displaied
;
;	ACT2SHOW: List of actuators used to make the maps for the movie (input to DISPLAY)
;
; OUTPUTS:
;   No explicit outputs.
;
; COMMON BLOCKS:
;   adsec, gr.
;
; SIDE EFFECTS:
;   The currently selected display is affected.
;
; RESTRICTIONS:
;   None.
;
; PROCEDURE:
;
;
; MODIFICATION HISTORY:
;
;	GBZ several times.
;-
@adsec_common

if test_type(time_hist, /REAL, DIM=dim) then $
    message, "time_hist must be real."
if dim[0] ne 2 then $
    message, "time_hist must have 2 dimensions."
if dim[2] ne adsec.n_actuators then $
    message, "time_hist 2nd dimension must be "+strtrim(adsec.n_actuators, 2)+"."
n_frames = dim[1]

if n_params() ge 2 then begin
    if test_type(time_vec, /REAL, DIM=dim) then $
        message, "time_vec must be real."
    if dim[0] ne 1 then $
        message, "time_vec must have 2 dimensions."
    if dim[1] ne n_frames then $
        message, "time_vec length is not compatible with time_hist."
endif else begin
    if keyword_set(time_format) then $
        message, "The keyword TIME_FORMAT cannot be set without time_vec input."
    time_vec = indgen(n_frames)
endelse

if n_elements(act2show) eq 0 then act2show=indgen(adsec.n_actuators)

min_time = min(time_vec, max=max_time)

if n_elements(nin) ne 0 then begin
    if test_type(nin, /REAL, N_EL=n_el) then $
        message, "nin must be real."
    if n_el ne 1 then $
        message, "nin must be scalar."
    nin = nin[0]
    if keyword_set(time_format) then begin
        if nin lt min_time then $
            message, "Not valid value for nin"
    endif else begin
        if nin lt 0 then $
            message, "Not valid value for nin"
    endelse
endif else begin
    if keyword_set(time_format) then nin=min_time else nin=0
endelse

if keyword_set(time_format) then begin
    dummy = min(abs(nin-time_vec), idx)
    frame_in = idx
endif else frame_in = nin

if n_elements(nfin) ne 0 then begin
    if test_type(nfin, /REAL, N_EL=n_el) then $
        message, "nfin must be real."
    if n_el ne 1 then $
        message, "nfin must be scalar."
    nfin = nfin[0]
    if keyword_set(time_format) then begin
        if nfin gt max_time then $
            message, "Not valid value for nfin"
    endif else begin
        if nfin gt n_frames-1 then $
            message, "Not valid value for nfin"
    endelse
endif else begin
    if keyword_set(time_format) then nfin=max_time else nfin=n_frames-1
endelse

if nfin le nin then $
    message, "nfin must be greater then nin."

if keyword_set(time_format) then begin
    dummy = min(abs(nfin-time_vec), idx)
    frame_fin = idx
endif else frame_fin = nfin

if n_elements(zoom) ne 0 then begin
    if test_type(zoom, /NOFLOAT, N_EL=n_el) then $
        message, "zoom must be integer."
    if n_el ne 1 then $
        message, "zoom must be scalar."
    zoom = zoom[0]
    if zoom lt 1 then $
        message, "Zoom must be greater or equal to 1."
endif else zoom = 1

if n_elements(framedec) ne 0 then begin
    if test_type(framedec, /NOFLOAT, N_EL=n_el) then $
        message, "framedec must be integer."
    if n_el ne 1 then $
        message, "framedec must be scalar."
    framedec = framedec[0]
    if framedec lt 0 then $
        message, "framedec must be greater or equal to 0."
endif else framedec = 0

if n_elements(max_frames) ne 0 then begin
    if test_type(max_frames, /NOFLOAT, N_EL=n_el) then $
        message, "max_frames must be integer."
    if n_el ne 1 then $
        message, "max_frames must be scalar."
    max_frames = max_frames[0]
    if max_frames lt 1 then $
        message, "max_frames must be greater or equal to 1."
endif else max_frames = n_frames


n_used_frames = ((frame_fin-frame_in+1)/(framedec+1)) < max_frames
used_dec = fix((frame_fin-frame_in+1)/n_used_frames)-1
frame_fin = ((frame_fin-frame_in)/(used_dec+1))*(used_dec+1)+frame_in

if not keyword_set(no_diff) then time_hist=time_hist-rebin(time_hist[0,*],n_frames,adsec.n_actuators)

if keyword_set(sub)  then begin
    x2=total(adsec.act_coordinates(0,*)^2)
    y2=total(adsec.act_coordinates(1,*)^2)
    n=adsec.n_actuators
    a=total(rebin(adsec.act_coordinates(0,*),nt,n)*time_hist)/x2/nt
    b=total(rebin(adsec.act_coordinates(1,*),nt,n)*time_hist)/y2/nt
    c=total(time_hist)/n/nt
    offset=a*adsec.act_coordinates(0,*)+b*adsec.act_coordinates(1,*)+c
    time_hist=time_hist-rebin(offset,nt,n)
endif

display,time_hist(frame_in,act2show),act2show,position=position,showplot=-1, smooth=smooth,/no_n,/hide
sz=size(position)
mov=fltarr(sz(1),sz(2),n_used_frames)
mov(*,*,0)=position

j=0
for i=frame_in,frame_fin-(used_dec+1),used_dec+1 do begin
    display,time_hist(i,act2show),act2show,position=position, showplot=-1, smooth=smooth,/no_n,/hide
    mov(*,*,j)=position
    j=j+1
endfor

animation,mov,pixels=zoom,mini=min_v,maxi=max_v

end
