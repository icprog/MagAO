;$Id: get_acc.pro,v 1.1 2007/03/14 09:58:52 labot Exp $$
;+
; NAME:
;   GET_ACC
;
; PURPOSE:
;   Get accumulator settings.
;
; USAGE:
;   err = get_acc(dsp_list, old_samples, old_delay)
;
; INPUT:
;   dsp_list:       dsp_list to receive
;   old_samples:    samples of accumulation set
;   old_delay:      delay of accumulation set
;
; OUTPUT:
;   Error code
;
; KEYWORD:
;   SINGLE:     test if all settings of dsp_list are equals and return a single value
;
; HISTORY
;   08 Mar 2007, Marco Xompero (MX)
;   marco@arcetri.astro.it
;   Creation and test
;-
function get_acc, dsp_list, old_samples, old_delay, SINGLE=single

    @adsec_common
    if sc.host_comm eq "Dummy" then begin
        old_samples = 4
        old_delay = 28
    endif else begin

        err = read_seq_dsp(dsp_list, dsp_map.num_samples, 1L, samples, /ULONG)
        if err ne adsec_error.ok then return, err
        
        err = read_seq_dsp(dsp_list, dsp_map.delay_acc, 1L, delay, /ULONG)
        if err ne adsec_error.ok then return, err

        if keyword_set(SINGLE) then begin
            if total(samples ne samples[0]) then begin
                message, "Not single value permitted. The accumulators have different settings.", CONT=(sc.debug eq 0)
                return, adsec_error.idl_acc_set_mismatch
            endif

            if total(samples ne samples[0]) then begin
                message, "Not single value permitted. The accumulators have different settings.", CONT=(sc.debug eq 0)
                return, adsec_error.idl_acc_set_mismatch           
            endif
            old_samples = samples[0]
            old_delay = delay[0]
        
        endif else begin
            old_samples = samples
            old_delay = delay
        endelse


    
    endelse
    
    return,adsec_error.ok

end
