;+
; convert the DSP value of preshaper step in preshaper setting time
; (in seconds)
;
; HISTORY
;   11 Aug 2005, written by A. Riccardi (AR)
;; TODO:::: FIX THE 0 CASE
;-
function step2time_preshaper, step_preshaper

    @adsec_common
    if sc.name_comm ne "Dummy" then begin
        if total(step_preshaper) ne 0 then $
          return, double(adsec.preshaper_len)*adsec.sampling_time/step_preshaper $
        else return, fltarr(adsec.n_actuators)
    endif else begin
        return, fltarr(adsec.n_actuators)
    endelse

end
