;$Id: change_block.pro,v 1.2 2007/02/07 16:57:56 marco Exp $$
;+
;   NAME:
;    CHANGE_BLOCK
;
;   PURPOSE:
;    Change  the block of memory of the reconstructur gain to use into the param_selector.
;       
;   USAGE:
;    err = change_block()
;
;   INPUT:
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
;   HISTORY
;   20 Oct 2005 Daniela Zanotti(DZ)
;   07 Dec 2005 Marco Xompero (MX)
;       Routine revised to mantain old pbs settings.
;   07 Feb 2007 MX
;       LUT of variable deleted and name refurbished.
;   
;-

Function change_block

    @adsec_common
    
    ;setting block to use
    err = read_seq_dsp(sc.all, rtr_map.param_selector, 1l,pb, /ul)

    if err ne adsec_error.ok then begin
        message, 'Error reading param_selector', CONT = (sc.debug eq 0)
        return, err
    endif
     
    if total(pb - pb[0]) ne 0 then begin
        message, "param_selectors have different settings. No block changed."
        return, adsec_error.idl_pbs_mismatch
    endif

    pb = pb[0]

    if is_even(pb) then pb += 1 else pb -= 1
        
    err = write_same_dsp(sc.all_dsp, rtr_map.param_selector, pb, /CHECK)
   
    if err ne adsec_error.ok then begin
        print, 'Error writing param_selector'
        return, err
    endif

    print, "RTR matrices and filters block changed."

    return, adsec_error.ok

end
