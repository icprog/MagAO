;this function searches for the last autosaved "shape" file contained in '$ADOPT_MEAS/adsec_calib/CMD/shape/' that is its default directory. this file contains the last list of the CL actuators

;call:  cl=last_status(tracking_number, stat)
; tracking number is in the form yyyymmdd_hhmmss
;!! only minutes are processed: no autosaving faster than 1minute is expected

function last_status, tracknum, STAT=stat, VERBOSE=verbose
    if strlen(tracknum) ne 15 and strlen(tracknum) ne 8 then begin
        message, "Wrong tracking number format (required 'yyyymmdd_hhmmss' or 'yyyymmdd')"
    endif 
    if strlen(tracknum eq 8) then begin
        tracknum = tracknum + '_235959'
    endif

    flat_base='$ADOPT_MEAS/adsec_calib/CMD/shape/'
    
    flat=file_search(filepath(ROOT=flat_base,'[1-2][0-9][0-9][0-9][0-1][0-9][3-9][0-9]_[0-2][0-9][0-5][0-9][0-5][0-9]_*.sav') $
                     , COUNT=n_flat, /TEST_REGULAR)
    tn = strmid(file_basename(flat),0,15)
    tn1 = [tracknum,tn]
    idx = sort(tn1)
    idx0 = (where(idx eq 0))[0]
    if idx0 eq 0 then begin
        message, "No flattening command application found before specified date", /INFO
        return, -1
    endif 
    
    flat = flat[idx[idx0-1]-1]

;    if strlen(tracknum eq 9) then begin 
;        tracknum = tracknum + '_235959'
;    endif
;    date=tracknum;strmid(tracknum,0,8)
;    time=intarr(6)
;    time[0]=fix(strmid(date,0,4))
;    time[1]=fix(strmid(date,4,2))
;    time[2]=fix(strmid(date,6,2))
;    time[3]=fix(strmid(date,9,2))
;    time[4]=fix(strmid(date,11,2))
;    time[5]=fix(strmid(date,13,2))
;    flat_base='$ADOPT_MEAS/adsec_calib/CMD/shape/'
;    found=0
;    while (found eq 0) do begin
;    ;    print, found
;        stime=strtrim(time[0],2)+string(time[1],format='(i2.2)')+string(time[2],format='(i2.2)')+'_'+$
;              string(time[3],format='(i2.2)')+string(time[4],format='(i2.2)');+string(time[5],format='(i2.2)'+)
;    ;    print, stime
;        flat=file_search(flat_base+stime+'*.sav')
;        ;help, flat
;        if (flat[0] ne '') then begin
;            found=1
;        endif else begin
;            time[4]-=1
;            print, time
;            if time[4] lt 1 then begin
;              time[4]=59
;              time[3]-=1
;              if time[3] lt 1 then begin
;                time[3]=23
;                time[2]-=1
;                if time[2] lt 1 then begin
;                  time[2]=31
;                  time[1]-=1
;                  if time[1] lt 1 then begin
;                      time[1]=12
;                      time[0]-=1
;                  endif
;                endif
;              endif
;            endif  
;
;
;        endelse
;    endwhile
if keyword_set(verbose) then print,'Found flattening file: '+flat

restore, flat

wp=where(flattened_status.command ne 0)
wc=where(flattened_status.current ne 0)
wcl=where(flattened_status.closed_loop eq 1B)
stat=flattened_status
ret=create_struct('wcl',Wcl, 'wpos',wp,'wcur',wc)
return, ret
end
