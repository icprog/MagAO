; $Id: dyn_comp_tf.pro,v 1.7 2006/11/14 10:48:43 labot Exp $

;+
; DYN_COMP_TF
;
; this procedure computes and plot (if requested) the tranfer functions
; from the data obtained on output by the dyn_data_elab procedure.
;
; err = dyn_comp_tf(fft_data)
;
; INPUTS
;
;   fft_data:     fft of data. This variable is returned by dyn_data_elab.
;                 It contains the FFT of the acquired signals.
;
; OUTPUTS
;
;   err:          scalar long int. Error code (see adsec_error
;                 structure in init_gvar.pro)
;
; KEYWORDS
;
;   OLTF:         if set, the Open Loop Tranfer Function of the excited
;                 mode/actutor is plotted.
;
;   CLTF:         if set, the Closed Loop Tranfer Function of the excited
;                 mode/actuator is plotted.
;
;   ETF:          if set, the Error Tranfer Function of the excited
;                 mode/actuator is plotted.
;
;   PTF:          if set, the Plant Transfer Function of the excited
;                 mode/actuator is plotted. In this case
;                 "plant" TF means actuator+mirror+sensor TF, i.e. all
;                 is within the DAC and the ADC.
;
;   INDEX_LIST:   long or short int, vector or scalar. List of the modes or
;                 actuator tranfer function to compute.
;                 If fft_data.modal_fft is true a mode index list is
;                 considered, otherwise an actuator index list is considered.
;                 If the keyword is not defined the tranfer function of the
;                 actuator/mode fft_data.index is returned.
;
;   PLOT_TF:      if set the computed tranfer functions are plotted.
;
; NO MORE NEEDED KEYWORD;   MODAL_TF:     is used to set the appropriate sign for the transfer functions
;
;
; HISTORY
;
;   Written by A. Riccardi (AR)
;   Osservatorio Asrofisico di Arcetri, ITALY
;   <riccardi@arcetri.astro.it>
;
;   13 Mar 2002, AR
;     Bug fixed in the CONT keyword setting of the MESSAGE call.
;   20 May 2005, MX
;     Minor changes.
;   14 Nov 2006, MX
;     Added CLTF in the curr excite case.
;-
;

function dyn_comp_tf, fft_data $
                      , OLTF=oltf, CLTF=cltf, ETF=etf, FTF=ftf $
                      , PTF=ptf, INDEX_LIST=index, PLOT_TF=plot_tf;;;; $
                      ;;; NO_MORE NEEDED KEYWORD, MODAL_TF=modal_tf

@adsec_common

if test_type(fft_data, /STRUCTURE, N_EL=n_el) then begin
    message, "fft_data must be a structure returned by dyn_data_elab" $
           , CONT=(sc.debug eq 0B)
    return, adsec_error.input_type
endif
if n_el ne 1 then begin
    message, "fft_data can not be a vector of structures" $
           , CONT=(sc.debug eq 0B)
    return, adsec_error.input_type
endif

if n_elements(index) eq 0 then begin
    index = fft_data.index
    n_index = 1
endif else begin
    if check_channel(index, /NOALL) then begin
        message, "The INDEX_LIST keyword must contains valid mode/act. " $
               + "numbers", CONT=(sc.debug eq 0B)
        return, adsec_error.input_type
    endif
    n_index = n_elements(index)
endelse

if fft_data.modal_fft then sign=1.0 else sign=-1.0

dummy = min(abs(fft_data.freq_vector-fft_data.freq_range[0]), idx0)
dummy = min(abs(fft_data.freq_vector-fft_data.freq_range[1]), idx1)
idx0 = idx0 > 1
if idx0 eq idx1 then begin
    idx0=1
    idx1=n_elements(fft_data.freq_vector)-1
endif

;; Plant TF
;;
if arg_present(ptf) then begin
    if fft_data.curr_excite then begin
        ptf = sign*fft_data.fft_position[*,index] $
              /(fft_data.fft_command[*,index]+fft_data.fft_ctrl_current[*,index])
    endif else begin
        ptf = sign*fft_data.fft_position[*,index]/fft_data.fft_ctrl_current[*,index]
    endelse

    if keyword_set(plot_tf) then begin
        window, /FREE, TITLE="Plant TF"
        !p.multi = [0,2,1]
        plot_amp, minmax(fft_data.freq_vector[idx0:idx1]), complex(minmax(abs(ptf[idx0:idx1,*]))) $
                , AUNITS="[pcount/ccount]", TITLE="Plant TF", /NODATA, SUBTITLE="idx="+strjoin(strtrim(index,2), " ");, xs=17
        for i=0,n_index-1 do $
            plot_amp, fft_data.freq_vector[idx0:idx1], ptf[idx0:idx1,i] $
                    , /OVERPLOT
        plot_phase, fft_data.freq_vector[idx0:idx1], ptf[idx0:idx1,0] $
                  , YRANGE=[-270.0, 20.0];, xs=17
        for i=1,n_index-1 do $
            plot_phase, fft_data.freq_vector[idx0:idx1], ptf[idx0:idx1,i] $
                    , /OVERPLOT
    endif
endif

;; Filter (compensator) TF
;;
if arg_present(ftf) then begin
    if fft_data.curr_excite then begin
        ftf = sign*fft_data.fft_ctrl_current[*,index]/(-fft_data.fft_position[*,index])
    endif else begin
        ftf = sign*fft_data.fft_ctrl_current[*,index] $
              /(fft_data.fft_command[*,index]-fft_data.fft_position[*,index])
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; debug code for the speed loop filter law computation
;;        ftf = sign*fft_data.fft_ctrl_current[*,index] $
;;              /(-fft_data.fft_position[*,index])
    endelse

    if keyword_set(plot_tf) then begin
        window, /FREE, TITLE="Filter TF"
        !p.multi = [0,2,1]
        plot_amp, minmax(fft_data.freq_vector[idx0:idx1]), complex(minmax(abs(ftf[idx0:idx1,*]))) $
                , AUNITS="[ccount/pcount]", TITLE="Filter TF", /NODATA, SUBTITLE="idx="+strtrim(index,2);, xs=17
        for i=0,n_index-1 do $
            plot_amp, fft_data.freq_vector[idx0:idx1], ftf[idx0:idx1,i] $
                    , /OVERPLOT
        plot_phase, fft_data.freq_vector[idx0:idx1], ftf[idx0:idx1,0] $
                  , YRANGE=[-180.0, 180.0];, xs=17
        for i=1,n_index-1 do $
            plot_phase, fft_data.freq_vector[idx0:idx1], ftf[idx0:idx1,i] $
                    , /OVERPLOT
    endif
endif

;; Error TF
;;
if arg_present(etf) then begin
    if fft_data.curr_excite then begin
        message, "The Error TF is not defined in this case", /INFO
        etf = 0
    endif else begin
        etf = (fft_data.fft_command[*,index]-fft_data.fft_position[*,index]) $
              /fft_data.fft_command[*,index]
    endelse

    if keyword_set(plot_tf) then begin
        window, /FREE, TITLE="Error TF"
        !p.multi = [0,2,1]
        plot_amp, minmax(fft_data.freq_vector[idx0:idx1]), complex(minmax(abs(etf[idx0:idx1,*]))) $
                , TITLE="Error TF", /NODATA, SUBTITLE="idx="+strtrim(index,2);, xs=17
        for i=0,n_index-1 do $
            plot_amp, fft_data.freq_vector[idx0:idx1], etf[idx0:idx1,i] $
                    , /OVERPLOT
        plot_phase, fft_data.freq_vector[idx0:idx1], etf[idx0:idx1,0] $
                  , YRANGE=[-270.0, 20.0];, xs=17
        for i=1,n_index-1 do $
            plot_phase, fft_data.freq_vector[idx0:idx1], etf[idx0:idx1,i] $
                    , /OVERPLOT
    endif
endif

;; Closed loop TF
;;
if arg_present(cltf) then begin
    if fft_data.curr_excite then begin
        ;message, "The Closed Loop TF is not defined in this case", /INFO
        ;cltf = 0
        cltf = fft_data.fft_position[*,index]/fft_data.fft_command[*,index]
    endif else begin
        cltf = fft_data.fft_position[*,index]/fft_data.fft_command[*,index]
    endelse

    if keyword_set(plot_tf) then begin
        window, /FREE, TITLE="Closed Loop TF"
        !p.multi = [0,2,1]
        plot_amp, minmax(fft_data.freq_vector[idx0:idx1]), complex(minmax(abs(cltf[idx0:idx1,*]))) $
                , TITLE="Closed Loop TF", /NODATA, SUBTITLE="idx="+strtrim(index,2);, xs=17
        for i=0,n_index-1 do $
            plot_amp, fft_data.freq_vector[idx0:idx1], cltf[idx0:idx1,i] $
                    , /OVERPLOT
        plot_phase, fft_data.freq_vector[idx0:idx1], cltf[idx0:idx1,0] $
                  , YRANGE=[-270.0, 20.0];, xs=17
        for i=1,n_index-1 do $
            plot_phase, fft_data.freq_vector[idx0:idx1], cltf[idx0:idx1,i] $
                    , /OVERPLOT
    endif
endif


;; Open loop TF
;;
if arg_present(oltf) then begin
    if fft_data.curr_excite then begin
        oltf = sign*(-fft_data.fft_ctrl_current[*,index]) $
               /(fft_data.fft_command[*,index]+fft_data.fft_ctrl_current[*,index])
    endif else begin
        oltf = (fft_data.fft_position[*,index]) $
               /(fft_data.fft_command[*,index]-fft_data.fft_position[*,index])
    endelse

    if keyword_set(plot_tf) then begin
        window, /FREE, TITLE="Open Loop TF"
        !p.multi = [0,2,1]
        plot_amp, minmax(fft_data.freq_vector[idx0:idx1]), complex(minmax(abs(oltf[idx0:idx1,*]))) $
                , TITLE="Open Loop TF", /NODATA, SUBTITLE="idx="+strtrim(index, 2);, xs=17
        for i=0,n_index-1 do $
            plot_amp, fft_data.freq_vector[idx0:idx1], oltf[idx0:idx1,i] $
                    , /OVERPLOT
        plot_phase, fft_data.freq_vector[idx0:idx1], oltf[idx0:idx1,0] $
                  , YRANGE=[-270.0, 20.0];, xs=17
        for i=1,n_index-1 do $
            plot_phase, fft_data.freq_vector[idx0:idx1], oltf[idx0:idx1,i] $
                    , /OVERPLOT
    endif
endif

if keyword_set(plot_tf) then !p.multi=0

return, adsec_error.ok
end
