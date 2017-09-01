Function full_dsp_mem_clear

    @adsec_common

    ;CLEAN ALL DSP MEMORY
    ;CRATE BCU
    err = write_same_dsp(sc.all, 0L, lonarr(bcu_datasheet.data_mem_size), /BCU)
    if err ne adsec_error.ok then return, err
    err = write_same_dsp(sc.all, '80000'xl, lonarr(bcu_datasheet.data_mem_size), /BCU)
    if err ne adsec_error.ok then return, err
    err = write_same_dsp(sc.all, '100000'xl, lonarr(bcu_datasheet.data_mem_size), /BCU)
    if err ne adsec_error.ok then return, err

    ;SWITCH BCU
    err = write_same_dsp(sc.all, 0L, lonarr(bcu_datasheet.data_mem_size), /SW)
    if err ne adsec_error.ok then return, err
    err = write_same_dsp(sc.all, '80000'xl, lonarr(bcu_datasheet.data_mem_size), /SW)
    if err ne adsec_error.ok then return, err
    err = write_same_dsp(sc.all, '100000'xl, lonarr(bcu_datasheet.data_mem_size), /SW)
    if err ne adsec_error.ok then return, err

    ;ACCELEROMETER
    err = write_same_dsp(1, 0L, lonarr(dsp_datasheet.data_mem_size), /SIG)
    if err ne adsec_error.ok then return, err
    err = write_same_dsp(1, '80000'xl, lonarr(dsp_datasheet.data_mem_size), /SIG)
    if err ne adsec_error.ok then return, err
    err = write_same_dsp(1, '100000'xl, lonarr(dsp_datasheet.data_mem_size), /SIG)
    if err ne adsec_error.ok then return, err

    err = write_same_dsp(3, 0L, lonarr(dsp_datasheet.data_mem_size), /SIG)
    if err ne adsec_error.ok then return, err
    err = write_same_dsp(3, '80000'xl, lonarr(dsp_datasheet.data_mem_size), /SIG)
    if err ne adsec_error.ok then return, err
    err = write_same_dsp(3, '100000'xl, lonarr(dsp_datasheet.data_mem_size), /SIG)
    if err ne adsec_error.ok then return, err

    err = write_same_dsp(5, 0L, lonarr(dsp_datasheet.data_mem_size), /SIG)
    if err ne adsec_error.ok then return, err
    err = write_same_dsp(5, '80000'xl, lonarr(dsp_datasheet.data_mem_size), /SIG)
    if err ne adsec_error.ok then return, err
    err = write_same_dsp(5, '100000'xl, lonarr(dsp_datasheet.data_mem_size), /SIG)
    if err ne adsec_error.ok then return, err
    ;DSP
    err = write_same_dsp(sc.all, 0L, lonarr(dsp_datasheet.data_mem_size))
    if err ne adsec_error.ok then return, err
    err = write_same_dsp(sc.all, '80000'xl, lonarr(dsp_datasheet.data_mem_size))
    if err ne adsec_error.ok then return, err
    err = write_same_dsp(sc.all, '100000'xl, lonarr(dsp_datasheet.data_mem_size))
    if err ne adsec_error.ok then return, err

    return, adsec_error.ok

End
