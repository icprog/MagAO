;$Id: gen_fastdiag_template.pro,v 1.6 2008/02/05 10:49:45 labot Exp $$
;+
; NAME:
;   GEN_FASTDIAGN_TEMPLATE
;
; PURPOSE:
;   To generate a template of the fast diagnostic frame.
;
; CATEGORY:
;   General utility.
;
; CALLING SEQUENCE:
;   result = gen_fastdiag_template(nframes)
;
; INPUTS:
;   nframes: number of frame in a single template
;
; KEYWORD PARAMETERS:
;   None.
;   
; OUTPUTS:
;   result: template generated.
;
; RESTRICTIONS:
;   None.
;
; MODIFICATION HISTORY:
;   30 Nov 2005
;    Created by Marco Xompero (MX)
;    marco@arcetri.astro.it
;   07 Feb 2007 MX
;       LUT of variable deleted and name refurbished.
;   05 Jun 2007, MX
;       Accelerometer len in data structure fixed
;-

Function gen_fastdiag_template, numframes, BCU=bcu, SWITCHB=switchb
    
    @adsec_common
    If n_elements(numframes) eq 0 then numframes=1
    if keyword_set(BCU) then begin
        struct = {                                                                               $
            wfs_frames_counter              : ulonarr(numframes)                        ,$
            param_selector                  : ulonarr(numframes)                        ,$
            mirror_frames_counter           : ulonarr(numframes)                        ,$
            dist_average                    : fltarr(adsec.n_actuators, numframes)      ,$
            curr_average                    : fltarr(adsec.n_actuators, numframes)      ,$
            dist_accumulator                : dblarr(adsec.n_actuators, numframes)      ,$
            curr_accumulator                : dblarr(adsec.n_actuators, numframes)      ,$
            dist_accumulator2               : dblarr(adsec.n_actuators, numframes)      ,$
            curr_accumulator2               : dblarr(adsec.n_actuators, numframes)      ,$
            modes                           : fltarr(adsec.n_actuators, numframes)      ,$
            new_delta_command               : fltarr(adsec.n_actuators, numframes)      ,$
            ff_command                      : fltarr(adsec.n_actuators, numframes)      ,$
            int_ctrl_current                : fltarr(adsec.n_actuators, numframes)      ,$
            ff_pure_current                 : fltarr(adsec.n_actuators, numframes)      ,$
            wfs_frames_counter_check        : ulonarr(numframes)                        ,$
            param_selector_check            : ulonarr(numframes)                        ,$
            mirror_frames_counter_check     : ulonarr(numframes)                         $
            }
    endif else begin
        if keyword_set(SWITCHB) then begin
            struct = {                                                                       $
                wfs_frames_counter              : ulonarr(numframes)                        ,$
                param_selector                  : ulonarr(numframes)                        ,$
                mirror_frames_counter           : ulonarr(numframes)                        ,$
                slopes                          : fltarr(rtr.n_slope, numframes)            ,$
                safe_skip_cnt                   : ulonarr( numframes)     ,$
                pend_skip_cnt                   : ulonarr( numframes)     ,$
                wfs_global_timeout_cnt          : ulonarr( numframes)     ,$
                acc_coeffs                      : fltarr(3, numframes)                      ,$
                timestamp                       : dblarr(numframes)     ,$
                wfs_frames_counter_check        : ulonarr(numframes)                        ,$
                param_selector_check            : ulonarr(numframes)                        ,$
                mirror_frames_counter_check     : ulonarr(numframes)                         $
            }
        endif else begin
;    If n_elements(numframes) eq 0 then begin
;        struct = {                                           $
;            wfs_frames_counter                  : 0UL           ,$
;            param_selector            : 0UL           ,$
;            dummy                           : ulonarr(2)    ,$
;            dist_average                    : fltarr(4)     ,$
;            curr_average                    : fltarr(4)     ,$
;            dist_accumulator                : lon64arr(4)   ,$
;            curr_accumulator                : lon64arr(4)   ,$
;            dist_accumulator2_lo            : lon64arr(4)   ,$
;            dist_accumulator2_hi            : uintarr(4)    ,$
;            dummy_dist_accumulator2         : ulonarr(2)    ,$
;            curr_accumulator2_lo               : lon64arr(4)   ,$
;            curr_accumulator2_hi            : uintarr(4)    ,$
;            dummy_curr_accumulator2         : ulonarr(2)    ,$
;            modes                           : fltarr(4)     ,$
;            new_delta_command               : fltarr(4)     ,$
;            ff_command                      : fltarr(4)     ,$
;            int_ctrl_current                : fltarr(4)     ,$
;            ff_pure_current                 : fltarr(4)     ,$
;            wfs_frames_counter_check            : 0UL           ,$
;            param_selector_check      : 0UL           ,$
;            dummy_check                     : ulonarr(2)     $
;            }
            struct = {                                                                               $
                wfs_frames_counter              : ulonarr(numframes)                        ,$
                param_selector                  : ulonarr(numframes)                        ,$
                mirror_frames_counter           : ulonarr(numframes)                        ,$
                dist_average                    : fltarr(adsec.n_actuators, numframes)      ,$
                curr_average                    : fltarr(adsec.n_actuators, numframes)      ,$
                dist_accumulator                : dblarr(adsec.n_actuators, numframes)      ,$
                curr_accumulator                : dblarr(adsec.n_actuators, numframes)      ,$
                dist_accumulator2               : dblarr(adsec.n_actuators, numframes)      ,$
                curr_accumulator2               : dblarr(adsec.n_actuators, numframes)      ,$
                modes                           : fltarr(adsec.n_actuators, numframes)      ,$
                new_delta_command               : fltarr(adsec.n_actuators, numframes)      ,$
                ff_command                      : fltarr(adsec.n_actuators, numframes)      ,$
                int_ctrl_current                : fltarr(adsec.n_actuators, numframes)      ,$
                ff_pure_current                 : fltarr(adsec.n_actuators, numframes)      ,$
                accelerometers_coeffs           : fltarr(4, numframes)                      ,$
                wfs_frames_counter_check        : ulonarr(numframes)                        ,$
                param_selector_check            : ulonarr(numframes)                        ,$
                mirror_frames_counter_check     : ulonarr(numframes)                         $
                }
 
        endelse
    endElse

    if numframes eq 1 then begin
        struct_one = create_struct('mirror_frames_counter_check', 0ul)
        names = tag_names(struct)
        for i= n_tags(struct)-2, 0, -1 do $
            if n_elements(struct.(i)) eq 1 $
                then struct_one = create_struct(names[i],(struct.(i))[0], struct_one) $
                else struct_one = create_struct(names[i],struct.(i), struct_one)
    endif else struct_one = temporary(struct)

    return, struct_one

End
