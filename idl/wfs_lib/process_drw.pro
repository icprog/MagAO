
function process_drw, filename, MODAL_HISTORY_FILE = MODAL_HISTORY_FILE, get_params = get_params

restore, filename, /v
FRAMES_LEN = 16000

; Restore optional param file

file2 = filename + '_params.sav'
info = FILE_INFO(file2)
if info.exists then restore, file2, /V

; the params struct is written into the file by the GUI
if keyword_set(params) then begin
    MODAL_HISTORY_FILE = params.PART_MODAL_DISTURB_FILE
    FRAMES_LEN = params.FRAMES_LEN
    get_params = params
endif

cc = drw.wfs_frames_counter
diff = cc[1:*] - cc[0: n_elements(cc)-2]
m = where(diff gt 100)   ; We only search for big steps

; Get the last transient
if (n_elements(m) gt 1) then begin
    last = m[ n_elements(m)-1]  
    m = last
endif

start = m-2; +2    ; One more for the diff algorithm, and one more for the 1-frame system delay between disturb and slopes
print,'Detected start of measure: frame',start
slopes = drw.slopes[ *, start:start+FRAMES_LEN-1+6]

return, transpose(slopes)
end
