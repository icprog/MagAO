; $Id: remove_low_order.pro,v 1.8 2007/05/17 15:36:06 labot Exp $
;+
;   NAME:
;    REMOVE_LOW_ORDER
;
;   PURPOSE:
;    Remove low order modes from shell in closed loop.
;
;   USAGE:
;    err=remove_low_order([ff_modes2remove], XADSEC=xadsec, VERBOSE=verbose, NO_TILT=no_tilt)
;
;   INPUT:
;    ff_modes2remove:       mode list to remove. Default are piston, tip and tilt.
;
;   KEYWORD:
;    XADSEC: update the xadsec panels.
;    VERBOSE: print some useful information.
;    NO_TILT: it doesn't remove tip, tilt and piston.
;
; HISTORY
;   28 May 2004, A. Riccardi (AR)
;    Modifications to match LBT formats
;   30 Jul 2004, D.Zanotti(DZ)
;    Modifications to data lenght units
;   08 Feb 2005, M.Xompero (MX)
;    Default low order modes removed are now piston tip and tilt.
;    Fixed casting problems with data. Help written.
;   Feb 2005, MX & AR
;    Corrected the low order computing. 
;-
function remove_low_order, ff_modes2remove, XADSEC=xadsec, VERBOSE=verbose, NO_TILT=NO_TILT
@adsec_common
if n_elements(ff_modes2remove) eq 0 then  ff_modes2remove = [0,1,2]

one=replicate(1.0,1,adsec.n_actuators)
if keyword_set(no_tilt) then begin
       x=coord[0,*]
	y=coord[1,*]
	ZZ = [one,x,y]       
endif else begin
       ZZ = one
endelse
ZZ[*,adsec.act_wo_cl] = 0

VVT = adsec.ff_p_svec[0:2,*] ## transpose(adsec.ff_p_svec[0:2,*])
VFF = adsec.ff_p_svec[ff_modes2remove,*]

err = update_status(1024)
if err ne adsec_error.ok then return,err

z = sys_status.position
c0= sys_status.command
d_comm = (VFF##transpose(VFF)-VVT##ZZ##invert(transpose(ZZ)##VVT##ZZ)##transpose(ZZ)##VVT)##z
p_mode = transpose(adsec.ff_p_svec) ## d_comm

if keyword_set(verbose) then begin
    print, "... removing some low-order modes..."
    print, p_mode[ff_modes2remove]
endif

time_ps=time_preshaper(ACT_LIST_TIME=adsec.act_w_cl)*adsec.time_secure_factor
;;
;; the low-mode correction is performed without the FF!!
;;
err = set_position_vector(c0-float(d_comm))
if err ne adsec_error.ok then message, "ERROR!"

err = update_command()
if err ne adsec_error.ok then message, "ERROR!"

wait, time_ps

if keyword_set(xadsec) then begin
    update_panels, /OPT
endif else begin
    err = update_status(1024)
    if err ne adsec_error.ok then return,err
endelse

if keyword_set(verbose) then begin

    print, "... done!"

    z = sys_status.position
    d_comm = (VFF##transpose(VFF)-VVT##ZZ##invert(transpose(ZZ)##VVT##ZZ)##transpose(ZZ)##VVT)##z
    p_mode = transpose(adsec.ff_p_svec) ## d_comm

    print, p_mode[ff_modes2remove]

    print, minmax(c0-sys_status.command)
endif

return, adsec_error.ok
end
