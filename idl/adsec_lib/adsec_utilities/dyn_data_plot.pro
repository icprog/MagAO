;lwrestora file, unire datiin array,fft data,  plot tf
pro dyn_data_plot, n_att, pos, ext, ENABLE=enable
@adsec_common
dir=adsec_path.meas+'dyn'+'/2004_12_01/'
dir = '/mnt/mirror/measP45/dyn/2006_12_15/'

;if keyword_set(enable) then ext=ext+'_coil_enabled' else  ext=ext+'_coil_disabled'

;strmodal = "a"
;max_amp = 0.15
;stramp = strtrim(string(max_amp,format="(e8.1)"),2)
;strsign="+"
;if ~keyword_set(n_fstep) then n_fstep = 1
;strpos=strtrim(string(pos,format="(e8.1)"),2)
; filename="dyn_"+strsign+strpos+"_"+stramp+"_"+strmodal+string(n_att,format='(I3.3)')$
;      +"__"+ext+'_??'+'.sav'
dir = '/mnt/mirror/measP45/dyn/2006_12_15/'
names = file_search(dir, 'dyn_+7.2e-05_1.0e-06_a???__b00.sav')
nomi_files = names
nomi_files = nomi_files[sort(nomi_files)]
restore, dir+'adsec_save_b.sav'




 nfiles = n_elements(nomi_files)
   for i=0, nfiles-1 do begin
      ;filename="dyn_"+strsign+pos+"_"+stramp+"_"+strmodal+string(n_att,format='(I3.3)')$
     ; +"_"+ext+string(i_fstep,FORMAT="(I2.2)")+'.sav'

      restore, nomi_files[i]
      print, i
      if i eq 0 then begin
         ptf_vector = dcomplexarr(n_samples)
         idx0_min = n_samples
         idx1_max = 0
      endif
      err = dyn_data_elab(data, fft_data, MODAL=data.modal, MODE_INDEX=data.index, ADSEC_SAVE=adsec_save)
      if err ne adsec_error.ok then message, "ERROR" ;return, err
         err = dyn_comp_tf(fft_data, INDEX=data.index, PTF=ptf)
         idx0 = round(data.freq_range[0]* adsec.sampling_time * n_samples)
         idx1 = round(data.freq_range[1]* adsec.sampling_time * n_samples)
         idx0_min = min([idx0_min, idx0])
         idx1_max = max([idx1_max, idx1])
         ptf_vector[idx0:idx1,*] = ptf[idx0:idx1,*]
   endfor
   plot_amp, fft_data.freq_vector[idx0_min:idx1_max], ptf_vector[idx0_min:idx1_max]
end
