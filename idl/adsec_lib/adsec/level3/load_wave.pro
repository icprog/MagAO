
function load_wave, type

; loading the 1/8 wave reference
@adsec_common
   old_ovs = rtr.oversampling_time
   if (old_ovs ne 0) then begin
       disp_mess, 'Error!! Oversampling not set to 0', /APPEND
       return, -1
   endif
  
 CASE type OF
 1: begin
     stype =''
 end
 
 2: begin
    stype ='_1o2'
 end 

 4: begin
    stype ='_1o4'
 end
 
 8: begin
    stype ='_1o8'
 end
 endcase

  name=   'wave'+stype+'.txt'

   filename=filepath(ROOT=adsec_path.commons, sub=['ref_wave'], name)

   disp_mess, 'Disabling DSP WATCHDOG...',/APPEND
   err = disable_watchdog(/DSP)
            if err ne adsec_error.ok then begin
                disp_mess, '... error on  DSP watchdog disabling.', /APPEND
                return, err
            endif
   disp_mess, '... done.', /APPEND


   err = init_adsec_wave(filename, /VER)
            if err ne adsec_error.ok then begin
                disp_mess, 'Reference wave parameters initialization failed!', /APPEND
                return, err
            endif
   disp_mess, 'Uploading reference wave paramenters...', /APPEND
            err = send_wave()
            if err ne adsec_error.ok then begin
                disp_mess, 'Reference wave parameters initialization failed!', /APPEND
                return, err
            endif


   err =  init_adsec_cals()
            if err ne adsec_error.ok then begin
                disp_mess, 'Calibration failed!', /APPEND
                return, err
            endif
   err = send_linearization()
             if err ne adsec_error.ok then begin
                 disp_mess, '... error on uploading the linearization coeffs.', /APPEND
                 return, err
             endif
    disp_mess, '... done.', /APPEND
    disp_mess, 'Enabling DSP WATCHDOG...',/APPEND
             err = enable_watchdog(/DSP)
             if err ne adsec_error.ok then begin
                 disp_mess, '... error on  DSP watchdog enabling.', /APPEND
                 return, err
             endif
    disp_mess, '... done.', /APPEND
; end of wave change
return, err
end
  
