


function save_master, name,DUMP=dump, TRACKNUM=track

        @adsec_common
	side = !AO_CONST.telescope_side
        if keyword_set(dump) then print, dump_diagn_history('masterdiagnostic.'+side)
        dump_fold = '$ADOPT_LOG'
        dump_name = 'ADAPTIVE-SECONDARY_00'
        ;masterdir = meas_path('master', date=track, /no_create)
        ;masterdir = '/towerdata/adsec_calib/master/'
		masterdir = '$ADOPT_MEAS/adsec_calib/master/'
        if n_elements(name) eq 0 then begin
            dump_file = file_search(dump_fold, dump_name+'*.log')
            if n_elements(dump_file) eq 0 then begin
                message, 'ERROR, no file found', /CONT
                return, ''
            endif
            ind=(sort(dump_file))[n_elements(dump_file)-1]
            print, "File to process:",dump_file[ind]
            dump2read= dump_file[ind]
        endif else begin
            dump2read=name
        endelse
        qq=strmid(dump2read, 18, /rev)
        track=(strsplit(qq,'.',/ex))[0]
        print, track
        print, read_master_buffer_dump(dump2read, data)
        name2save = masterdir+'MASTERDGN_DATA_'+track+'.sav' 
        ex=file_test(name2save)
        if ex eq 0 then begin
        print, pack_master_data(data, name2save)
        ;save, filename=name2save, data
         print, 'Saving file '+name2save 
                    return, name2save
                endif else begin
                    print, 'MASTERDGN data file has already been created'
                    return, ''   
                endelse
            
end
