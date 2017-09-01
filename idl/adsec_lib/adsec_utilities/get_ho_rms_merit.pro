function get_ho_rms_merit, tracknum
    
    @adsec_common
    if n_elements(tracknum) eq 0 then begin
        err = save_master(/dump, TRACK=tracknum)
        if err ne adsec_error.ok then return, 1e6
    endif
    filename = file_search(getenv('ADOPT_LOG')+'/ADAPTIVE-SECONDARY_*'+tracknum+'*')
    err = read_master_buffer_dump(filename, buffer)
    if err ne adsec_error.ok then return, 1e6
    nel = max(size(buffer, /DIM))
    curr = transpose(reform(buffer.crate_bcu.single_dsp_record.curraverage[*], adsec.n_actuators, nel))
    mm = transpose(adsec.ff_f_svec) ## curr
    mm -= rebin(rebin(mm, 1, adsec.n_actuators), nel, adsec.n_actuators, /samp)
    mm_s = dblarr(1, adsec.n_actuators) & for i=0, adsec.n_actuators-1 do mm_s[i] = stddev(mm[*,i])
    idcut = 100
    mm_s[0:idcut-1] = 0
    return, total(mm_s)

end
