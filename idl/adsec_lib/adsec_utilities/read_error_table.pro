;+; $Id: read_error_table.pro,v 1.2 2005/10/07 16:31:02 labot Exp $$
;+
; NAME:
;        READ_ERROR_TABLE    
;
; PURPOSE:
;        The READ_ERROR_TABLE from the errordb.txt file on Supervisor software.
;
; SYNTAX:
;        struc = read_ascii_structure(filename)
;
; RETURN VALUE:
;        returns the structure adsec_error wich contains error labels,
;        codes and message.
;
; ARGUMENTS:
;   filename:  scalar string containing the name of the text file that contains the
;              sructure definition. The file has to have the ".txt" extension.
;
; DEFINITION OF THE FILE FORMAT
;
; The format of a text file describing a structure is the following:
;
; ;optional comment or blank lines
; name   number  optional message description
;
; Empty lines or lines having ";" or "#" as first non-blank character (optional
; comment lines) are skipped.
;
; Fields are separated by (single or multiple) spaces or tabs.
;
; HISTORY
;  Created: 05 Mar 2004  M.Xompero (MX) marco@arcetri.astro.it
;-

function read_error_table, filename, UNIT=unit, DEBUG=debug

if not keyword_set(debug) then on_error, 2

catch, error_status
if (error_status ne 0) then begin
    catch, /CANCEL
    ;
    ; start of cleaning code and return value setting
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    error_occurred = 1B
    if (n_elements(unit) ne 0) and (not is_open) then $
       free_lun, unit
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; end of cleaning code and return value setting
    ;
    message, !ERROR_STATE.MSG
endif

is_open = n_elements(unit) ne 0
if not is_open then openr, unit, filename, /GET_LUN

comment_chars = [";","#", " "]
comment_regex = "["+strjoin(comment_chars)+"]"
the_line = ""
is_first = 1B
tot_error = 0;
while not eof(unit) do begin
    readf, unit, the_line
    ;the_line = strtrim(the_line,2)
    if strlen(the_line) ne 0 then begin
       first_char = strmid(the_line,0,1)
       if total(first_char eq comment_chars) eq 0 then tot_error = tot_error+1;
    endif
endwhile

print, tot_error
free_lun, unit
openr, unit, filename, /GET_LUN
name = strarr(tot_error);
mess = strarr(tot_error);
value = lonarr(tot_error);
index=0
while not eof(unit) do begin
    readf, unit, the_line
;   the_line = strtrim(the_line,2)
	if strlen(the_line) ne 0 then begin
   	first_char = strmid(the_line,0,1)
      if total(first_char eq comment_chars) eq 0 then begin
			
        	the_line = (strsplit(the_line, "[;#]", /EXTRACT, /REGEX))[0]
			field = strsplit(the_line, /EXTRACT, /REGEX)
         n_fields = n_elements(field)
			mess[index]='';

      	if n_fields le 2 then begin
				mess[index] = 'Generic error by default';
      	endif else begin
				for i = 2, n_fields-1 do $
				mess[index] = mess[index] + ' ' +strcompress(string(field[i]), /REMOVE_ALL) 
      	endelse

		name[index] = strcompress(field[0],/remove_all);
		value[index] = long(field[1]);
		index = index+1
      endif
   endif
endwhile
is_first = 1B
for index = 0, tot_error-1 do begin
	if is_first then begin
		adsec_error = create_struct(name[index], value[index])
		is_first = 0B
	endif else begin
		adsec_error = create_struct(adsec_error,create_struct(name[index],value[index]))
	endelse
endfor
if n_elements(adsec_error) eq 0 then begin
    message, "No data in structure definition." $
           , /NONAME, /NOPRINT
endif else begin
    if not is_open then free_lun, unit
    return, adsec_error
endelse

end
