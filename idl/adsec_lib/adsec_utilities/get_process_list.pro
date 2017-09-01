Function get_process_list, filename, OUT=p_list

    @adsec_common
    ;READ PROCESS LIST TEXT FILE
    if n_elements(filename) eq 0 then filename = adsec_path.conf+'adsec/current/processConf/processList.conf'
    ll = read_ascii_structure(filename)
    pl_names =[ strsplit(ll.sys_processes, ' ', /ex), strsplit(ll.adsec_processes,' ', /ex), strsplit(ll.interfaces, ' ', /ex)]
    p_list = { processList: filename}
    for i=0, n_elements(pl_names)-1 do begin
        if pl_names[i] ne "msgdrtdb" then begin
            add = {                                                            $
                msgd_name : pl_names[i]+'.'+getenv('ADOPT_SIDE'),              $
                conf :  file_dirname(filename)+'/'+pl_names[i]+'.conf'     $
            }
            p_list = create_struct(p_list, pl_names[i], add)
        endif
    endfor

    process_list = p_list
    if n_elements(adsec_error) gt 0 then return, adsec_error.ok else return, 0

end

    
    
