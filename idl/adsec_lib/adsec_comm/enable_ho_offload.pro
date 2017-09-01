Function enable_ho_offload

    @adsec_common
    err = write_var(subs=process_list.adsecarb.msgd_name,'DOOFFLOADHIGHORDER',  1, /COMMON)
    return, err


End
