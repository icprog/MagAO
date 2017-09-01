;+
;   NAME:
;    SET_RECONSTRUCTOR_PAR
;
;   PURPOSE:
;    Computes and writes the size of reconstructor parameters in function of the n_slope,
;    n_actuators, n_slope_delay, n_modes_delay, n_act_per_dsp(number of channels)
;
;   USAGE:
;    err = set_reconstructor_par()
;
;   INPUT:
;    None.
;
;   OUTPUT:
;    Error code.
;
;   KEYWORD:
;    None.
;
;   NOTE:
;    None.
;
; HISTORY
;   8 Feb 2005 Daniela Zanotti (DZ)
;   20 May 2005: Marco Xompero (MX)
;       Dsp_map updated to rtr_map.
;   07 Feb 2007 MX
;       LUT of variable deleted and name refurbished.
;       Added managing n_modes2correct and SWITCH BCU and Crate BCU
;       reconstructor parameters settings.
;-
function set_reconstructor_par

    @adsec_common

    n_slope = rtr.n_slope
    n_act = adsec.n_actuators
    n_modes = rtr.n_modes2correct
    n_slope_delay = rtr.n_slope_delay
    n_modes_delay = rtr.n_modes_delay
    n_ch = adsec.n_act_per_dsp

;-------------------------------------------------------------------------------------------------

    ;DSP RTR CONFIGURATION
    ; Setting last_slope_size 
    last_slope_size = ulong(n_slope*n_ch/8.0)
    err = write_same_dsp(sc.all_dsp, dsp_map.last_slope_size, last_slope_size,/check )
    if err ne adsec_error.ok then return, err

    ; Setting modes_size
    modes_size = ulong(n_modes*n_ch/8.0)
    err = write_same_dsp(sc.all_dsp, dsp_map.modes_size, modes_size,/check )
    if err ne adsec_error.ok then return, err

    ; Setting feedfwd_size 
    feedfwd_size = ulong(n_act*n_ch/8.0)
    err = write_same_dsp(sc.all_dsp, dsp_map.feedfwd_size,feedfwd_size,/check )
    if err ne adsec_error.ok then return, err

    ; Setting slope_memcpy_size 
    slope_memcpy_size = ulong(n_slope/8.0)
    err = write_same_dsp(sc.all_dsp, dsp_map.slope_memcpy_size,slope_memcpy_size,/check)
    if err ne adsec_error.ok then return, err


    ; Setting prod_slope_delay_size 
    prod_slope_delay_size = ulong(n_slope*n_ch*n_slope_delay/8.0)
    err = write_same_dsp(sc.all_dsp, dsp_map.prod_slope_delay_size,prod_slope_delay_size,/check )
    if err ne adsec_error.ok then return, err


    ; Setting modes_memcpy_size  
    modes_memcpy_size  = ulong(n_modes/8.0)
    err = write_same_dsp(sc.all_dsp, dsp_map.modes_memcpy_size ,modes_memcpy_size,/check)
    if err ne adsec_error.ok then return, err

    ; Setting prod_modes_delay_size     
    prod_modes_delay_size = ulong(n_modes*n_ch*n_modes_delay/8.0)
    err = write_same_dsp(sc.all_dsp, dsp_map.prod_modes_delay_size, prod_modes_delay_size,/check)
    if err ne adsec_error.ok then return, err

    ; Setting slope_delay_size
    slope_delay_size = ulong(n_slope*n_slope_delay)
    err = write_same_dsp(sc.all_dsp, dsp_map.slope_delay_size, slope_delay_size,/check)
    if err ne adsec_error.ok then return, err

    ; Setting modes_delay_size   
    modes_delay_size = ulong(n_modes*n_modes_delay)
    err = write_same_dsp(sc.all_dsp, dsp_map.modes_delay_size, modes_delay_size,/check)
    if err ne adsec_error.ok then return, err

;-------------------------------------------------------------------------------------------------

    ;CRATE BCU RTR CONFIGURATION: nothing to do.

;-------------------------------------------------------------------------------------------------

    ;SWITCH BCU RTR CONFIGURATION:
    ; Setting numTotSlopes
    numTotSlopes = ulong(n_slope/4.0)
    err = write_same_dsp(0, switch_map.numtotslopes, numTotSlopes, /CHECK, /SWITCH)
    if err ne adsec_error.ok then return, err

    ;Setting numActuators
    numActuators = ulong(n_act/8.0)
    err = write_same_dsp(0, switch_map.numactuators, numActuators, /CHECK, /SWITCH)
    if err ne adsec_error.ok then return, err

    ;Setting numModes
    numModes = ulong(n_modes/8.0)
    err = write_same_dsp(0, switch_map.nummodes, numModes, /CHECK, /SWITCH)
    if err ne adsec_error.ok then return, err

;-------------------------------------------------------------------------------------------------

    return,adsec_error.ok

end
