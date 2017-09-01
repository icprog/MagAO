;script per testare il ricostruttore con cambiamento al volo del blocco di memoria. 

block=0
err = clear_dl()

for jj=0,5 do begin 
test_rtr, /fly, block=block
;change block
  err=change_block()
  if err then message, 'Impossibile cambiare blocco'

  err = read_seq_dsp(sc.all, rtr_map.param_selector, 1l,pb, /ul)
  block=pb[0]
  print, 'block',block
endfor
end      
