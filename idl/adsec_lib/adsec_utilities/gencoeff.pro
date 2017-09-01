; $Id: gencoeff.pro,v 1.5 2004/07/15 17:14:06 riccardi Exp $
;+
; par is the structure returned by tfl_gui (syntax: err=tfl_gui(par))
; Units of data stored in par are :
; rad/s for poles and zeros
; N/m for gains
;
; HISTORY
;   completely re-written to match LBT specs
;   15 Jul 2004: fixed bug in the ordering of filter coefficients
;-
pro gencoeff, par, N_per_ccount, m_per_pcount, coeff

@adsec_common

samp_freq = 1d0/adsec.sampling_time ; sampling frequency

n_s_zero = par.n_s_zero
n_s_pole = par.n_s_pole

; computation of the coefficents of the numerator starting from the zeros
if n_s_zero eq 0 then begin
    s_num = [1d0]
endif else begin
    s_num = zero2coeff(par.s_zero[0:(n_s_zero-1)])
endelse
s_num = par.s_const * s_num ; [N/m]

; computation of the coefficents of the denominator starting from the poles
if n_s_pole eq 0 then begin
    s_den = [1d0]
endif else begin
    s_den = zero2coeff(par.s_pole[0:(n_s_pole-1)])
endelse

; scaling
s_num_c = (m_per_pcount/N_per_ccount) * s_num ; converts in ccount/pcount
s_den_c = s_den

tustin, s_num_c, s_den_c, samp_freq, z_num_c, z_den_c

; reorder coefficients as requested by LBT configuration
filter_max_order = adsec.tfl_max_n_coeff
coeff = fltarr((filter_max_order+1)+filter_max_order)
coeff[0] = z_num_c
coeff[0]=shift(reverse(coeff[0:filter_max_order]),1)
if n_elements(z_den_c) gt 1 then begin
    coeff[filter_max_order+1] = z_den_c[1:*]
    coeff[filter_max_order+1] = reverse(coeff[filter_max_order+1:*])
endif
end

