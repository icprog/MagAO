; $Id
;+
; path_str = meas_path(meas_class_str [,filename][, /NO_CREATE][, DATE=dat])
;
; returns the directory path with the today date in the subdir defined
; by the string meas_class_str of the standard meas directory. Il the
; optional argument filename is passed, it is appended to the path
; string.
;
; If the directory does not exist, the entire path is created unless
; the NO_CREATE keyword is set.
;
; By default the today date is used. A different date can be forced passing
; the corresponding string in the format "yyyy_mm_dd" to the DATE keyword.
; In case the empty string ("") is passed to DATE keyword, no date is appended
; to the path
;
; EXAMPLE
;   (the first part of the path depends on the OS and the installation path)
;   IDL> print, meas_path("noise") ;today is 2nd July 2007 12:01:21
;   ....\meas\noise\20070702_120121\
;
;   IDL> print, meas_path("noise", DATE='20070801_141027')
;	....\meas\noise\20070801_14_10_27\
;
;   IDL> print, meas_path("noise",/NO, DATE="")
;   ....\meas\noise\\
;
; HISTORY
;   02 July 2007, A. Riccardi (AR) riccardi@arcetri.astro.it
;   09 July 2007, AR: added keyword DATE
;   02 Mar 2010, changed default format. Default format is tracking number: yyyymmdd_hhmmss
;                to return the old style format (yyyy_mm_dd) use /OLD_FORMAT keyword
;-

function meas_path, meas_class_str, filename, NO_CREATE=no_create, DATE=date_str

	@adsec_common
	if n_elements(date_str) eq 0 then begin
		caldat, systime(/JULIAN), month, day, year, hh, mm, ss
        if keyword_set(old_format) then begin
            date_str = STRING(year, FORMAT="(I4.4)")+"_"+ $
		               STRING(month,FORMAT="(I2.2)")+"_"+ $
		               STRING(day,  FORMAT="(I2.2)")
        endif else begin
		    date_str = STRING(year, FORMAT="(I4.4)")+ $
		               STRING(month,FORMAT="(I2.2)")+ $
		               STRING(day,  FORMAT="(I2.2)")+"_"+ $
                       STRING(hh,   FORMAT="(I2.2)")+ $
                       STRING(mm,   FORMAT="(I2.2)")+ $
                       STRING(ss,   FORMAT="(I2.2)")
        endelse
	endif
	path = filepath(ROOT=adsec_path.meas, SUB=[meas_class_str,date_str], "")
	if ~keyword_set(no_create) then file_mkdir, path
	if n_elements(filename) ne 0 then path=filepath(ROOT=path, filename)
	return, path

end
