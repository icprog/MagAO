; $Id: get_ave_pc.pro,v 1.15 2007/11/05 17:07:41 marco Exp $
;+
;   NAME:
;    GET_AVE_PC
;
;   PURPOSE:
;    It uses the accumulator to retrieve current and position data.
;
;   USAGE:
;    err=get_ave_pc(ave_pos, ave_curr, min_pos, max_pos, SAMPLE=samples, DELAY=delay)
;
;   INPUT:
;    None.
;
;   OUTPUT:
;    Error code.
;
;   KEYWORD:
;    SAMPLES:   number of samples for an acquisition. The default is 1024.
;    DELAY:     delay clock cycles to wait before and acquisition. The deffault is 10.
;
; HISTORY
;   ????: written by A. Riccardi (AR)
;   13 July 2004: AR
;    long2float not longer used
;    use of ulong type is now implemented
;
;   27 Jul 2004: AR
;    *used new start_acc mechanism to start the accumulators
;   02 Nov 2004, MX
;    Adsec.all changed in sc.all.
;   08 Feb 2005, MX
;    Help written.
;   06 Apr 2005, AR
;    removed division by samples. Now it is implemented inside DSPs.
;   18 Apr 2005, DZ
;    cast to float ave_pos, ave_curr, min_pos, max_pos when the rms was
;    calculated
;   20 May 2005, MX
;    Min and max parameters now calculated with rms and new reading function "read_seq_acc".
;    min_pos and max_pos defined as a distance of rms/2 from the mean.
;   03 Aug 2005, MX
;    Position and current rms are now available on output.
;   March 2006, MX
;    Re-start accumulator with old values at the end of procedure.
;   07 Feb 2007 MX
;       LUT of variable deleted and name refurbished.
;
;-
;
function get_ave_pc, ave_pos, ave_curr, min_pos, max_pos, rms_pos, rms_curr, SAMPLES=samples, $
                     DELAY=delay

@adsec_common

if n_elements(samples) eq 0 then samples=1024UL
if n_elements(delay) eq 0 then delay=10UL
t_wait = (adsec.sampling_time*(delay+samples)) > 0.01

err = set_acc(samples, delay, old_samples, old_delay)
if err ne adsec_error.ok then return, err

;err = set_io_buffer(sc.all, [0,1], [dsp_map.distance, dsp_map.float_dac_value], uint(samples+delay), dsp_map.reset_acc, 1UL,  /trig , RECORD=adsec.n_act_per_dsp, /start)
err = start_acc()
if err ne adsec_error.ok then return, err

wait, t_wait
max_count = 5
count = 1
repeat begin
    err=read_seq_dsp(sc.all_dsp, dsp_map.acc_counter, 1L, acc_counter, /ULONG)
    if err ne adsec_error.ok then return, err

    max_time = (max(acc_counter)*adsec.sampling_time)
    if max_time ne 0 then begin
        wait, max_time > 0.01
        count = count + 1
    endif
endrep until (max_time eq 0.0) or (count gt max_count) or sc.host_comm eq "Dummy"
if max_time ne 0.0 and sc.host_comm ne "Dummy" then begin
    message, "Accumulation was not ended after "+strtrim(max_count)+" attempts.", $
        CONT = (sc.debug eq 0B)
    return, adsec_error.acc_not_ended
endif

;Note: instead of  dist_average and curr_average, accumulator values
;are used because the error in the rms compute is 4e-13 then 4e-8.
;err = read_io_buffer_ch(sc.all, [0,1], (delay+samples), buf)

err=read_seq_acc(sc.all, dsp_map.dist_accumulator,dsp_const.pos_fixed_point,acc_pos, /bit64, /ISPOS)
if err ne adsec_error.ok then return, err
ave_pos = acc_pos/double(samples)

err=read_seq_acc(sc.all,dsp_map.curr_accumulator,dsp_const.curr_fixed_point, acc_curr,/bit64)
if err ne adsec_error.ok then return, err
ave_curr = acc_curr/double(samples)

err=read_seq_acc(sc.all, dsp_map.dist_accumulator2,(dsp_const.pos_fixed_point)^2,dist_variance, /bit80, /ISPOS)
if err ne adsec_error.ok then return, err

err=read_seq_acc(sc.all, dsp_map.curr_accumulator2,(dsp_const.curr_fixed_point)^2,curr_variance, /bit80)
if err ne adsec_error.ok then return, err

rms_pos = sqrt((dist_variance/double(samples)-ave_pos^2) > 0)
rms_curr = sqrt((curr_variance/double(samples)-ave_curr^2) > 0)
min_pos = float(ave_pos-rms_pos/2)
max_pos = float(ave_pos+rms_pos/2)
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
ave_pos=float(ave_pos)
ave_curr=float(ave_curr)

; restore the previous status
;
err = set_acc(old_samples, old_delay)
if err ne adsec_error.ok then return, err

;rimetto in canna i valori corretti
err = start_acc()
if err ne adsec_error.ok then return, err
wait, (old_samples[0]+old_delay[0])*adsec.sampling_time


return,adsec_error.ok
end
