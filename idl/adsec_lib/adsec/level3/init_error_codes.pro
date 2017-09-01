;$Id: init_error_codes.pro,v 1.8 2009/09/18 08:25:01 marco Exp $$
;+
;HISTORY
; 2005 Created by Marco Xompero(MX)
; January 2006 MX
; The error codes are read from a text file.
;   07 Feb 2007 MX
;       LUT of variable deleted and name refurbished.
;-
Pro init_error_codes

    @adsec_common
    filename = filepath(ROOT=getenv('ADOPT_ROOT'), SUB=['idl'],'errordb.pro')
    str = read_ascii_structure(filename)
    str_copy = str
    newstr = create_struct('ok', 0, str_copy)
    
    ;PATCH FOR IDL ERROR
    names = tag_names(str)
    for i=0, n_tags(str)-1 do begin
        dstr = stregex(names[i], 'IDL_.*', /ex)
        if dstr ne "" then begin
            dstr = strmid(dstr, 4)
            if stregex(dstr, '^(0|1|2|3|4|5|6|7|8|9)*', /ex) ne "" then begin
                pos=stregex(dstr, '^(0|1|2|3|4|5|6|7|8|9)*', /sub)
                dstr = strmid(dstr, pos[1]+1)
            endif 
            if max(strmatch(names, dstr)) eq 0 then newstr = create_struct(dstr, str.(i),newstr )
        endif
    endfor
    adsec_error=newstr
end
