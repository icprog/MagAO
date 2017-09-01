; $Id: get_ave_nlpc.pro,v 1.6 2007/02/07 17:21:41 marco Exp $
;+
; 27 Jul 2004: A. Riccardi
;   * default value of samples now matches the same default value of
;     get_ave_pc
;   * some code optimization
;   07 Feb 2007 MX
;       LUT of variable deleted and name refurbished.
;-
  

function get_ave_nlpc, ave_pos, ave_curr, sd_pos, sd_curr, SAMPLES=samples, BUF=buf

@adsec_common 

if n_elements(samples) eq 0 then samples=1024UL
if samples lt 1 or samples gt dsp_const.MAX_DATA_IO_LEN then return,adsec_error.generic_error

err = get_ch_data_buf( [dsp_map.float_adc_value,dsp_map.float_dac_value], samples, buf)
if err ne adsec_error.ok then stop

buf=double(temporary(buf))

if samples eq 1 then begin
	buf=reform(temporary(buf))
	ave_pos=buf[*,0]
	ave_curr=buf[*,1]
	sd_pos=0.0
	sd_curr=0.0
endif else begin
	sum=reform(total(buf,1))/samples
	sum2=reform(total(buf^2,1))/samples
	ave_pos=sum[*,0]
	ave_curr=sum[*,1]
	sd_pos=sqrt(sum2[*,0]-sum[*,0]^2)
	sd_curr=sqrt(sum2[*,1]-sum[*,1]^2)
endelse

return,adsec_error.ok
end

