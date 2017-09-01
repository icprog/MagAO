
; Returns a tracking number formatted as a 'YYYYMMDD_HHMMSS' string

function get_tracknum

date = bin_date(systime())
tracknum = strtrim(date[0],2)
if date[1] lt 10 then tracknum += '0'
tracknum += strtrim(date[1],2) 
if date[2] lt 10 then tracknum += '0'
tracknum += strtrim(date[2],2)
tracknum += '_'
if date[3] lt 10 then tracknum += '0'
tracknum += strtrim(date[3],2)
if date[4] lt 10 then tracknum += '0'
tracknum += strtrim(date[4],2)
if date[5] lt 10 then tracknum += '0'
tracknum += strtrim(date[5],2)

return, tracknum

end
