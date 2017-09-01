; Cloop modal gain optimization
;
; gainstart = gain iniziale per tutti i modi, default=0.3
; gainstep  = step con cui cambiare il gain, default= 0.05
; passo  = quanti frames mediare per ogni step (default = 500)
; nmodi     = n. modi totali del loop (default = 400) 
; iter   = n. di iterazioni dopo cui uscire (se FREERUN non e' settato), default=20
; primo_modo = primo modo da ottimizzare (default = 1), i modi piu' bassi sono lasciati a <startgain>
; FREERUN = se settato, non esce mai e continua ad ottimizzare


pro main_optgain, gainstart=gainstart, gainstep=gainstep, passo=passo, nmodi=nmodi, iter=iter, primo_modo = primo_modo, FREERUN=FREERUN

if not keyword_set(passo) then passo = 500
if not keyword_set(nmodi) then nmodi = 496
if not keyword_set(iter) then iter = 20
if not keyword_set(gainstart) then gainstart=0.3
if not keyword_set(primo_modo) then primo_modo = 1
if not keyword_set(gainstep) then gainstep=0.05

gain = fltarr(672)
gain[0:nmodi-1] = gainstart
gain_fixed = gain
var_prev = fltarr(672)
dir = fltarr(672)-1.0
cmd = fltarr(672,passo)

; SIngle gain

totdir = 1.0
totvar_prev = 0
totgain = gainstart
gain[0:nmodi-1] = totgain
hist_len=20
gain_hist=fltarr(hist_len)
totvar_hist = fltarr(hist_len)

; Set an initial gain for all modes.

dummy = set_optgain(gain)


window,0

s=0
while 1 do begin

  s+=1
  if ((not keyword_set(FREERUN)) and (s gt iter)) then break

    ; Accumulate command variance for each mode, compensating for gain and integrator effects.
  var = fltarr(672)
  for ii = 0,passo-1 do begin
    c = read_commands()
    idx = where(c ne 0)
    cmd[idx, ii] = c[idx] / gain[idx]

    if ii gt 0 then commands = cmd[*,ii] - cmd[*,ii-1] $
        else continue

    if ii lt 25 then continue   ; Skip transient response
    var += commands^2.

  endfor 

    ; Decide whether to continue incrementing or decrementing each mode gain.
    ; Reverse direction if the command variance starts to increase.

;  for g=0, nmodi-1 do begin
;    if var[g] ne 0  then begin
;        if var[g] gt var_prev[g] then dir[g] *= -1
;        gain[g] += gainstep * dir[g]
;        var_prev[g] = var[g]
;    endif
;  endfor 
;  print, gain[0:nmodi-1]

  totvar = sqrt(total(var)) / passo
  if totvar gt totvar_prev then totdir *= -1
  totgain += gainstep * totdir

  ; Limit gain
  if totgain lt 0 then totgain=0
  if totgain gt 2 then totgain=2
  print, 'Gain: ',totgain, ' totvar: ', totvar

  gain_hist[0:hist_len-2] = gain_hist[1:*]
  gain_hist[hist_len-1] = totgain

  totvar_hist[0:hist_len-2] = totvar_hist[1:*]
  totvar_hist[hist_len-1] = totvar

!p.multi=[0,1,2]
  erase
  plot,gain_hist[where(gain_hist ne 0)], xrange=[0,hist_len], title="Loop gain", psym=-4
  plot_io,totvar_hist[where(totvar_hist ne 0)], xrange=[0,hist_len], title="Total command variance", psym=-4
!p.multi=0

  wait,0.1


  totvar_prev = totvar
  gain = replicate(totgain, 672)
 
 
    ; Apply new gain and repeat
 
  dummy = set_optgain(gain)

endwhile

end