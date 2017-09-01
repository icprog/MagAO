
function tracknum, OLD_FORMAT=old_format, day, time

        caldat, systime(/JULIAN), month, day, year, hh, mm, ss
        if keyword_set(old_format) then begin
            track = STRING(year, FORMAT="(I4.4)")+"_"+ $
                       STRING(month,FORMAT="(I2.2)")+"_"+ $
                       STRING(day,  FORMAT="(I2.2)")
        endif else begin
            day      = STRING(year, FORMAT="(I4.4)")+ $
                       STRING(month,FORMAT="(I2.2)")+ $
                       STRING(day,  FORMAT="(I2.2)")
            
            time     = STRING(hh,   FORMAT="(I2.2)")+ $
                       STRING(mm,   FORMAT="(I2.2)")+ $
                       STRING(ss,   FORMAT="(I2.2)")
            track    = day+'_'+time
        endelse
  


return, track
end


