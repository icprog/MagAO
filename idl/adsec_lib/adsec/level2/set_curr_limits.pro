;$Id: set_curr_limits.pro,v 1.3 2009/12/21 10:55:08 marco Exp $$
;+
; NAME:
;   SET_CURR_LIMITS
;
; PURPOSE:
;   Set up the hardware threshold on maximum current applied by the Adaptive Secondary coils.
;
; USAGE:
;   err = set_curr_limits(crateLimit, crateTotal)
;
; INPUT:
;   crateLimit:
;   crateTotal:
;
; OUTPUT:
;   err:
;
; KEYWORDS:
;   PUSHTOTAL:  to set up the maximum total current negative (push out of RB)
;   PULLTOTAL:  to set up the maximum total current positive (pull against the RP)
;   CRATETOTAL: to set up the maximum absorption for each crate.
;   PERMANENT:  to save the threshold values in the flash memory of BCU.
;
;   NB: the PUSHTOTAL and PULLTOTAL keyword are settable only for the Crate Master
;   Typical values of crateLimit at 90 [deg] elevation angle: /CRATETOTAL:16 [A] Max absorption on single crate
;                                                             /PUSHTOTAL: 40 [A] Max total current for pushing shell away from RP
;                                                             /PULLTOTAL: 40 [A] Max total current for pulling shell against the RP
;   Typical values of crateLimit at 10 [deg] elevation angle: /CRATETOTAL:16 [A] Max absorption on single crate
;                                                             /PUSHTOTAL: 60 [A] Max total current for pushing shell away from  RP
;                                                             /PULLTOTAL: 40 [A] Max total current for pulling shell against the RP
; HISTORY:
;   Created by Marco Xompero on 2008
;-

Function set_curr_limits, crateLimit, crateToSet                                          $
			, PUSHTOTAL=pushtotal, PULLTOTAL=pulltotal, CRATETOTAL=cratetotal $
			, PERMANENT=permanent


	@adsec_common
        if ~keyword_set(PUSHTOTAL) and ~keyword_set(PULLTOTAL) and ~keyword_set(CRATETOTAL) then begin
            message, "You must specify a keyword for overcurrent protection setting", /INFO
            return, adsec_error.IDL_MISSING_INPUT
        endif


	pbgainmatrix = readfits(adsec_path.data+'pbgainmatrix.fits', /SILENT)

	err = getdiagnvalue(process_list.housekeeper.msgd_name, 'BCUPowerBackplaneSerial', crateToSet, crateToSet, pb, TIME=5)
	if err ne adsec_error.ok then return, err
   	err = getdiagnvalue(process_list.housekeeper.msgd_name, 'BCUDigitalIOCrateIsMaster', crateToSet, crateToSet, ism, TIME=5)
	if err ne adsec_error.ok then return, err

        pb = pb.last
        ism = ism.last

        if keyword_set(PULLTOTAL) or keyword_set(PUSHTOTAL) and ~(ism) then begin
            message, "WARNING: threshold not applicable beacause the crate is not Master.", /INFO
            return, adsec_error.IDL_INPUT_TYPE
        endif

	data0 = ishft(2L,18)
	if keyword_set(PUSHTOTAL) then begin
		idc= 22-1
		coeff= float(pbgainmatrix[idc, *, pb-1])
		val = round(coeff[0]*crateLimit^2+coeff[1]*crateLimit+coeff[2])
		data1 = ishft(3L,keyword_set(PERMANENT)*5) +ishft(val,16)
                err = reset_devices_wrap(sc.mirrctrl_id[crateToSet], 255, 255, 0L, 2L, [data0, data1])
        if err ne adsec_error.ok then return, err
        log_print, "Push total overcurrent threshold set to "+string(crateLimit, FORMAT='(F5.2)')

	endif

	if keyword_set(PULLTOTAL) then begin
		idc= 23-1
		coeff= float(pbgainmatrix[idc, *, pb-1])
		val = round(coeff[0]*crateLimit^2+coeff[1]*crateLimit+coeff[2])
		data1 = ishft(4L,keyword_set(PERMANENT)*5) +ishft(val,16)
        err = reset_devices_wrap(sc.mirrctrl_id[crateToSet], 255, 255, 0L, 2L, [data0, data1])
        if err ne adsec_error.ok then return, err
        log_print, "Pull total overcurrent threshold set to "+string(crateLimit, FORMAT='(F5.2)')
	endif

	if keyword_set(CRATETOTAL) then begin
		idc= 20-1
		coeff= float(pbgainmatrix[idc, *, pb-1])
		val = round(coeff[0]*crateLimit^2+coeff[1]*crateLimit+coeff[2])
		data1 = ishft(1L,keyword_set(PERMANENT)*5) +ishft(val,16)
                err = reset_devices_wrap(sc.mirrctrl_id[crateToSet], 255, 255, 0L, 2L, [data0, data1])
        if err ne adsec_error.ok then return, err
        endif
    ;   print, val
    ;   print, data0
    ;   print, data1

	return, adsec_error.ok

end

