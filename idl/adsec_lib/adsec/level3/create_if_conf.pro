

function create_if_conf, template, num_of_frames=num_of_frames,amprms=amprms, SHORT=short, track=track, nmodes=nmodes, KL=kl, DOCHOP=dochop, OFFSET=offset, DOPISTON=dopiston, COMMANDTYPE=commandtype, usermodes=usermodes

    @adsec_common
    tab = string(9B)
    if !d.name eq "WIN" then cr= string([13b, 10b])else cr = string([10B])    
    
    if keyword_set(DOCHOP) then begin
        default_file=adsec_path.conf+'scripts/4d_chop_default.txt'
        def=read_ascii_structure(default_file)
        if def.chop_a eq '' then def.chop_a='""'
        if def.chop_b eq '' then def.chop_b='""'
        dataw  = ''
             if (keyword_set(short)) then begin
                dataw    =         '  Tracknum = '+track+cr
                dataw   = dataw+ 'Modes Type    CHOPPING'+cr
                if n_elements(amprms) ne 0 then dataw   =  dataw+ 'Modes Amp        '+string(amprms)          +cr
                dataw   =  dataw+ 'ovs           '+string(def.ovs)         +cr
                dataw   =  dataw+ 'offset        '+string(offset)         +cr
                dataw   =  dataw+ 'dec_factor    '+string(def.dec_factor)  +cr
                dataw   =  dataw+ 'N of frames   '+string(num_of_frames)   +cr
                dataw   =  dataw+ 'chop_a        '+string(def.chop_a)      +cr
                dataw   =  dataw+ 'chop_b        '+string(def.chop_b)      +cr
                dataw   =  dataw+'****************'+cr
            endif else begin
            ;     dataw   = 'template   array'+cr
            ;    dataw   = dataw+ 'int '+string(n_elements(template))+cr
             ;   temps   = ''
            ;        for i=0, n_elements(template)-1 do temps =  temps+ string(template[i])+' '
            ;    dataw   =  dataw+ temps + cr+'end'+cr
                dataw   =  dataw+ 'nframes        int    '+string(num_of_frames)   +cr
                dataw   =  dataw+ 'ovs            float  '+string(def.ovs)         +cr
                dataw   =  dataw+ 'dec_factor     int    '+string(def.dec_factor)  +cr
                dataw   =  dataw+ 'offset         int    '+string(offset)      +cr
                ;dataw   =  dataw+ 'ini_zeros      int    '+string(def.ini_zeros)   +cr
                ;dataw   =  dataw+ 'end_zeros      int    '+string(def.end_zeros)   +cr
                ;dataw   =  dataw+ 'trigmode_amp   float  '+string(def.trigmode_amp)+cr

                ; if n_elements(amprms) ne 0 then begin
                ;    dataw   =  dataw+ 'mode_amp         float  '+string(amprms)          +cr
                ; endif else begin
                ;    dataw   =  dataw+ 'mode_amp         float  '+string(def.mode_amp)          +cr
                ;endelse
                dataw   =  dataw+ 'chop_a         string '+string(def.chop_a)      +cr
                dataw   =  dataw+ 'chop_b         string '+string(def.chop_b)      +cr
                dataw   =  dataw+ 'file4d         string '+       def.file4d       +cr
                dataw   =  dataw+ 'flat4d         string '+       def.flat4d       +cr
            endelse


    endif else begin
;

        if keyword_set(DOPISTON) then begin
            default_file=adsec_path.conf+'scripts/4d_piston_default.txt'
            def=read_ascii_structure(default_file)
            dataw  = ''
                 if (keyword_set(short)) then begin
                    dataw    =         '  Tracknum = '+track+cr
                    dataw   = dataw+ 'Modes Type    PISTONING'+cr
                    if n_elements(amprms) ne 0 then dataw   =  dataw+ 'Modes Amp        '+string(amprms)          +cr
                    dataw   =  dataw+ 'ovs           '+string(def.ovs)         +cr
                    dataw   =  dataw+ 'offset        '+string(offset)          +cr
                    dataw   =  dataw+ 'dec_factor    '+string(def.dec_factor)  +cr
                    dataw   =  dataw+ 'N of frames   '+string(num_of_frames)   +cr
                    dataw   =  dataw+ 'Type          '+def.commandtype         +cr
                    dataw   =  dataw+'****************'+cr
                endif else begin
                ;     dataw   = 'template   array'+cr
                ;    dataw   = dataw+ 'int '+string(n_elements(template))+cr
                 ;   temps   = ''
                ;        for i=0, n_elements(template)-1 do temps =  temps+ string(template[i])+' '
                ;    dataw   =  dataw+ temps + cr+'end'+cr
                    dataw   =  dataw+ 'nframes        int    '+string(num_of_frames)   +cr
                    dataw   =  dataw+ 'ovs            float  '+string(def.ovs)         +cr
                    dataw   =  dataw+ 'dec_factor     int    '+string(def.dec_factor)  +cr
                    dataw   =  dataw+ 'offset         int    '+string(offset)      +cr
                    ;dataw   =  dataw+ 'ini_zeros      int    '+string(def.ini_zeros)   +cr
                    ;dataw   =  dataw+ 'end_zeros      int    '+string(def.end_zeros)   +cr
                    ;dataw   =  dataw+ 'trigmode_amp   float  '+string(def.trigmode_amp)+cr

                    ; if n_elements(amprms) ne 0 then begin
                    ;    dataw   =  dataw+ 'mode_amp         float  '+string(amprms)          +cr
                    ; endif else begin
                    ;    dataw   =  dataw+ 'mode_amp         float  '+string(def.mode_amp)          +cr
                    ;endelse
                    dataw   =  dataw+ 'chop_a         string '+string(def.chop_a)      +cr
                    dataw   =  dataw+ 'chop_b         string '+string(def.chop_b)      +cr
                    dataw   =  dataw+ 'file4d         string '+       def.file4d       +cr
                    dataw   =  dataw+ 'flat4d         string '+       def.flat4d       +cr
                endelse


        endif else begin





;


            default_file=adsec_path.conf+'scripts/4d_opt_intmat_default.txt'
            def=read_ascii_structure(default_file)
            type='Mirror'
            if (keyword_set(short)) then begin
                dataw    =         '  Tracknum = '+track+cr
                dataw   =  dataw+ 'Modes ##'+ string(nmodes[0])+'   '+string(nmodes[1])+cr
                if keyword_set(KL) then type='KL'
                if keyword_set(usermodes) then type = 'User defined matrix'
                dataw   = dataw+ 'Modes Type    '+type+cr
                if n_elements(amprms) ne 0 then dataw   =  dataw+ 'Modes Amp        '+string(amprms)          +cr
                dataw   =  dataw+ 'Num +- applic '+string(n_elements(template))+cr
                dataw   =  dataw+ 'ovs           '+string(def.ovs)         +cr
                dataw   =  dataw+ 'dec_factor    '+string(def.dec_factor)  +cr
                dataw   =  dataw+ 'trigmode_amp  '+string(def.trigmode_amp)+cr
                dataw   =  dataw+'****************'+cr
            endif else begin
                dataw   = 'template   array'+cr
                dataw   = dataw+ 'int '+string(n_elements(template))+cr
                temps   = ''
                for i=0, n_elements(template)-1 do temps =  temps+ string(template[i])+' '
                dataw   =  dataw+ temps + cr+'end'+cr
                dataw   =  dataw+ 'ovs            float  '+string(def.ovs)         +cr
                dataw   =  dataw+ 'dec_factor     int    '+string(def.dec_factor)  +cr
                dataw   =  dataw+ 'offset         int    '+string(def.offset)      +cr
                dataw   =  dataw+ 'ini_zeros      int    '+string(def.ini_zeros)   +cr
                dataw   =  dataw+ 'end_zeros      int    '+string(def.end_zeros)   +cr
                dataw   =  dataw+ 'trigmode_amp   float  '+string(def.trigmode_amp)+cr
        
                if n_elements(amprms) ne 0 then begin 
                    dataw   =  dataw+ 'mode_amp         float  '+string(amprms)          +cr
                endif else begin
                    dataw   =  dataw+ 'mode_amp         float  '+string(def.mode_amp)          +cr
                endelse
                dataw   =  dataw+ 'file4d         string '+       def.file4d       +cr
                dataw   =  dataw+ 'flat4d         string '+       def.flat4d       +cr
            endelse
        endelse
    endelse

    return, dataw
end
