Function  create_var , VarName, Length, PROPRIETARY=proprietary, STRING=string   $
                    , FLOAT=float, BYTE=byte, UINT=UINT, ULONG=ULONG, UL64=UL64

    @adsec_common
    if test_type(VarName, /STRING) then begin
        message, "VarName must be a string!", CONT=(sc.debug eq 0B)
        return, adsec_error.input_type
    endif

    if test_type(Length, /INT, /UINT, /BYTE, /LONG, /ULONG) then begin
        message, "Length must be an integer value!", CONT=(sc.debug eq 0B)
        return, adsec_error.input_type
    endif

    if keyword_set(proprietary) then VarMode='G' else VarMode='P'

    ;get the MsgRTDB variable types from shared library)
    err = get_vartypes(msg_types)
    if err ne adsec_error.ok then return, err

    VarType = msg_types[0]

    if keyword_set(float) then VarType = msg_types[1]
    if keyword_set(string) then begin
        Length += 1
        VarType = msg_types[2]
    endif
    if keyword_set(byte) then VarType = msg_types[3]
    if keyword_set(uint) then VarType = msg_types[4]
    if keyword_set(ulong) then VarType = msg_types[5]
    if keyword_set(ul64) then VarType = msg_types[6]
    ;end of the checks

    ;Dummy case
    if sc.host_comm eq "Dummy" then begin

        return, adsec_error.ok

    endif else begin

        ;Natural case
        err = call_external(sc.dll_name, 'createvar', VarName, VarMode, VarType, Length)

        if err ne adsec_error.ok then message, "SETTING VAR ERROR", CONT=(sc.debug eq 0B)
        return, err
    
    endelse

End
