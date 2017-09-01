function convert_xml_acc, filename, root_dir, NAME_LUT=name_lut, PREFIX_LEN=prefix_len

    if n_elements(name_lut) eq 0 then n_lut=0 else n_lut=n_elements(name_lut[*,0])

    oDocument = OBJ_NEW('IDLffXMLDOMDocument')
    oDocument->Load, FILENAME=filename, /EXCLUDE
    oLinkproject = oDocument->GetFirstChild()
    oNodeList = oLinkproject->GetElementsByTagName('SYMBOL')
    nItems = oNodeList->GetLength()

    default_type = 4 ;float
    count = 0L

    for i=0L,nItems-1 do begin
        oName = oNodeList->Item(i)
        oSymbol= oName->GetAttributes()
        name = strlowcase((oSymbol->GetNamedItem("name"))->GetNodeValue())
        addr_str = (oSymbol->GetNamedItem("address"))->GetNodeValue()
        len_str  = (oSymbol->GetNamedItem("size"))->GetNodeValue()
        reads, len_str, len, FORMAT="('0x',Z)"
        reads, addr_str, addr, FORMAT="('0x',Z)"

        if len ne 0 then begin
            if n_elements(prefix_len) gt 0 then name = strmid(name, prefix_len)
            if n_lut ne 0 then begin
                idx = where(name_lut[*,0] eq name, count_lut)
                if count_lut eq 1 then name = name_lut[idx[0],1]
            endif
            if count ne 0 then begin
               acc_map = create_struct(acc_map, name, {a:addr, l:len, t:default_type, d:""})
            endif else begin
               acc_map = create_struct(name, {a:addr, l:len, t:default_type, d:""})
            endelse
            print, name, " 0x"+conv2hex(addr), len
            count += 1
        endif
        if OBJ_VALID(oSymbol) then OBJ_DESTROY, oSymbol
        if OBJ_VALID(oName) then OBJ_DESTROY, oName
    ENDFOR
    if OBJ_VALID(oNodeList) then OBJ_DESTROY, oNodeList
    if OBJ_VALID(oLinkproject) then OBJ_DESTROY, oLinkproject
    if OBJ_VALID(oDocument) then OBJ_DESTROY, oDocument

    return, acc_map
end


function convert_acc_xml_map, file, FORCE=force, TEXT=text, CRATE=crate, switch_b=switch_b
;$Id: convert_acc_xml_map.pro,v 1.1 2008/04/08 09:45:33 labot Exp $$
;+
;   NAME:
;    CONVERT_ACC_XML_MAP
;
;   PURPOSE:
;    Translate the HW xml and map file in the standard text file.
;
;   USAGE:
;    convert_dsp_xml_map, file, TEXT=text, FORCE=force, CRATE=crate, switch_b=switch_b
;
;   INPUT:
;    file:  file to convert
;
;   OUTPUT:
;    None
;
;   KEYWORDS:
;    TEXT:  string array for conversion check
;    FORCE: force new memory map write
;    CRATE: translate the CrateBCU memory map (default Dsp memory map)
;    switch_b: translate the switch_bBCU memory map 
;
;   HISTORY
;    Created on 16 Jan 2007  
;    by Marco Xompero (MX)
;    marco@arcetri.astro.it
;-
    @adsec_common

    root_dir=file_dirname(file)
    bool_kw = fix(keyword_set(CRATE))*2 + fix(keyword_set(switch_b))
    if bool_kw eq 3 then begin
        message, 'Both CRATE and switch_b keywords cannot be used.', CONT=(sc.debug ne 1)
        return, adsec_error.input_type
    endif

            filename_lut = filepath(ROOT=root_dir,"acc_map_name_lut.txt")
            filename_xml = (file_info(filepath(ROOT=root_dir,"cut_AccAcquisition.map.xml")))[0]
            map_filename = file_info(filepath(ROOT=root_dir, "acc_dm_map.txt"))
            file2add = file_info(filepath(ROOT=root_dir, "acc_map2add.txt"))
            prefix_len=4



    if ~(file_info(filename_lut)).read then begin

        message, /INFO, "Lookup table of DSP map names NOT FOUND. No name conversion will be applied."
        n_lut = 0

    endif else begin

        print, "Lookup table of DSP map names:"
        print, filename_lut
        n_lut = file_lines(filename_lut)
        tmp = strarr(n_lut)
        openr, unit, filename_lut, /GET_LUN
        readf, unit, tmp
        free_lun, unit

        name_lut = strarr(n_lut, 2)
        for i=0,n_lut-1 do name_lut[i,*]=strlowcase(strsplit(tmp[i],/EXTRACT))

    endelse

    if ~filename_xml.read then begin
        message, filename_xml.name +": not exists or not readable.", cont= (sc.debug ne 1)
        return, adsec_error.invalid_file
    endif

    acc_map = convert_xml_acc(filename_xml.name, NAME_LUT=name_lut, PREFIX_LEN=prefix_len) 

    names = strlowcase(tag_names(acc_map))
    n_names = n_elements(names)
    name_len = intarr(n_names)
    for i=0,n_names-1 do name_len[i]=strlen(names[i])
    field_len = max(name_len)+1
    padded_names = strarr(n_names)

    print, "IDL ACCELEROMETER  memory map filename:"
    print, map_filename.name

    for i=0,n_names-1 do begin
        name = names[i]+string(replicate(byte(" "),field_len-name_len[i]))
        padded_names[i] = name + string(acc_map.(i).a, FORMAT="('long x',Z8.8)")
        print, padded_names[i]
    endfor

    ;FILE TO ADD
    if file2add.read then begin
        print, "Extra lines to add to IDL DSP memory map:"
        print, file2add.name
        n_lines2add = file_lines(file2add.name)
        map2add_str = strarr(n_lines2add)
        openr, unit_add, file2add.name, /GET_LUN
        str=""
        for i=0,n_lines2add-1 do begin
            readf, unit_add, str
            map2add_str[i] = str
            print, str
        endfor
        free_lun, unit_add
    endif else begin
        message, /INFO, "No extra lines to add to IDL DSP memory map."
        message, /INFO, file2add.name+" NOT FOUND."
    endelse
    
    if n_elements(map2add_str) gt 0 then data2w = [padded_names, map2add_str] $
                                    else data2w = padded_names

    if keyword_set(TEXT) and ~keyword_set(FORCE) then begin

        bool = strcmp(text, data2w)
        if total(bool) ne n_elements(bool) then begin
            message, "The xml files don't match with the .txt files.", CONT=(sc.debug ne 1)
            return, adsec_error.invalid_file
        endif

    endif

    openw, unit, map_filename.name, /GET
    printf, unit, transpose(data2w)
    free_lun, unit




    return, adsec_error.ok
end

