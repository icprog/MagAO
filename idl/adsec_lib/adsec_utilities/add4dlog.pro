
function add4dlog, tracknum, process

@adsec_common
default_file=adsec_path.conf+'scripts/4d_opt_intmat_default.txt'
def=read_ascii_structure(default_file)
logfile=adsec_path.meas+def.log4d
cr=string([10B])
openr, 1,logfile
datalog=''
tmplog=''
while ~ eof(1)do begin
    tmplog=''
    readf, 1, tmplog
    datalog += tmplog+cr
endwhile
close, 1
res=strsplit(datalog, tracknum, /regex, /ex)
;other=strsplit(res[1], cr+'Modes', /regex,/ex)  ; was cr
;help, other
;miss=strsplit(res[1], other[0], /regex, /ex)
;if other[0] ne cr then begin
;    datalog=res[0]+tracknum+other[0]+process+' done'+miss
;endif else begin
datalog=res[0]+tracknum+'     '+process+' done'+res[1]
;endelse
openw, 1, logfile
printf, 1, datalog
close, 1

return,0
end

