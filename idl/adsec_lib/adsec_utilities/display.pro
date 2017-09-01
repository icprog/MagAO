; $Id: display.pro,v 1.11 2009/04/10 13:56:21 labot Exp $

pro display, the_val, the_list, FLAG=flag, TITLE=title, NO_PLOT=show $
              , POSITION=position, SMOOTH=smooth, SHOWBAR=showbar $
              , NO_NUMBERING=no_numbering, OUT_VAL=out_val $
              , _EXTRA=plot_keywords, NO_BACKGROUND=no_background $
              , ROT=rot, REFL=refl, min_v=min_pos , max_v=max_pos $
              , MAP_SAMPLING=map_samp, BAR_TITLE=bar_title, NUM_TYPE=num_type, NUM_LIST=num_list $
              , NOERASE=noerase, YTICKF_BAR=ytickf_bar, XSTYLE=xstyle, YSTYLE=ystyle, YTITLE=ytitle , XTITLE=xtitle $
              , ACT_PSYM=act_psym, ACT_SYMSIZE=act_symsize, NO_SMOOTH_VAL=no_smooth_val $
              , NUM_CHARTHICK=num_charthick, NUM_CHARSIZE=num_charsize, OUT_EL=out $
              , IN_EL=in, COL_INVERT=col_invert, LOG=log, INV=inv, SPOT_MAGNIFICATION=spot_mag, XBAR=xbar $
              , PARAM_FILE=param_file, ADSEC_SAVE=adsec_save, ADSEC_SHELL_SAVE=adsec_shell_save, SC_SAVE=sc_save, GR_SAVE=gr_save, SECOND_LIST=second_list $
              , XX=xx, YY=yy
;+
; NAME:
;   DISPLAY
;
; PURPOSE:
;   Display an image based on the values sampled at the actuator locations (defined in the structure adsec).
;
; CATEGORY:
;   General graphics.
;
; CALLING SEQUENCE:
;       display, val, list, FLAG=flag, TITLE=title, NO_PLOT=show $
;              , POSITION=position, SMOOTH=smooth, SHOWBAR=showbar $
;              , NO_NUMBERING=no_numbering, OUT_VAL=out_val $
;              , _EXTRA=plot_keywords, NO_BACKGROUND=no_background $
;              , ROT=rot, REFL=refl, min_v=min_pos , max_v=max_pos $
;              , MAP_SAMPLING=map_samp, BAR_TITLE=bar_title, NUM_TYPE=num_type $
;              , NOERASE=noerase, YTICKF_BAR=ytickf_bar, XSTYLE=xstyle,YSTYLE=ystyle, YTITLE=ytitle $
;              , ACT_PSYM=act_psym, ACT_SYMSIZE=act_symsize, NO_SMOOTH_VAL=no_smooth_val $
;              , NUM_CHARTHICK=num_charthick, NUM_CHARSIZE=num_charsize, OUT_EL=out $
;              , IN_EL=in, COL_INVERT=col_invert, LOG=log, INV=inv
;
; INPUTS:
;   VAL:    Values assumed at the actuator locations defined by the input LIST.
;           The_val must have adsec.n_actuators elements or the same size of LIST.
;           In the first case the subset of the_val stated by LIST is considered.
;
;   LIST:   List of actuators used to sample the image.
;           If it is omitted LIST is set to adsec.true_act by default.
;
;
; KEYWORD PARAMETERS:
;
;   FLAG:   if not set shows a 2-d map of the mirror if set to 2 shows a shaeded surface
;
;   TITLE: sets the title of the plot
;
;   HIDEPLOT: If this keyword is set the plot is not shown (the image can be saved in the variable POSITION for future plotting)
;
;   POSITION: If set in the call equal to a named variable this variable returns the 2-d map
;
;   SMOOTH: Generates a smoothed version of the map.
;
;   SHOWBAR:    If set the color bar is shown
;
;   NO_NUMBERING: If set the actuator numbering is not shown
;
;   OUT_VAL: Allows to set the background color
;
;   _EXTRA: Allows to use the general keywords used with the plot routine to insert titles and other
;           features (see help on plot).
;
;   NO_BACKGROUND: Sets the background color to the minimum value
;
;
;   ROT: Allows to set the rotation of the actuator mapping (usually set to the values defined in the structure gr)
;
;   REFL: Allows to set the reflection along the x-axis of the actuator mapping (usually set to the values defined in the structure gr)
;
;   MIN_V: Sets the minimum value displaied
;
;   MAX_V: Sets the maximum value displaied
;
;   MAP_SAMPLING: Sets the number of pixels used in the map (it is set by default to 151 in the un-smoothed image and to 101 in the smoothed one)
;
;   SPOT_MAGNIFICATION: float scalar. 0 < spot_mag <= 1. Dimention of
;   spots in units of interactuator pitch. Default value is 0.25.
;
;   XBAR: set the initial point of the bar in x
;   axis. 0<xbar<1. Default 0.75.
;
;   NUM_LIST: set a index list of passed data which indicates the actuators that you want to have numberi displayed
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
;   GBZ several times.
;   Aug 2003: by A. Riccardi (AR).
;             More flexibility for the size of VAL has been introduced.
;             New default value for LIST (adsec.true_act)
;
;   2 Nov 2004: by D. Zanotti
;               adsec: in_radius, out_radius are changed in adsec_shell: in_radius, out_radius
;
;   20 May 2005: AR, SPOT_MAGNIFICATION keyword added
;
;   04 Aug 2005: by AR:
;                changed default output map size in case SMOOTH is not defined (from 151 to 255).
;                fixed bugs in actuator number and symbol display
;
;   28 April 2008 by AR, MX
;                 adsec_save keyword_added.
;
;   5 Nov 2008: by MX
;                 numlist keyword added. Set default xtitle and ytitle.
;
;-

@adsec_common
if n_elements(param_file) eq 0 then begin
    @adsec_common
endif else begin
    restore, param_file
endelse
if n_elements(adsec_save) eq 0 then adsec_save = adsec
if n_elements(sc_save) eq 0 then sc_save = sc
if n_elements(adsec_shell_save) eq 0 then adsec_shell_save = adsec_shell
if n_elements(gr_save) eq 0 then gr_save = gr
if n_elements(XTITLE) eq 0 then xtitle='[mm]'
if n_elements(YTITLE) eq 0 then ytitle='[mm]'

on_error, 2
val=reform(the_val)*1.0
if (size(val))(0) ne 1 then message,'Wrong input format'
if n_params() ne 1 then begin
    list = reform(the_list)
    nlist=n_elements(list)
	case n_elements(val) of
		adsec_save.n_actuators: begin
			val=val[list]
		end

		n_elements(list): begin
		end

		else: message, 'Wrong list size'
	endcase
endif else begin
	case n_elements(val) of
		adsec_save.n_actuators: begin
			val=val[adsec_save.true_act]
		end

		n_elements(adsec_save.true_act): begin
		end

		else: message, 'Wrong input size'
	endcase

	list=adsec_save.true_act
    nlist=n_elements(list)
    if n_elements(num_list) gt 0 then begin
        tmp = fltarr(adsec_save.n_actuators)
        tmp[num_list] = 1
        tmp = tmp[adsec_save.true_act]
        num_list = where(tmp)
    endif
endelse

ima=val

if n_elements(bar_title) eq 0 then bar_title=''
if n_elements(num_type) eq 0 then num_type=gr_save.num_type
if n_elements(noerase) eq 0 then noerase=0B
if n_elements(flag) eq 0 then flag=1
if n_elements(title) eq 0 then title=''
if n_elements(rot) eq 0 then rot = gr_save.theta_tv
if n_elements(refl) eq 0 then refl = gr_save.x_reflect_tv
;
; actuator geometry definition
;
if keyword_set(smooth) then begin
    x=adsec_save.act_coordinates(0,*)*(1.+randomn(seed,adsec_save.n_actuators)*1e-4)
    y=adsec_save.act_coordinates(1,*)*(1.+randomn(seed,adsec_save.n_actuators)*1e-4)
endif else begin
    x=adsec_save.act_coordinates(0,*)
    y=adsec_save.act_coordinates(1,*)
endelse

if n_elements(rot) ne 0 then begin
    t=rot/180.0*!pi
    matrot=transpose([[cos(t),sin(t)],[-sin(t),cos(t)]])
    dummy=matrot ## transpose([x,y])
    x=dummy[*,0]
    y=dummy[*,1]
endif

if keyword_set(refl) then x=-x

if n_elements(num_charthick) eq 0 then charthick=1.0
if n_elements(num_charsize) eq 0 then charsize=1.0
;the random term was added to correct a problem with the trigrid routine

;
; interpolation geometry definition
;
lim=fltarr(4)
gs1=fltarr(2)
if n_elements(map_samp) eq 0 then $
    if keyword_set(smooth) then map_samp=101 else map_samp=255;151
np=map_samp
lim(0)=-adsec_shell_save.out_radius
lim(1)=-adsec_shell_save.out_radius
lim(2)=adsec_shell_save.out_radius
lim(3)=adsec_shell_save.out_radius
gs1(0)=2*adsec_shell_save.out_radius/(np-1)
gs1(1)=2*adsec_shell_save.out_radius/(np-1)
x1=2*adsec_shell_save.out_radius*(findgen(np)/(np-1)-.5)*float(np-1)/np
y1=2*adsec_shell_save.out_radius*(findgen(np)/(np-1)-.5)*float(np-1)/np
;dis=shift(dist(np),(np-1)/2,(np-1)/2)/(np-1)*2*adsec_shell_save.out_radius
dis=shift(dist(np),(np-1)/2,(np-1)/2)/np*2*adsec_shell_save.out_radius
out=where((dis gt adsec_shell_save.out_radius) or (dis lt adsec_shell_save.in_radius))
in =where((dis le adsec_shell_save.out_radius) and (dis ge adsec_shell_save.in_radius))
;dummy=complement(out,lindgen(long(map_samp)^2),in,count)
if keyword_set(smooth) then begin
;   triangulate,x(list),y(list),tr,b
;   position=trigrid(x(list),y(list),ima,tr,gs1,lim,EXTRA=b)
    position=min_curve_surf(ima,x(list),y(list),GS=gs1,BOUNDS=lim,/TPS)
    if n_elements(min_pos) eq 0 then begin
    	min_pos=min(position[in])
    	no_lt=1B
    endif else no_lt=0B

    if n_elements(max_pos) eq 0 then begin
    	max_pos=max(position[in])
    	no_gt=1B
    endif else no_gt=0B

endif else begin
    ;spot_radius = 0.25*sqrt(!PI*(adsec_shell_save.out_radius^2-adsec_shell_save.in_radius^2)/adsec_save.n_actuators)
    ;position=fltarr(np,np)
    ;xcoord=rebin(x1,np,np,/s)
    ;ycoord=rotate(xcoord,1)
    ;for i=0,nlist-1 do begin
    ;    dummy=where(((xcoord-x(list(i)))^2+(ycoord-y(list(i)))^2) lt spot_radius^2)
    ;    position(dummy) =val(i)
    ;    if i eq 0 then in = dummy else in = [in, dummy]
    ;endfor
    if n_elements(spot_mag) eq 0 then spot_mag=0.5
    spot_radius = 0.5*spot_mag*sqrt(!PI*(adsec_shell_save.out_radius^2-adsec_shell_save.in_radius^2)/adsec_save.n_actuators)/(2*adsec_shell_save.out_radius)*np
    spot_diam = (round(2*spot_radius)/2*2+1) > 3
    spot_radius = spot_diam/2
    spot_map = fltarr(spot_diam,spot_diam)
    spot_map_idx = where(shift(dist(spot_diam),spot_radius,spot_radius) le spot_radius)
    spot_map(where(shift(dist(spot_diam),spot_radius,spot_radius) gt spot_radius))=0.0

    position=fltarr(np,np)
    if n_elements(no_smooth_val) ne 0 then begin
    	position[*,*]=no_smooth_val
    	spot_map(where(shift(dist(spot_diam),spot_radius,spot_radius) gt spot_radius))=no_smooth_val
    endif else begin
    	position[*,*]=min(val)
    	spot_map(where(shift(dist(spot_diam),spot_radius,spot_radius) gt spot_radius))=min(val)
    endelse
    xx=((round((x+adsec_shell_save.out_radius)/(2*adsec_shell_save.out_radius)*np - spot_radius)) > 0) < (np-spot_diam)
    yy=((round((y+adsec_shell_save.out_radius)/(2*adsec_shell_save.out_radius)*np - spot_radius)) > 0) < (np-spot_diam)
    for i=0,nlist-1 do begin
    	spot_map(spot_map_idx)=val[i]
        position[xx[list[i]],yy[list[i]]] = spot_map
        ;dummy=where(((xcoord-x(list(i)))^2+(ycoord-y(list(i)))^2) lt spot_radius^2)
        ;position(dummy) =val(i)
        ;if i eq 0 then in = dummy else in = [in, dummy]
    endfor
    if n_elements(min_pos) eq 0 then begin
    	min_pos=min(val)
    	no_lt=1B
    endif else no_lt=0B

    if n_elements(max_pos) eq 0 then begin
    	max_pos=max(val)
    	no_gt=1B
    endif else no_gt=0B
 ;   if n_elements(min_pos) eq 0 then min_pos=min(val)
 ;   if n_elements(max_pos) eq 0 then max_pos=max(val)
endelse

if n_elements(out_val) ne 0 then begin
    position(out)=out_val
endif else begin
    ave=mean(val)
    position(out)=ave
endelse



if keyword_set(NO_BACKGROUND) then position(out)=max_pos

if keyword_set(show) then goto,fine_dis

if keyword_set(COL_INVERT) then begin
	if total(replicate(!d.name,3) eq ['mac','x','win']) ge 1 then device,decompose=1
	background_old=!p.background
	color_old=!p.color
	!p.background=color_old
	!p.color=background_old
endif

if (flag eq 2) then begin
    shade_surf,min_v=min_pos,max_v=max_pos,position
endif

if (flag eq 1) then begin

    ;min_pos = min(val)
    ;max_pos = max(val)


    image_show, position, /as, sh=keyword_set(showbar), xax=x1, yax=y1 $
            , min_v=min_pos , max_v=max_pos, posiz=posiz, title=title $
            ,_EXTRA=plot_keywords, dig=2, BAR_TIT=bar_title, NOERASE=noerase $
            , NO_LT=no_lt, NO_GT=no_gt $
            ,YTICKF_BAR=ytickf_bar,XSTYLE=xstyle,YSTYLE=ystyle,YTITLE=ytitle $
            , LOG=log, INV=inv, XBAR=xbar, XTITLE=xtitle

    if not keyword_set(no_numbering) then begin
        top = !D.table_size
        if keyword_set(log) then begin
	        except=!except
	        ret=check_math(/print)
	        !except=0
	        if min(val) le 0 then message,'Values <=0',/cont
	        if keyword_set(inv) then begin
        		col=(bytscl(-alog10(val), MIN=min([-alog10(min_pos),-alog10(max_pos)]), MAX=max([-alog10(min_pos),-alog10(max_pos)]), TOP=top)+byte(top/2))<top
        	endif else begin
        		col=(bytscl(alog10(val), MIN=alog10(min_pos), MAX=alog10(max_pos), TOP=top)+byte(top/2))<top
        	endelse
			ret=check_math()
	        !except=except
        endif else begin
        	if keyword_set(inv) then begin
        		col=(bytscl(-val, MIN=min([-max_pos,-min_pos]), MAX=max([-min_pos,-max_pos]), TOP=top)+byte(top/2))<top
        	endif else begin

        		col=(bytscl(val, MIN=min_pos, MAX=max_pos, TOP=top)+byte(top/2))<top
        	endelse
        endelse

        if float(!version.release) lt 5.3 then begin
            can_use_colormap = 0
        endif else begin
            dev_name = !D.name
            if dev_name eq 'WIN' or dev_name eq 'X' or dev_name eq 'MAC' then begin
                can_use_colormap = colormap_applicable()
            endif else begin
                can_use_colormap = 1
            endelse
        endelse

        if not can_use_colormap then begin
            TVLCT, R, G, B, /GET
            col = r[col]+256L*g[col]+256L^2*b[col]
        endif

        xp=(x-lim(0))/(lim(2)-lim(0))*(posiz(2)-posiz(0))+posiz(0)
        yp=(y-lim(1))/(lim(3)-lim(1))*(posiz(3)-posiz(1))+posiz(1)

		if n_elements(ACT_PSYM) ne 0 then begin
	        for i=0,nlist-1 do begin
	            plots,xp(list[i]),yp(list[i]) $
	                , color=col(i), /device, PSYM=act_psym, SYMSIZE=act_symsize
	        endfor
	    endif else begin
	        if num_type ge 0 then begin
                    if n_elements(num_list) gt 0 then begin
                        for i=0,n_elements(num_list)-1 do begin
;                            if n_elements(list) eq n_elements(adsec_save.true_act) then begin
;                                tmp = fltarr()
;                                tmp[num_list] = 1
;                                num_list = where(tmp[list])
;                            endif
                            xyouts,xp(list[num_list[i]]),yp(list[num_list[i]]),sc_save.act_name[list[num_list[i]],num_type] $ ;strtrim(string(list[i]),2) $
                                   , color=col(num_list[i]),alignment=0.5,/device,CHARTHICK=num_charthick,CHARSIZE=num_charsize
                        endfor
                        for i=0,n_elements(second_list)-1 do begin
                            xyouts,xp(list[second_list[i]]),yp(list[second_list[i]]),sc_save.act_name[list[second_list[i]],num_type] $ 
                                   , color='00ff00'xl,alignment=0.5,/device,CHARTHICK=num_charthick,CHARSIZE=num_charsize
                        endfor
                    endif else begin
                        for i=0,nlist-1 do begin
                            xyouts,xp(list[i]),yp(list[i]),sc_save.act_name[list[i],num_type] $ ;strtrim(string(list[i]),2) $
                                   , color=col(i),alignment=0.5,/device,CHARTHICK=num_charthick,CHARSIZE=num_charsize
                        endfor
                    endelse

	        endif
	    endelse

    endif
endif

if keyword_set(COL_INVERT) then begin
	!p.background=background_old
	!p.color=color_old
endif

fine_dis:
end
