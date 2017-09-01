; $Id: init_rtr_par.pro,v 1.19 2009/11/27 09:17:48 marco Exp $$
;
;+
;  NAME:
;   INIT_RTR_PAR
;
;  PURPOSE:
;   Load  parameter in the rtr structure.
;
;  USAGE:
;   err = INIT_RTR_PAR(filename,STRUCT=struct, NOFILL=nofill)
;
;  INPUT:
;   filename(optional)  : filename of data to load. Default adsec_path.conf+'filters_LBT672.txt'.
;
;  OUTPUT:
;   error               : error code.
;
;  KEYWORDS:
;   STRUCT              : structure with parameters loaded and calculated.
;   NOFILL              : do not fill the adsec structure.
;   N_SLOPE             : set the slope number to the passed number
;   N_SL_DL             : set the n_slope_delay to the passed number
;   N_DP_DL             : set the n_modes_delay to the passed number
;
;  COMMON BLOCKS:
;   RTR               : reconstructor base common block. A part of it will be filled.
;
;  PACKAGE:
;   LBT Adsec Libraries.
;
;  HISTORY
;   Created by Daniela Zanotti on 2005
;   20 May 2005 Marco Xompero (MX)
;    Help written. n_slope_delay configuration added.
;   03 Aug 2005, MX
;    New fast diagnostic structure parameter configuration added.
;   04 Aug 2005, MX
;    New parameters added for fast diagnostic and pre-added retrieving
;    n_slope from msgdrtdb.
;   29 Nov 2005 MX
;    New checks added for the slope configuration. 
;   January 2006 DZ
;    Added the new field block memory used.
;   March 2006 DZ
;   Added wfs parameters.  
;   07 Feb 2007 MX
;       LUT of variable deleted and name refurbished.
;       Disturb, mode to correct configurations added.
;   26 Feb 2006 MX
;       New switch_ and bcu_ diagnostic len added to rtr structure.
;   28 Mar 2007 MX
;       Switch diagnostic frame len updated.
;   03 April 2007, MX & DZ
;       Fixed error in the filling rtr structure. 
;   17 May 2007, MX
;       Added automatic configuration MAC address and destination ip for switch bcu and crate bcus.
;
;-



Function init_rtr_par, filename, N_SLOPE=n_slope, N_SL_DL=n_sl_dl, N_DP_DL=n_dp_dl          $
                               , DIAG_LEN=diag_len, BLOCK=block, WFS_FREQ=wfs_freq          $
                               , N_MODES2CORRECT=n_modes2correct, DISTURB_LEN=disturb_len   $
                               , STRUCT=struct, NOFILL=nofill

; common blocks definition
@adsec_common



;;=========================================================================
;; data structure loading
;;
if n_elements(filename) eq 0 then filename=filepath(ROOT=adsec_path.conf, SUB='general', "recos.txt")
data = read_ascii_structure(filename, DEBUG=0)
if n_elements(data) eq 0 then begin
    log_message, 'The configuration file "'+filename+'" does not exist or has a wrong format.', ERR=adsec_error.idl_invalid_file
endif

if keyword_set(n_slope) then begin
    data.n_slope = n_slope 
endif else begin
; TBW
endelse


if keyword_set(wfs_freq) then data.wfs_freq = wfs_freq
if keyword_set(n_sl_dl) then data.n_slope_delay = n_sl_dl
if keyword_set(n_dp_dl) then data.n_modes_delay = n_dp_dl
if keyword_set(diag_len) then data.diag_len = diag_len
if keyword_set(n_modes2correct) then data.n_modes2correct = n_modes2correct
if keyword_set(disturb_len) then data.disturb_len = disturb_len

;if ceil((data.n_slope*2)/16.) ne (data.n_slope*2)/16 then begin
if ceil((data.n_slope*2)/8.) ne (data.n_slope*2)/8 then begin
    log_message, 'The slope number set is not compatible.', ERR=adsec_error.input_type
endif

if  (data.n_modes2correct gt adsec.n_actuators) or (data.n_modes2correct/8. ne data.n_modes2correct/8) then begin
    log_message, 'The number of modes to correct is not compatible or multiple of 8.', ERR=adsec_error.input_type
endif

bcu_diag_len = long(data.diag_len-12)*adsec.n_board_per_bcu*adsec.n_dsp_per_board+8
switch_diag_len = long(data.n_slope+8+8+4)
if sc.name_comm ne "Dummy" then begin
    err = getbcuudpport(id)
    if err ne adsec_error.ok then log_message, "Error retrieving udp port for Crate BCU master diagnostic configuration.", ERR=err
    data.bcu_remote_udp_port = id

    err = getbcuudpport(id, /SWITCH)
    if err ne adsec_error.ok then log_message, "Error retrieving udp port for Switch BCU master diagnostic configuration.", ERR=err
    data.switch_remote_udp_port = id

    err = getbcuip(ip_bcu)
    if err ne adsec_error.ok then log_message, "Error retrieving IP address for Crate BCU master diagnostic configuration.", ERR=err

    err = getbcuip(ip_switch, /SWITCH)
    if err ne adsec_error.ok then log_message, "Error retrieving IP address for Switch BCU master diagnostic configuration.", ERR=err

    err = bcuip2mac(ip_switch, MAC=mac_switch, MYIP=myip)
    if err ne adsec_error.ok then log_message, "Error retrieving MAC address for Switch BCU master diagnostic configuration.", ERR=err
    data.switch_remote_mac_address = mac_switch
    data.switch_remote_ip_address = myip

    for i=0, adsec.n_crates-1 do begin
        err = bcuip2mac(ip_bcu[i], MAC=mac_bcu, MYIP=myip)
        if err ne adsec_error.ok then log_message, "Error retrieving MAC address for Crate BCU master diagnostic configuration on crate #"+strtrim(i, 2), ERR=err
        data.bcu_remote_mac_address[i] = mac_bcu 
        data.bcu_remote_ip_address[i] = myip
    endfor
endif

;;=========================================================================

struct =                                                        $
    {                                                           $
    n_slope                     : data.n_slope,                 $ ;total number of slopes
    n_slope_delay               : data.n_slope_delay,           $ ;number of step for reconstuctor filters
    n_modes_delay               : data.n_modes_delay,           $ ;number of communication boards accessed by diag communication
    n_modes2correct             : data.n_modes2correct,         $ ;number of modes to correct
    disturb_len                 : data.disturb_len,             $ ;number of modes to correct
    diag_len                    : data.diag_len,                $ ;dsp board diagnostic frame len for each dsp (with header and footer)
    bcu_diag_len                : bcu_diag_len,                 $ ;bcu diagnostic frame len
    switch_diag_len             : switch_diag_len,              $ ;switch bcu diagnostic frame len
    diag_shm_fields             : data.diag_shm_fields,         $ ;fast diagnostic fields in shared memory 
    diag_shm_head               : data.diag_shm_head,           $ ;fast diagnostic frame header len in words
    max_n_slope                 : data.max_n_slope,             $ ;max slope number
    block                       : data.block,                   $ ;block memory used
    wfs_freq                    : data.wfs_freq,                $ ;wavefront sensor frequency rate   
    enable_master_diag          : data.enable_master_diag,      $
    bcu_decimation_master       : data.bcu_decimation_master,   $            ;master bcu mode, decimation factor of CRATEBCU
    switch_decimation_master    : data.switch_decimation_master,$ ;master bcu mode, decimation factor of SWITCHBCU
    bcu_decimation_save         : data.bcu_decimation_save,     $          ;only download in SDRAM decimation
    switch_decimation_save      : data.switch_decimation_save,  $    ;only dowload in SDRAM deimation
    bcu_remote_mac_address      : data.bcu_remote_mac_address,      $
    bcu_remote_ip_address       : data.bcu_remote_ip_address,       $
    bcu_remote_udp_port         : data.bcu_remote_udp_port,         $
    switch_remote_mac_address   : data.switch_remote_mac_address,         $
    switch_remote_ip_address    : data.switch_remote_ip_address,         $
    switch_remote_udp_port      : data.switch_remote_udp_port,         $
    oversampling_time           : data.oversampling_time        $

}

;===========================================================================
if not keyword_set(nofill) then begin

    rtr.n_slope             = data.n_slope          ;number of slope
    rtr.n_slope_delay       = data.n_slope_delay    ;delay step number of slope
    rtr.n_modes_delay       = data.n_modes_delay    ;delay step number of deltapos
    rtr.n_modes2correct     = data.n_modes2correct  ;number of modes to correct
    rtr.disturb_len         = data.disturb_len      ;number of modes to correct
    rtr.diag_len            = data.diag_len         ;fast diagnostic len in words on sdram for each dsp
    rtr.bcu_diag_len        = bcu_diag_len          ;bcu diagnostic frame len
    rtr.switch_diag_len     = switch_diag_len       ;switch bcu diagnostic frame len
    rtr.diag_shm_fields     = data.diag_shm_fields  ;fast diagnostic fields in shared memory
    rtr.diag_shm_head       = data.diag_shm_head    ;fast diagnostic frame header len in words
    rtr.block               = data.block            ;block memory used   
    rtr.wfs_freq            = data.wfs_freq         ;wavefront sensor frequency rate
    rtr.disturb_len         = data.disturb_len
    rtr.enable_master_diag  = data.enable_master_diag
    rtr.bcu_decimation_master   = data.bcu_decimation_master             ;master bcu mode, decimation factor of CRATEBCU
    rtr.switch_decimation_master = data.switch_decimation_master         ;master bcu mode, decimation factor of SWITCHBCU
    rtr.bcu_decimation_save = data.bcu_decimation_save                   ;only download in SDRAM decimation
    rtr.switch_decimation_save = data.switch_decimation_save             ;only dowload in SDRAM deimation
    rtr.bcu_remote_mac_address   = data.bcu_remote_mac_address
    rtr.bcu_remote_ip_address   = data.bcu_remote_ip_address
    rtr.bcu_remote_udp_port     = data.bcu_remote_udp_port
    rtr.switch_remote_mac_address   = data.switch_remote_mac_address
    rtr.switch_remote_ip_address   = data.switch_remote_ip_address
    rtr.switch_remote_udp_port     = data.switch_remote_udp_port
    rtr.oversampling_time   = data.oversampling_time
    
endif

return, adsec_error.ok

End
