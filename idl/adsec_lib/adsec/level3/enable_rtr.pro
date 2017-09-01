;$Id: enable_rtr.pro,v 1.2 2010/01/11 10:16:31 marco Exp $$
;+
; NAME:
;   ENABLE_RTR
;
; PURPOSE:
;   This .pro enable the slope receiving process by changing a pointer in the 
;   switch bcu memory map, pointing in a free memory space.
;
; CATEGORY:
;   Adsec Hardware Function, Level 3
;
; CALLING SEQUENCE:
;   Err =  DISABLE_RTR()
;
; INPUT:
;   None.
;
; OUTPUT:
;   Err: Error code.
;
; KEYWORDS:
;   None.
;
; COMMON BLOCKS:
;   Adsec std common blocks (see ADSEC_COMMON.pro )
;
; HISTORY
;   Created by Marco Xompero on Nov 2009
;-
Function enable_rtr, CLEAN=clean

    ;CLEAN the MEMORY and EXIT
;    if keyword_set(CLEAN) then begin
        log_print, "SwitchBCU memory clean..."
        err = write_same_dsp(0, '80000'xl,rtr.n_slopes+4L,0L, /SW, /CHECK)
        if err ne adsec_error.ok then begin
            log_print, "Unable to clean switchbcu memory: maybe the SC is sending slopes?"
            return, err
        endif
        log_print, 'done'
;    endif

    

    log_print, "Set up new param selector and wfs frame counter..."
    wfsc=666ul
    err = read_seq_dsp(0, rtr_map.param_selector, 1L, pbs, /UL)
    if err ne adsec_error.ok then return, err
    offload_block = pbs and 2L^13
    rtr_block = pbs and 1L
    sto_block = pbs and 2L^14
    new_pbs = 'C0'xl or offload_block or rtr_block or sto_block
    err = write_same_dsp(0, rtr_map.switch_SCStartRTR, [wfsc,pbs], /SWITCH, /CHECK)
    if err ne adsec_error.ok then return, err
    log_print, 'done'

 
    ;MOVE RTR pointers
    log_print, "Move reconstructor pointer..."
;    err = write_same_dsp(0, switch_map.SCSlopeVectorPTR, rtr_map.switch_SCSlopeVector, /SW, /CHECK)
;    if err ne adsec_error.ok then return, err
    err = write_same_dsp(0, switch_map.SCStartRTRPTR, rtr_map.switch_SCStartRTR, /SW, /CHECK)
    if err ne adsec_error.ok then return, err
    log_print, 'done'
    return, adsec_error.ok

End
