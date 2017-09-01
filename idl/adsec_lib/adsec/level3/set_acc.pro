; $Id: set_acc.pro,v 1.14 2005/05/20 16:39:02 labot Exp $
;+
; HISTORY
;   13 July 2004: A. Riccardi (AR)
;     changed data types to match LBT specifications
;
;   27 Jul 2004: AR
;     *old_samples and old_delay vectors are returned on request
;
;   27 Jul 2004: M. Xompero (MX) and  AR
;     check on delay_acc modified. Now delay_acc can be zero.
;     input samples and delay can be vector type (only n_dsp elements)
;   02 Nov 2004, MX
;     Adsec.all changed in sc.all.
;   18 Gen 2005, MX & Daniela Zanotti(DZ)
;     Added NODIV keyword to set inv_num_samples to 1.0 in order to avoid accumulator division
;   08 Feb 2005, MX
;    'Check' keyword addded.
;   06 Apr 2005, AR
;     removed NODIV keyword
;-
function set_acc, the_samples, the_delay, old_samples, old_delay, NODIV=nodiv, CHECK=do_check

@adsec_common

if test_type(the_samples,/INT,/UINT,/LONG,/ULONG, N_EL=n_el) then begin
    message,'samples must be integer',CONT=(sc.debug ne 1B)
    return,adsec_error.input_type
endif
if n_el ne 1 and n_el ne adsec.n_dsp then begin
    message,'samples must be scalar or must have adsec.n_dsp elements.',CONT=(sc.debug ne 1B)
    return,adsec_error.input_type
endif
if total(the_samples le 0) then begin
    message,'samples must be gt 0',CONT=(sc.debug ne 1B)
    return,adsec_error.input_type
endif
samples = ulong(the_samples)
n_el_samples = n_el

if test_type(the_delay,/INT,/UINT,/LONG,/ULONG, N_EL=n_el) then begin
    message,'delay must be integer',CONT=(sc.debug ne 1B)
    return,adsec_error.input_type
endif
if n_el ne 1 and n_el ne adsec.n_dsp then begin
    message,'delay must be scalar or must have adsec.n_dsp elements.',CONT=(sc.debug ne 1B)
    return,adsec_error.input_type
endif
if total(the_delay lt 0) then begin
    message,'delay must be ge 0',CONT=(sc.debug ne 1B)
    return,adsec_error.input_type
endif
delay=long(the_delay)
n_el_delay = n_el

np = n_params()
if np ge 3 then begin
    err = read_seq_dsp(sc.all_dsp, dsp_map.num_samples, 1L, old_samples, /ULONG)
    if err ne adsec_error.ok then return, err
    if sc.host_comm eq "Dummy" then old_samples = old_samples > 1
endif
if np ge 4 then begin
    err = read_seq_dsp(sc.all_dsp, dsp_map.delay_acc, 1L, old_delay, /ULONG)
    if err ne adsec_error.ok then return, err
endif

if n_el_samples eq 1 then begin
    err = write_same_dsp(sc.all_dsp, dsp_map.num_samples, samples, CHECK=do_check)
    if err ne adsec_error.ok then return, err
    err = write_same_dsp(sc.all_dsp,dsp_map.inv_num_samples,1.0/samples, CHECK=do_check)
endif else begin
    err = write_seq_dsp(sc.all_dsp, dsp_map.num_samples, samples, CHECK=do_check)
    if err ne adsec_error.ok then return, err
    err = write_seq_dsp(sc.all_dsp,dsp_map.inv_num_samples,1.0/samples, CHECK=do_check)
endelse
if err ne adsec_error.ok then return, err

if n_el_delay eq 1 then begin
    err = write_same_dsp(sc.all_dsp, dsp_map.delay_acc, delay, CHECK=do_check)
endif else begin
    err = write_seq_dsp(sc.all_dsp, dsp_map.delay_acc, delay, CHECK=do_check)
endelse
if err ne adsec_error.ok then return, err

return,adsec_error.ok

end
