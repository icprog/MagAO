Function convert_dsp_map_xml_perl, filename, root_dir, NAME_LUT=name_lut

    @adsec_common
    spawn, adsec_path.prog+'xml2cfg.pl < '+filename+' > '+adsec_path.prog+sc.prog_ver+'/xml_map_converted'
    if n_elements(name_lut) eq 0 then n_lut=0 else n_lut=n_elements(name_lut[*,0])

    openr, unit, adsec_path.prog+sc.prog_ver+'/xml_map_converted', /GET_LUN
    addr = 0L
    len = 0L
    str = ""
    name = ""
    default_type = 4 ;float
    count = 0
    while 1B do begin
        readf, unit, str
        if str eq "" then break
        str_fields = strsplit(str, 'long 0x', /EXTR, /REGEX)
        if n_elements(str_fields) ne 2 then begin
            free_lun,unit
            message, "Unexpected line format."
        endif
        reads, str_fields[1], len, FORMAT="(Z)"
        if len ne 0 then begin
            name = strlowcase(strtrim(str_fields[0],2))
            if n_lut ne 0 then begin
                idx = where(name_lut[*,0] eq name, count_lut)
                if count_lut eq 1 then name = name_lut[idx[0],1]
            endif
            reads, str_fields[1], addr, FORMAT="(Z)"
            if count ne 0 then begin
               dsp_map = create_struct(dsp_map, name, {a:addr, l:len, t:default_type, d:""})
            endif else begin
               dsp_map = create_struct(name, {a:addr, l:len, t:default_type, d:""})
            endelse
            print, name, " 0x"+conv2hex(addr), len
            count = count+1
        endif
    endwhile
    free_lun, unit
    return, dsp_map
end



Function convert_dsp_map_map, filename, root_dir, NAME_LUT=name_lut

    if n_elements(name_lut) eq 0 then n_lut=0 else n_lut=n_elements(name_lut[*,0])

    openr, unit, filename, /GET_LUN
    str = ""
    str_match = "Name                          Demangled Name"
    str_len = strlen(str_match)
    repeat begin
       readf, unit, str
    endrep until strcmp(str,str_match,str_len)

    addr = 0L
    len = 0L
    name = ""
    default_type = 4 ;float
    count = 0
    while 1B do begin
        readf, unit, str
        if str eq "" then break
        str_fields = strsplit(str, '0x', /EXTR, /REGEX)
        if n_elements(str_fields) ne 3 then begin
            free_lun,unit
            message, "Unexpected line format."
        endif
        reads, str_fields[2], len, FORMAT="(Z)"
        if len ne 0 then begin
            name = strlowcase(strtrim(str_fields[0],2))
            if n_lut ne 0 then begin
                idx = where(name_lut[*,0] eq name, count_lut)
                if count_lut eq 1 then name = name_lut[idx[0],1]
            endif
            reads, str_fields[1], addr, FORMAT="(Z)"
            if count ne 0 then begin
               dsp_map = create_struct(dsp_map, name, {a:addr, l:len, t:default_type, d:""})
            endif else begin
               dsp_map = create_struct(name, {a:addr, l:len, t:default_type, d:""})
            endelse
            print, name, " 0x"+conv2hex(addr), len
            count = count+1
        endif
    endwhile
    free_lun, unit
    return, dsp_map
end


function convert_dsp_map_xml, filename, root_dir, NAME_LUT=name_lut

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
            if n_lut ne 0 then begin
                idx = where(name_lut[*,0] eq name, count_lut)
                if count_lut eq 1 then name = name_lut[idx[0],1]
            endif
            if count ne 0 then begin
               dsp_map = create_struct(dsp_map, name, {a:addr, l:len, t:default_type, d:""})
            endif else begin
               dsp_map = create_struct(name, {a:addr, l:len, t:default_type, d:""})
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

    return, dsp_map
end


pro convert_dsp_map, ROOT_DIR=root_dir, PERL=perl
;$Id: convert_dsp_map.pro,v 1.6 2007/02/07 18:06:07 marco Exp $$
;+
;   NAME:
;    CONVERT_DSP_MAP
;
;   PURPOSE:
;    Setting up the variable map translating the DSP program xml and map file.
;
;   USAGE:
;    convert_dsp_map, ROOT_DIR=root_dir
;
;   INPUT:
;    None.
;
;   OUTPUT:
;    None,
;
;   KEYWORDS:
;    ROOT_DIR:  new dir to retrieve the LUT to add. Default is adsec_path.pro/sc.prog_ver
;
;   HISTORY
;   13 July 2004, A. Riccardi
;    some diagnostic print
;    check of variable names using lowcase
;    str_match changed
;   29 Aug 2005, AR
;    Now it manages map files and xml file. 
;-
@adsec_common

if n_elements(root_dir) eq 0 then $
    root_dir=filepath(ROOT=adsec_path.prog,SUB=[sc.prog_ver], "")

filename_lut = filepath(ROOT=root_dir,"dsp_map_name_lut.txt")
if (findfile(filename_lut)) eq "" then begin
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

filename_map = (findfile(filepath(ROOT=root_dir,"DspMainProgram.map")))[0]
filename_xml = (findfile(filepath(ROOT=root_dir,"DspMainProgram.map.xml")))[0]

case (filename_map ne "")+2B*(filename_xml ne "") of
    0B: message, "No DspMainProgram map file"
    3B: message, "Both MAP and XML version of DspMainProgram are present. Remove the conflicting one."
    1B: begin
        print, "MAP release of DspMainProgram."
        xml = 0B
        filename = filename_map
    end
    2B: begin
        print, "XLM release of DspMainProgram."
        xml = 1B
        filename = filename_xml
    end
endcase

print, "Filename: ", filename

if xml then begin
    if keyword_set(perl) then dsp_map=convert_dsp_map_xml_perl(filename, root_dir, NAME_LUT=name_lut) else $
      dsp_map = convert_dsp_map_xml(filename, root_dir, NAME_LUT=name_lut) 
endif else begin
    dsp_map = convert_dsp_map_map(filename, root_dir, NAME_LUT=name_lut)
endelse

names = strlowcase(tag_names(dsp_map))
n_names = n_elements(names)
name_len = intarr(n_names)
for i=0,n_names-1 do name_len[i]=strlen(names[i])
field_len = max(name_len)+1
padded_names = strarr(n_names)
map_filename = filepath(ROOT=root_dir, "dsp_dm_map.txt")
print, "IDL DSP memory map filename:"
print, map_filename
openw, unit, map_filename, /GET_LUN
for i=0,n_names-1 do begin
    name = names[i]+string(replicate(byte(" "),field_len-name_len[i]))
    padded_names[i] = name
    printf, unit, name, dsp_map.(i).a, FORMAT="(A,'long x',Z8.8)"
    print, name, dsp_map.(i).a, FORMAT="(A,'long x',Z8.8)"
endfor
filename = filepath(ROOT=root_dir, "dsp_map2add.txt")
if (file_info(filename)).read then begin
    print, "Extra lines to add to IDL DSP memory map:"
    print, filename
    n_lines2add = file_lines(filename)
    openr, unit_add, filename, /GET_LUN
    str=""
    for i=1,n_lines2add do begin
       readf, unit_add, str
       printf, unit, str
       print, str
    endfor
    free_lun, unit_add
endif else begin
    message, /INFO, "No extra lines to add to IDL DSP memory map."
    message, /INFO, filename+" NOT FOUND."
endelse

free_lun, unit
end

