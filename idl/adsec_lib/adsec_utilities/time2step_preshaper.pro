;+
; convert time (seconds) of preshaper setting time in the value of
; preshaper step to be written in the DSP memory. Because of 
; quantization of the preshaper step, the effective preshaper time can
; differ from the requested (input) one. The presheper step that match
; closer (round) the requested setting time is returned.
;
; vector is allowed as input. In this case a corresponding vector is
; returned as output. 
;
; HISTORY
;   11 Aug 2005, written by A. Riccardi (AR)
;-
function time2step_preshaper, t_preshaper_in $
  , APPLIED_TIME=applied_t_preshaper_time, VERBOSE=verbose

@adsec_common

t_preshaper = t_preshaper_in
idx = where(t_preshaper le 0.0, count)
if count gt 0 then t_preshaper[idx] = adsec.sampling_time/2d0

step_preshaper = double(adsec.preshaper_len)*adsec.sampling_time/t_preshaper
step_preshaper = step_preshaper < dsp_const.fastest_preshaper_step
step_preshaper = step_preshaper > dsp_const.slowest_preshaper_step

step_preshaper = ulong(round(step_preshaper,/L64))

applied_t_preshaper_time = step2time_preshaper(step_preshaper)

if keyword_set(verbose) then begin
    print, "Required preshaper setting time [ms]: ",strtrim(t_preshaper_in*1e3,2)
    print, "Obtained preshaper setting time [ms]: ",strtrim(applied_t_preshaper_time*1e3,2)
    print, "Obtained preshaper step             : ",strtrim(step_preshaper,2)
    print, "max allowed presh. setting time [ms]: " $
           ,strtrim(step2time_preshaper(dsp_const.slowest_preshaper_step)*1e3,2)
    print, "min allowed presh. setting time [ms]: " $
           ,strtrim(step2time_preshaper(dsp_const.fastest_preshaper_step)*1e3,2)
endif

return, step_preshaper
end
