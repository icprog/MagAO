;script di test della connessione tra modi e comandi
;
;carica M
;legge i modi
;calcola i comandi
;lancia lo start_mm
;polla nendof_mm
;legge il command_vector e confronta
test_fast, M2c=mm
;print, 'Setting the nendof to 1UL ...'
;err = set_nendof()

print, 'Caricamento matrice M...'
M = loadmat(rtr_map.matrix_m2c, adsec.n_actuators,adsec.n_actuators) 
print, 'Trasferimento vettore random di modes...'
modes = randomu( seed, adsec.n_actuators)
print, start_rtr()
wait,1
err = write_same_dsp(sc.all, rtr_map.modes_vector, modes)
print, 'Calcolo e verifica'
err=start_mm()
;err = write_same_dsp(sc.all, rtr_map.start_mm, 1l)
cc=1
err = read_seq_dsp(sc.all, rtr_map.start_mm, 1l, nendof_mm, /ul)
while (total(abs(nendof_mm)) ne 0) or cc eq 10 do begin
    cc +=1
    print, cc
    err = read_seq_dsp(sc.all, rtr_map.start_mm, 1l, nendof_mm, /ul)
endwhile
if cc eq 10 then message, 'Conto modi non ancora finito' else print, 'Conto modi finito in '+strtrim(string(cc),2)+' tentativi'


err = read_seq_ch(sc.all,dsp_map.new_delta_command, 1, cmd)

cal=mamano(M, modes)
if total(abs(cmd - cal)) eq 0 then print, 'Test passed.' else print, 'Test Failed'
 
 end
