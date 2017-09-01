
function parselog, prefix, START_TIME=START_TIME, END_TIME=END_TIME, LOGGERNAME=LOGGERNAME


 cmd = './parselog '+prefix
 outputfile = '/tmp/idldata.txt'

 if keyword_set(START_TIME) then cmd += ' -start "'+strtrim(START_TIME,2)+'"'
 if keyword_set(END_TIME) then cmd += ' -end "'+strtrim(END_TIME,2)+'"'
 if keyword_set(LOGGERNAME) then cmd += ' -name "'+strtrim(LOGGERNAME,2)+'"'

 cmd += ' > '+outputfile

 ;;print, cmd
 SPAWN, cmd

 data = READ_ASCII(outputfile)

 FILE_DELETE, outputfile
 return, data

end
