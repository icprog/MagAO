Function init_process_list, file2restore

    @adsec_common
    if n_elements(file2restore) eq 0 then begin
        log_message, 'No filename provided '+file2restore, CONT = (sc.debug eq 0), ERR=adsec_error.invalid_file
        return, !AO_ERROR
    endif

    if ~(file_test(file2restore, /READ, /REGULAR)) then begin
        log_message, 'Wrong filename provided '+file2restore, CONT = (sc.debug eq 0), ERR=adsec_error.invalid_file
        return, !AO_ERROR
    endif
    err = get_process_list(file2restore)
    if err ne adsec_error.ok then return, err

    return, adsec_error.ok

End
