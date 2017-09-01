;+
;;;;;;;;;;;;;;;;;;; ROUTINES FOR STRESS COMPUTATION ;;;;;;;;;;;;;;;;;;;;
;
; PREREQUISITES:
;
; In the specified stress_basedir, the following files must be available: 
;    k_mat.sav,          (
;    s_k_global.sav,     (stress matrix and related) 
; 
; At 15 jun 07 these files can be found on aodisk-1 that can be accessed by
; obelix at /backup/lbusonibackup/FEA/p45 or lbt672. Make sure these files 
; can be accessed by the machine you run IDL on. Ask sysadmin if you want
; your workstation to NFS mount aodisk-1.
; Once this is done, go to step -1.
;
;
; -1) Edit ...../conf/stress.txt and set the right path for the stress_basedir 
;
; 0) Run idl in one of the 'adsecP45', 'adsec672a' or similar directories
;
; 1) Initialize common structures.
;
; > sigma_setup
;
; The common blocks 'stress' and 'c_s_k_mat' are filled. Use @sigma_common to import them in the $MAIN$ 
; namespace
;
; 2) Compute the stress corresponding to a given shape.
;
; > mode = dblarr(45) & mode[20]=1d-6 
; > sigma_shape, S , mode, MAX=xy, STRESS=0, NACT=near_act, dmin=dmin
;
; S is the stress pattern corresponding to the given vector of modes (m rms). 
; In the example above the stress is computed for a shell having a 1um rms of mode #20.
;
;
; 3) Compute the force needed to impose a given shape to the shell
;
; > force_shape, F, mode, MAX=maxP, NACT=nact
; 
; 
; !!!!! See README.sigma for a (maybe) more up to date version of this README !!!!!!!!!
;-

pro sigma_setup
@adsec_common
@sigma_common

if n_elements(stress) eq 0 then begin

    if n_elements(adsec_path) eq 0 then begin
        message, 'Software uninitialized. Please run idl in one of "adsecP45", "adsec672a", ... directories'
    endif

    conffile = filepath(ROOT=adsec_path.conf, "stress.txt")
    stressconf = read_ascii_structure(conffile, DEBUG=0)
    if n_elements(stressconf) eq 0 then begin
        message, 'The configuration file "'+conffile+'" does not exist or has a wrong format.'
    endif
    basedir = stressconf.stressdir
    print, 'Basedir for stress files is '+basedir

    filename = filepath(ROOT=basedir, 'k_mat.sav')
    if file_test(filename, /READ,/REGULAR) then begin 
        restore, filename, /v
    endif else begin
        message, 'The k-matrix file "'+filename+'" does not exist or has a wrong format.' 
    endelse
    filename = filepath(ROOT=basedir, 's_k_global.sav')
    if file_test(filename, /READ,/REGULAR) then begin 
        restore, filename, /v
    endif else begin
        message, 'The s_k_global file "'+filename+'" does not exist or has a wrong format.' 
    endelse

    px2m = 2*(adsec_shell.out_radius/1000.)/ngrid

    act_xy    = transpose(adsec.act_coordinates[*,adsec.mir_act2act[0:adsec_shell.n_true_act-1]] / 1000.0)
    act_xy_px = act_xy / (adsec_shell.out_radius/1000.0) * ngrid/2 + ngrid / 2.
    frame_act = intarr(ngrid,ngrid)
    frame_act[act_xy_px[*,0], act_xy_px[*,1]]=1

    ; create triplet of glue droplets  
    ; droplets are spaced 120deg at 3mm from the actuator center
    ; hence at coords (0,r), (rcos120,-rsin120), (-rcos120, -rsin120) 
    r = 3e-3
    ACT_GLUE =  transpose([ $
           [ ACT_XY[*,0]  , ACT_XY[*,0]+r*cos(!pi/6) , ACT_XY[*,0]-r*cos(!pi/6) ] , $
           [ ACT_XY[*,1]+r, ACT_XY[*,1]-r*sin(!pi/6) , ACT_XY[*,1]-r*sin(!pi/6) ]   $
                ])

    ; frame_act_glue has the same size of stress patterns and is 1 where glue drops are
    ; ROZZO use image_show, /as,/sh, S + stress.frame_act_glue*max(S) to overimpose stress pattern S and actuator position
    act_glue_px = act_glue / (adsec_shell.out_radius/1000.0) * ngrid/2  + ngrid/2
    frame_act_glue = intarr(ngrid,ngrid)
    frame_act_glue[ round(act_glue_px[0,*]), round(act_glue_px[1,*]) ] = 1

    ; thresholds from conffile
    stress_thre = stressconf.stress_thre
    force_act_thre = stressconf.force_act_thre
    int_force_thre = stressconf.int_force_thre
    stroke_thre = stressconf.stroke_thre

    stress = { k_mat      : temporary(k_mat),       $
               v_k_mat    : temporary(v_k_mat),     $
               w_k_mat    : temporary(w_k_mat),     $
               ngrid      : ngrid,       $
               idxpupil   : idxpupil,    $
               stress_n   : stress_n,    $
               act_xy     : act_xy,      $
               act_xy_px  : act_xy_px,   $
               act_glue   : act_glue,    $
               px2m       : px2m,        $
               frame_act  : frame_act,   $
               frame_act_glue: frame_act_glue, $
               stress_thre: stress_thre, $
               force_act_thre: force_act_thre, $
               int_force_thre: int_force_thre, $
               stroke_thre: stroke_thre, $
               basedir    : basedir      $
             }
    c_s_k_mat = temporary(s_k_mat)

    tele = { r_out     : 8.25d/2,         $;
             lambda    :  0.5d-6,         $ ;
             r0        :  0.1d            $ ;
           }
    print, ''
    print, 'You may want to run @sigma_common to import the common blocks (stress, c_s_k_mat, ...) into the main namespace'
    print, ''

endif else begin
    print, 'Structures for stress simulation have been already initialized'
    print, 'You may want to run @sigma_common to import the common blocks (stress, c_s_k_mat, ...) into the main namespace'
endelse
end

