function get_hexapod_pos, pos

    @adsec_common
    name = 'HEXAPODCTRL00'+'.HXPD_POS'
    err = read_var(name,pos)
    return, err

end
    
