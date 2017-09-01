;$Id: test_gc_crate.pro,v 1.1 2007/04/12 13:48:29 labot Exp $$
;+
;   NAME:
;       TEST_GC_CRATE
;
;   PURPOSE:
;    ROUTINE TO TEST THE SINGLE CRATE IDENTITY.
;
;   NOTE:
;
;   USAGE:
;    test_gc_crate.
;
;
;   INPUT:
;
;   OUTPUT:
;       None.
;
;   KEYWORDS:
;       None.
;
;    HISTORY
;
;    March 2007, Daniela Zanotti(DZ)
;-

function test_gc_crate, step_crate 

@adsec_common
 
 step_crate = 1000000UL
 n_dsp_per_crate = adsec.n_board_per_bcu*adsec.n_dsp_per_board   
 ; inizializzo gc??
 
 ;get all gc
 dsp_list = -1
 err = get_global_counter(dsp_list, init_gc)
    if err ne adsec_error.ok then message,'Error reading global counter'

;testo il delta
 ;qn_gc = n_elements(init_gc)
 ;delta_gc = ulong(max(abs(rebin(float(init_gc), n_gc,n_gc)-rebin(float(transpose(init_gc)), n_gc,n_gc))))
 ;if delta_gc gt step_crate then begin 
 ;    message, 'There is a great delay between the gc in the dsp', /INFO
 ;    print, 'The new step is :', delta_gc
 ;    step_crate = delta_gc
 ;endif else begin
 ;    print,"The step crate is 1000000UL"
 ;endelse
           
 

 err = manage_interrupt(/disable)
    if err ne adsec_error.ok then message, 'Error during the interrupt disable' 

   
;impongo uno step_gc_diverso per ciascuno
 for i = 0, adsec.n_crates-1 do begin

    ; scrivo uno step per ogni crate piu' grande del delta_gc 
    i_dsp = i*n_dsp_per_crate
    err = write_same(i_dsp, i_dsp+n_dsp_per_crate-1, dsp_map.global_counter, step_crate*i) ; ?step p gc
     if err ne adsec_error.ok then return, err
 
 endfor
 
 err = manage_interrupt(/enable)
    if err ne adsec_error.ok then message, 'Error during the interrupt enable'


;io qui dovrei capire chi e'  chi
 err = get_global_counter(dsp_list, end_gc)
    if err ne adsec_error.ok then message,'Error reading global counter'
 window, 0, retain=2
 plot, end_gc

 return, adsec_error.ok

end
;
