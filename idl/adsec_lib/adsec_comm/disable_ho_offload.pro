Function disable_ho_offload

    @adsec_common
    err = write_var(SUBS=process_list.adsecarb.msgd_name, 'DOOFFLOADHIGHORDER',  0, /COMMON)
    return, err


End
