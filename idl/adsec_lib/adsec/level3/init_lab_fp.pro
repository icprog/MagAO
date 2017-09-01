; $Id: init_lab_fp.pro,v 1.2 2004/12/03 18:49:41 labot Exp $$
;
;+
;  NAME:
;   INIT_LAB_FP
;
;  PURPOSE:
;   Load structures in the enviroment common block to use the Field Point data acquisition.
;
;  USAGE:
;   err = INIT_LAB_FP(filename)
;
;  INPUT:
;   filename(optional)  : filename of data to load. Default 'p45_fp.txt' into the current directory.
;
;  OUTPUT:
;   error               : error code. If 0 the data are succefully loaded. If 1, something not do well.
;
;  COMMON BLOCKS:
;   env_status          : enviroment status variable on common block crated.
;
;  PACKAGE:
;   OAA libraries/Utilities.
;
;  HISTORY
;   Created on 25 Oct 2004 by Marco Xompero (MX).
;   marco@arcetri.astro.it
;   01 Dec 2004: MX
;     default configuration filename changed
;-

Function init_lab_fp, filename

if n_elements(filename) eq 0 then filename='fp.txt'

common env_status_block, env_status

a = file_search(filename)

if a eq "" then begin
   message, "File Field Point configuration NOT FOUND!!", /continue
   print, "Procedures with Field point parameter specifications not usable. Continuing..."
   return, 1
endif else begin

   env_status = read_ascii_structure(filename)
return, 0
endelse
End
