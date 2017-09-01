;find the error in error_db.txt
;temporary ROUTINE
;error_find, error_num

pro error_find, err_num
str = getenv('ADOPT_SOURCE')
spawn, 'grep '+ strtrim(abs(err_num), 2) + str+'/errordb.txt -i'
end
