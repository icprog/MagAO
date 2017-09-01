; $Id: start_acc.pro,v 1.4 2005/08/03 17:03:16 labot Exp $
;+
; START_ACC
;
; err = start_acc()
;
; start_acc function reset and start the position and current
; accumulators of all the DSPs. Accumulators have to be previously
; configured using set_acc function.
;
; HISTORY
;   27 July 2004: written by A. Riccardi (AR)
;   02 Nov 2004, MX
;     Adsec.all changed in sc.all.
;   29 Jul 2005, MX
;     Wait keyword added.
;-
function start_acc, WAIT=wait

@adsec_common

err = write_same_dsp(sc.all_dsp, dsp_map.reset_acc, 1UL)
if err ne adsec_error.ok then return, err

if keyword_set(wait) then begin

       err = read_seq_dsp(sc.all, dsp_map.num_samples, 1L, num_s, /ul)
       if  err ne adsec_error.ok then return, err
       err = read_seq_dsp(sc.all, dsp_map.delay_acc, 1L, del_s, /ul)
       if  err ne adsec_error.ok then return, err
       wait, (max(num_s+del_s)*adsec.sampling_time * 1.1) > 0.01

endif

return,adsec_error.ok

end
