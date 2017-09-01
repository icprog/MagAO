Function set_accelerometer  , crate_id, CONFIGURE=configure, FILTER_VEC=filter_vec, PROJ_MAT=proj_mat , INT_GAIN=int_gain $
                            , START_STORAGE = START_STORAGE, STOP_STORAGE=stop_storage, ACCGAIN=accgain                $
                            , START_ACQ=start_acq, STOP_ACQ=stop_acq , START_ACC=start_acc, STOP_ACC=stop_acc, FORCENULL_ACC=forcenull_acc
    @adsec_common


    if n_elements(crate_id) eq 0 then crate_id = 5
    dspver = float(prog_vers.switchbcu.dsp) ge 1.11
    accver = float((prog_vers.crate.siggen.dsp)[crate_id]) ge 1.01

    if keyword_set(STOP_ACC) then begin
        if dspver then begin
            err = write_seq_dsp(sc.all, switch_map.EnableAccCorrection, 0L, /SWITCH)
            if err ne adsec_error.ok then return, err
        endif
    endif

    if keyword_set(FORCENULL_ACC) then begin
        if dspver then begin
            err = write_seq_dsp(sc.all, switch_map.EnableAccCorrection, 2L, /SWITCH)
            if err ne adsec_error.ok then return, err
        endif
    endif

    if keyword_set(STOP_STORAGE) then begin
        err = write_same_dsp(crate_id,acc_map.ena_SDRAM_storing, 0L , /CHECK, /SIGGEN)
        if err ne adsec_error.ok then return, err
    endif

    if keyword_set(STOP_ACQ) then begin
        if dspver then err = write_seq_dsp(sc.all, switch_map.EnableAccAcquisition, 0L, /SWITCH) $
        else err = write_seq_dsp(sc.all, switch_map.EnableAccAquisition, 0L, /SWITCH)
        if err ne adsec_error.ok then return, err
    endif
   

    if keyword_set(CONFIGURE) then begin
        ;questa riga e' da rimuovere
        if dspver then err = write_seq_dsp(sc.all, switch_map.EnableAccAcquisition, 0L, /SWITCH) $
        else err = write_seq_dsp(sc.all, switch_map.EnableAccAquisition, 0L, /SWITCH)
        ;ACCELEROMETER DSP map configurationsc.accel_prog_ver+'AccAcquisition.ldr'
        ;ACQUISITION filter configuration
        if (n_elements(FILTER_VEC) eq 0) or ~accver then begin

            message, "WARNING: no filtered loaded. Raw data acquisition set.", /INFO
            filter_vec = [1.,0.,0.,0.,0.,0.,0.,  1.,0.,0.,0.,0.,0.,0.]
            filt_reord = filter_vec[[0,7,3,2,1,6,5,4,10,9,8,13,12,11]]
            filt2dsp = rebin(filt_reord, n_elements(filt_reord)*adsec.n_act_per_dsp, /SAMPLE)
            log_print, 'No Filter loaded'

        endif else begin

            f0num = transpose(filter_vec.filter0_num)
            f0den = -transpose(filter_vec.filter0_den)
            f1num = transpose(filter_vec.filter1_num)
            f1den = -transpose(filter_vec.filter1_den)
            f2num = transpose(filter_vec.filter2_num)
            f2den = -transpose(filter_vec.filter2_den)
            f3num = transpose(filter_vec.filter3_num)
            f3den = -transpose(filter_vec.filter3_den)
            fnum = [[f0num], [f1num], [f2num], [f3num]]
            fden = [[f0den], [f1den], [f2den], [f3den]]

            filt2dsp = [replicate(fnum[0,0], 4), replicate(fnum[0,1], 4), replicate(fnum[0,2], 4), replicate(fnum[0,3], 4)]
            for i=0, 3 do for j=3,1,-1 do filt2dsp = [filt2dsp, [replicate(fnum[j, i], 4), replicate(fden[j, i], 4)]]

        endelse

        err = write_seq_dsp(crate_id, acc_map.ch03_coeff, filt2dsp,/SIGGEN)
        if err ne adsec_error.ok then return, err
        err = write_seq_dsp(crate_id, acc_map.ch47_coeff, filt2dsp,/SIGGEN)
        if err ne adsec_error.ok then return, err
        
        ;ACCELEROMETER SRAM configuration
        err = write_same_board(crate_id,acc_sram_map.diagnostic_record_ptr, acc_map.sdram_record , /CHECK, SET_PM=dsp_datasheet.sram_mem_id, /SIGGEN)
        if err ne adsec_error.ok then return, err
        err = write_same_board(crate_id,acc_sram_map.diagnostic_record_len, 16L , /CHECK, SET_PM=dsp_datasheet.sram_mem_id, /SIGGEN)
        if err ne adsec_error.ok then return, err

        ;BIAS retrieving
        
        ;log_print, "Identification bias value for the accelerometer..."
        ;ns=1000
        ;tmp = fltarr(8, ns)
        ;for i=0, ns-1 do begin
        ;    err = read_seq_dsp(crate_id, acc_map.offgain_ADC_value, 8l, bb, /siggen) 
        ;    if err ne adsec_error.ok then return, err
        ;    tmp[*,i]=bb
        ;endfor
        ;offset = float(round(rebin(tmp, 8, 1)))
        ;print, offset

        if accver then begin

            offset = -[32761. , 32758. , 32768., 32778. , 32768. , 32791., 33119. , 33490.]
            log_print, "Sending bias value for the accelerometers..."
            err = set_accel_offset(offset)
            if err ne adsec_error.ok then return, err

            log_print, "Sending gain value for the accelerometers..."
            err = set_accel_gain(accgain)
            if err ne adsec_error.ok then return, err

        endif

    endif
   ;SWITCH BCU fastlink configuration
    num_acc = 8
    last_crt = 5
    acc_dsp = 248
    dummy = ulonarr(4)
    dummy[0] = num_acc * 65536ul+8704
    dummy[1] = 10ul*16777216+(last_crt)*1048576ul + acc_dsp*4096ul + last_crt*256 +acc_dsp
    dummy[2] = acc_map.AccFiltered
;        ;va spostata nella inizializzazione di tutta la fibra!!!!!
    dummy[3] = num_acc *65536ul+floor(switch_map.AccData/524288.)*32768ul+(switch_map.AccData)/2
    err = write_same_dsp(sc.all, switch_map.FastlinkCmd+40, dummy, /SWITCH, /CHECK)


    ;SWITCH BCU projection matrix configuration
    if dspver then err = write_seq_dsp(sc.all, switch_map.EnableACCAcquisition, 0L, /SWITCH) $
    else err = write_seq_dsp(sc.all, switch_map.EnableACCAquisition, 0L, /SWITCH)
    if err ne adsec_error.ok then return, err
    if keyword_set(PROJ_MAT) then begin
        pm2dsp = proj_mat[*]
        pm2dsp +=0.  ;1.    ; mod RB: dummy matrix to get an initial feedback on accelerometers reading
        print, pm2dsp
        err = write_seq_dsp(sc.all, switch_map.AccParams, pm2dsp, /SWITCH, /CHECK)
        if err ne adsec_error.ok then return, err
    endif

    ;SET switch BCU filter on the projected (keep only the fist two rows (Tip/Tilt)
    if dspver then begin
        init_gain=1
        tmp = float([init_gain,0,0,0,0,0,0,0,0])
        filter = rebin(tmp, 9, 4, /SAMP)
        filter[*,2:3]=0
        filt2dsp = (transpose(filter))[*]
        err = write_seq_dsp(sc.all, switch_map.AccFilter, filt2dsp, /SWITCH, /CHECK)
        if err ne adsec_error.ok then return, err
    endif


    if keyword_set(START_STORAGE) then begin
        err = write_same_dsp(crate_id,acc_map.ena_SDRAM_storing, 1L , /CHECK, /SIGGEN)
        if err ne adsec_error.ok then return, err
    endif


    if keyword_set(START_ACQ) then begin
        if dspver then err = write_seq_dsp(sc.all, switch_map.EnableAccAcquisition, 1L, /SWITCH) $
        else  err = write_seq_dsp(sc.all, switch_map.EnableAccAquisition, 1L, /SWITCH)
        if err ne adsec_error.ok then return, err
    endif

    if keyword_set(START_ACC) then begin
        if dspver then begin
            err = write_seq_dsp(sc.all, switch_map.EnableAccCorrection, 1L, /SWITCH)
            if err ne adsec_error.ok then return, err
        endif
    endif

    return, adsec_error.ok

End

;le maschere sono:#define 
;MASTER_DSPADDR_SHIFT 0
;MASTER_DSPADDR_MASK  0x00007FFF
;MASTER_DSPBCK_SHIFT  15
;MASTER_DSPBCK_MASK   0x00018000
;TOT_DATALEN_SHIFT    17
;TOT_DATALEN_MASK     0xFFFE0000
 
