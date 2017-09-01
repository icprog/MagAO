;+
;   NAME:
;    CLEAR_RTR_MEM
;
;   PURPOSE:
;       Clear the dsp memory dedicated to the reconstructor data, and move temporary the start_??_ptr 
;       in a cleared zone.   
;
;   USAGE:
;    err = clear_rtr_mem(MV_PTR=mv_ptr)
;
;   INPUT:
;       None.
;   OUTPUT:
;    Error code.
;
;   KEYWORD:
;    None.
;
;   HISTORY
;    Created on April 2005 by Daniela Zanotti (DZ).
;    29 Nov 2005 Marco Xompero (MX)
;     Update mv_ptr keyword with configurable parameters. 
;    26 Jan 2007 MX
;     Removed MV_RTR keyword. Not longer used.
;   07 Feb 2007 MX
;       LUT of variable deleted and name refurbished.
;   26 Feb 2007 MX
;       Added the cleaning of switch bcu and crate bcu.
;
;   NOTE:
;    None.
;

function clear_rtr_mem, ALL=all

    @adsec_common
    
    ;DSP RTR clean
    ;cleaning bank #1 from slope_vector address to test_address for remapping
    size_mem =   ulong(rtr_map.test_address-rtr_map.slope_vector+1)
    err = write_same_dsp(sc.all, rtr_map.slope_vector, fltarr(size_mem), /CHECK)
    if err ne adsec_error.ok then return, err

    ;cleaning bank #2 from matrix_m2c address to end for remapping
    size_mem =   '100000'xl + dsp_datasheet.data_mem_size - rtr_map.matrix_m2c 
    err = write_same_dsp(sc.all, rtr_map.matrix_m2c, fltarr(size_mem), /CHECK)
    if err ne adsec_error.ok then return, err

    ;SWITCH RTR clean
    size_mem = dsp_datasheet.data_mem_size
    err = write_same_dsp(sc.all, rtr_map.switch_SCSlopeVector, fltarr(size_mem), /CHECK, /SWITCH)
    if err ne adsec_error.ok then return, err

    err = write_same_dsp(sc.all, '100000'xl, fltarr(size_mem), /CHECK, /SWITCH)
    if err ne adsec_error.ok then return, err

    ;Crate BCU clean
    ;bank #1
    size_mem = dsp_datasheet.data_mem_size
    err = write_same_dsp(sc.all, rtr_map.bcu_ReplyVector, fltarr(size_mem), /CHECK, /BCU)
    if err ne adsec_error.ok then return, err

    ;bank #2
    err = write_same_dsp(sc.all, rtr_map.bcu_DiagVector, fltarr(size_mem), /CHECK, /BCU)
    if err ne adsec_error.ok then return, err


    return, adsec_error.ok

end
