Function ptr2value, strin, RECURSIVE=recursive
    
    
    names = tag_names(strin)
    for i=0, n_tags(strin)-1 do begin

        if (ptr_valid(strin.(i)))[0] then cvalue = *strin.(i) else cvalue = strin.(i)
        if ~test_type(cvalue, /struct) then cvalue = ptr2value(cvalue)
        if i eq 0 then strout = create_struct(names[i], cvalue) else strout = create_struct(names[i], cvalue, strout)

    endfor
    return, strout

End
