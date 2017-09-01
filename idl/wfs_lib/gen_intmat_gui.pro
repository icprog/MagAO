;
; Versione di gen_intmat specializzata per l'uso dalla GUI. Cambiano solo alcuni parametri di input/output
; PATH_PREFIX: prefix for all files
; INTMAT_ACQ_DIR : directory where the .sav files from SwitchBCU are located, with trailing slash. They also contains a reference to the modal disturb file
; OUTPUT_FILE: .fits file where the intmat is saved
;
; SKIP_FRAME: how many frames to skip after a push-pull step
; AVG_FRAME : how many frames to average
;
; NMODES =  how many modes to retain in the analysis. If not set, it will ask interactively
; INTERACTIVE = allow interaction (needed if NMODES is not set)
;
; Controllo saturazione:
;
; SIG_CHECK : activate saturation check
; SIG_MAX
; SIG_MIN ; threshold min/max sui segnali raw
;
; Stima rumore
;
; NOISE_ESTIMATION  : estimate the slope noise level by performing an intmat computation over an acquisition with the mirror not moving.
;
; Testadimoro correction
;
; BADMODES    :   an optional vector of modes to be removed from the interaction matrix. Used for testa di moro correction


pro gen_intmat_gui, PATH_PREFIX = PATH_PREFIX, INTMAT_ACQ_DIR = INTMAT_ACQ_DIR, OUTPUT_FILE = OUTPUT_FILE, SKIP_FRAME = SKIP_FRAME, AVG_FRAME = AVG_FRAME, NO_DISPLAY=NO_DISPLAY, NMODES = NMODES, INTERACTIVE = INTERACTIVE, SIG_CHECK = SIG_CHECK, SIG_MAX =SIG_MAX, SIG_MIN = SIG_MIN, NOISE_ESTIMATION = NOISE_ESTIMATION, REMOVE_TILT=REMOVE_TILT, BADMODES=BADMODES

REMOVE_FOCUS =0

; Saturation level defaults

if not keyword_set(SIG_MAX) then SIG_MAX = 0.7
if not keyword_set(SIG_MIN) then SIG_MIN = 0.06


n_modes = 672
n_slopes = 1600

if not keyword_set(PATH_PREFIX) then PATH_PREFIX=""

intmat = fltarr( n_modes, n_slopes)
intmat_counter = intarr(n_modes)        ; to keep track of averages over multiple files
intmat_normalize = fltarr(n_modes)      ; saved normalization factor 
mode_avg_counter = fltarr(n_modes)      ; how many transients per mode
devs = fltarr(n_modes)
mins = fltarr(n_modes)
maxs = fltarr(n_modes)

over_sig_max = fltarr(n_modes)
under_sig_min = fltarr(n_modes)
in_threshold = fltarr(n_modes)

;; -preparazione lookuptable display

filename = PATH_PREFIX + path_sep() + INTMAT_ACQ_DIR+"/system.fits"
dummy = readfits(filename, wfshdr)

pupname = get_fits_keyword( wfshdr, 'sc.PUPILS')
binning = get_fits_keyword( wfshdr, 'ccd39.BINNING')
if pupname eq '' then pupname = get_fits_keyword( wfshdr, 'PUPILS')

pupPath = getenv('ADOPT_ROOT')+"/calib/wfs/current/ccd39/LUTs"+ pupname + path_sep()

; Read slope reorder tables (for the signal display)
tx = read_ascii(pupPath +"slopex")
tx = tx.field1
dim = 80/binning
newdim_x = (80/dim)*dim*2
newdim_y = (80/dim)*dim


fx = fltarr(dim,dim)

tx_nonzero = where(tx ne -1)




; -------------------------------
; Multiple acquisition processing

INTMAT_ACQ_DIR += path_sep()

files = FILE_SEARCH( PATH_PREFIX + path_sep() + INTMAT_ACQ_DIR+'SwitchBCU_*acq?.sav')
files2 = FILE_SEARCH( PATH_PREFIX + path_sep() + INTMAT_ACQ_DIR+'SwitchBCU_*acq??.sav')
if not (files2[0] eq '') then files = [files,files2]
mm_raw = fltarr(n_modes,2)
rms_raw = fltarr(n_modes)

modes_cumulative = fltarr(n_modes, n_slopes)
modes_media = fltarr(600,n_modes)
modes_rms = fltarr(600,n_modes)
modes_counters = intarr(n_modes)

debug_plot=0

if debug_plot ne 0 then begin
    window,10 , xsize=newdim_x*13,retain=2
    window,11, xsize=newdim_x*13,retain=2
    window,12, xsize=newdim_x*13,retain=2
    	n0=0
	n1=0
	n2=0
endif

;doshift = [-4,-4,-4,-4,-2,-2]

for n=0, n_elements(files)-1 do begin
    drw_file = files[n]
    print, drw_file

    ; Get slopes plus MODAL_HISTORY_FILE and GET_PARAMS as an output
    slopes = process_drw( drw_file, MODAL_HISTORY_FILE = MODAL_HISTORY_FILE, GET_PARAMS = GET_PARAMS)

    if keyword_set(NOISE_ESTIMATION) then ext=1 else ext=0    
    mode_history = readfits( PATH_PREFIX + path_sep() + modal_history_file, EXTEN=ext)

    ; Detect transpose
    if n_elements(slopes[0,*] eq n_slopes) then slopes = transpose(slopes)

    n_frames = n_elements(slopes[*,0])

    ; Extract an integer number of modal history cycles
    histlen = n_elements(mode_history[*,0])
    cycles = n_frames / histlen
    new_n_frames = histlen * cycles

    slopes = slopes[0:new_n_frames-1, *]
    n_frames = new_n_frames

    ; Multiply mode history if there is more than one cycle
    mm = mode_history
    if cycles gt 1 then begin
        for i=1,cycles-1 do begin
            mm = [mm, mode_history]
        endfor
    endif
    mode_history =mm

    ; Detect modes for this acquisition

    goodmodes = where( total(abs(mode_history),1) ne 0)
    n_goodmodes = n_elements(goodmodes)

    amp_modes = fltarr(n_modes)

    goodslopes = where( total(abs(slopes),1) ne 0)
    n_goodslopes = n_elements(goodslopes)

    ; Calculate period using the first mode detected

    epsilon = 1e-20

    dev_m1 = fltarr(histlen-1)
    for j=0,n_elements(dev_m1)-2 do dev_m1[j] = abs(mode_history[j+1,goodmodes[0]] - mode_history[j,goodmodes[0]])
    steps = where(dev_m1 gt epsilon)
    period = steps[1]-steps[0]

    ; SHIFT TEST


 if not keyword_set(doshift) then begin
    slopes_ok = slopes
    dev_hist = fltarr(n_frames-1)

    do_shift = 6
    shift_start_mode=0
    stdev = fltarr(do_shift,10)
    pv    = fltarr(do_shift,10)
    images = fltarr(do_shift,10,newdim_x, newdim_y)

    for ss=0,do_shift-1 do begin

     s = ss- (do_shift-1)
     slopes = shift(slopes_ok, s, 0)

        for i=0,9 do begin


        ; Extract modulation
        hist = mode_history[*,goodmodes[i+shift_start_mode]]

        ; Search for steps
        for j=0,n_elements(dev_hist)-2 do dev_hist[j] = hist[j+1]-hist[j]
        transients = where(dev_hist lt -epsilon, count)

        ; "transients" sono gli indici dei frames PRIMA di uno step verso il basso
        n_trans = n_elements(transients)

	; Initialized to zero
        s_diff = fltarr( n_slopes)
		
        for t=0,n_trans-1 do begin
            s_up = slopes[ transients[t]+SKIP_FRAME+1 : transients[t]+SKIP_FRAME+1 + AVG_FRAME-1 , *]
            s_up = total(s_up, 1)/AVG_FRAME

            s_down = slopes[ -period + transients[t]+SKIP_FRAME+1 : -period + transients[t]+SKIP_FRAME+1 + AVG_FRAME-1 , *]
            s_down = total(s_down, 1)/AVG_FRAME

            ; Accumulate slopes for later averaging
            s_diff += (s_up - s_down)
	endfor

        fkfkf = s_diff[ tx[tx_nonzero]]
        fx[tx_nonzero] = fkfkf
        pv[ss,i] = max(fx)-min(fx)
        stdev[ss,i] = stddev(fkfkf)
        images[ss,i, * , *] = rebin(fx[*,dim/2:dim-1],newdim_x,newdim_y,/SAMPLE)

    endfor
    endfor
    slopes = slopes_ok

   ; Trova lo shift migliore

    pvmax = fltarr(10)
    rmsmax = fltarr(10)
    sok = fltarr(10)
    for mmm=0,9 do begin
	pvmax[mmm] = (where(pv[*,mmm] eq max(pv[*,mmm])))[0]
	rmsmax[mmm] = (where(stdev[*,mmm] eq max(stdev[*,mmm])))[0]
	if pvmax[mmm] eq rmsmax[mmm] then begin
		print,'Modo ',mmm,' shift ',pvmax[mmm]- (do_shift-1)
		sok[mmm] = pvmax[mmm]- (do_shift-1)
	endif else begin	
		print,'Modo ',mmm,' shift non trovato'
		sok[mmm] = -9999
	endelse
    endfor
    sss = median(sok)
    ;sss = -9999   ; Uncomment this line to force interactive shift
    if sss ne -9999 then begin
        print, 'Shift calcolato: ', sss
    endif else begin

      ; Shift interattivo
     window,15,title='shift test', xsize= newdim_x*10, ysize=newdim_y*10, retain=2
     for i=0,9 do begin
      for ss=0, (do_shift-1) do begin

        tvscl, images[ss,i,*,*], i+ss*10

       s = ss- (do_shift-1)
       print, 'Shift ',s, ' pv ',pv[ss,i], 'rms', stdev[ss,i]
      endfor
    endfor
     sss = 0
     read, sss, prompt="Shift migliore:"
    endelse
  


    slopes = shift(slopes,sss,0)
  endif else begin

	slopes = shift(slopes, doshift[n], 0)
         print, 'Shift di ',doshift[n]

  endelse

; Remove pathological cases
    idx = where(abs(slopes) gt 1e6, count)
    if count gt 0 then slopes[idx] =0


    ; Iterate over the modes of this acquisition and accumulate slopes in the intmat
    for i=0,n_goodmodes-1 do begin

        ; Extract modulation
        hist = mode_history[*,goodmodes[i]]

        ; Search for steps
        dev_hist = fltarr(n_frames-1)
        for j=0,n_elements(dev_hist)-2 do dev_hist[j] = hist[j+1]-hist[j]
        transients = where(dev_hist lt -epsilon, count)

        ; "transients" sono gli indici dei frames PRIMA di uno step verso il basso
        n_trans = n_elements(transients)
	if transients[0] eq -1 then begin
		print, 'No mode push-pull found!'
		exit
	endif


        s_diff = fltarr( n_slopes)
        s_up_acc = fltarr( n_slopes)
        s_down_acc = fltarr( n_slopes)
        for t=0,n_trans-1 do begin
            s_up = slopes[ transients[t]+SKIP_FRAME+1 : transients[t]+SKIP_FRAME+1 + AVG_FRAME-1 , *]
            s_up = total(s_up, 1)/AVG_FRAME

            s_down = slopes[ -period + transients[t]+SKIP_FRAME+1 : -period + transients[t]+SKIP_FRAME+1 + AVG_FRAME-1 , *]
            s_down = total(s_down, 1)/AVG_FRAME

;            print,'UP:', -period + transients[t]+SKIP_FRAME+1,  -period + transients[t]+SKIP_FRAME+1 + AVG_FRAME-1
;            print,'DOWN:  ',transients[t]+SKIP_FRAME+1, transients[t]+SKIP_FRAME+1 + AVG_FRAME-1
            mm = minmax([s_up, s_down])
            rms = stddev([s_up, s_down])
            mm_raw[ goodmodes[i], *] = minmax( [ mm_raw[ goodmodes[i],*], transpose(mm)])
            rms_raw[ goodmodes[i]] += rms


            ; Accumulate slopes for later averaging
            s_diff += (s_up - s_down)
            s_up_acc += s_up
            s_down_acc += s_down

if debug_plot ne 0 then begin
	if i eq 1 then begin
		wset,10
        	fkfkf = s_up[ tx[tx_nonzero]]
        	fx[tx_nonzero] = fkfkf
    	    	tvscl,rebin(fx[*,dim/2:dim-1],newdim_x,newdim_y,/SAMPLE), n0

        	fkfkf = s_down[ tx[tx_nonzero]]
        	fx[tx_nonzero] = fkfkf
    	    	tvscl,rebin(fx[*,dim/2:dim-1],newdim_x,newdim_y,/SAMPLE), n0+1

		ddd = s_up - s_down

        	fkfkf = ddd[ tx[tx_nonzero]]
        	fx[tx_nonzero] = fkfkf
    	    	tvscl,rebin(fx[*,dim/2:dim-1],newdim_x,newdim_y,/SAMPLE), n0+2
 
	n0+=3
	endif
	if i eq 200 then begin
		wset,11
        	fkfkf = s_up[ tx[tx_nonzero]]
        	fx[tx_nonzero] = fkfkf
    	    	tvscl,rebin(fx[*,dim/2:dim-1],newdim_x,newdim_y,/SAMPLE), n1

        	fkfkf = s_down[ tx[tx_nonzero]]
        	fx[tx_nonzero] = fkfkf
    	    	tvscl,rebin(fx[*,dim/2:dim-1],newdim_x,newdim_y,/SAMPLE), n1+1

		ddd = s_up - s_down

        	fkfkf = ddd[ tx[tx_nonzero]]
        	fx[tx_nonzero] = fkfkf
    	    	tvscl,rebin(fx[*,dim/2:dim-1],newdim_x,newdim_y,/SAMPLE), n1+2
 
	n1+=3
	endif
	if i eq 399 then begin
		wset,12
        	fkfkf = s_up[ tx[tx_nonzero]]
        	fx[tx_nonzero] = fkfkf
    	    	tvscl,rebin(fx[*,dim/2:dim-1],newdim_x,newdim_y,/SAMPLE), n2

        	fkfkf = s_down[ tx[tx_nonzero]]
        	fx[tx_nonzero] = fkfkf
    	    	tvscl,rebin(fx[*,dim/2:dim-1],newdim_x,newdim_y,/SAMPLE), n2+1

		ddd = s_up - s_down

        	fkfkf = ddd[ tx[tx_nonzero]]
        	fx[tx_nonzero] = fkfkf
    	    	tvscl,rebin(fx[*,dim/2:dim-1],newdim_x,newdim_y,/SAMPLE), n2+2
 
	n2+=3
	endif
endif

	    ; Convergence calculation
	    modes_cumulative[goodmodes[i], *] += (s_up - s_down)
            modes_counters[goodmodes[i]] +=1
            modes_media[ modes_counters[i], goodmodes[i]] = mean(modes_cumulative[ goodmodes[i], 0:*:2]) / modes_counters[goodmodes[i]]
            modes_rms[ modes_counters[i], goodmodes[i]] = sqrt(total((modes_cumulative[ goodmodes[i], 0:*:2] /  modes_counters[goodmodes[i]]) ^2)/n_elements(modes_cumulative[ goodmodes[i], 0:*:2]))

        endfor

if debug_plot ne 0 then begin
	if i eq 1 then begin
		wset,10
        	fkfkf = s_diff[ tx[tx_nonzero]]
        	fx[tx_nonzero] = fkfkf
    	    	tvscl,rebin(fx[*,dim/2:dim-1],newdim_x,newdim_y,/SAMPLE), n0
		n0 +=1
	n0 = (n0/11+1)*11
	endif



	if i eq 200 then begin
		wset,11
        	fkfkf = s_diff[ tx[tx_nonzero]]
        	fx[tx_nonzero] = fkfkf
    	    	tvscl,rebin(fx[*,dim/2:dim-1],newdim_x,newdim_y,/SAMPLE), n1
		n1 +=1
 
	n1 = (n1/11+1)*11
	endif

	if i eq 399 then begin
		wset,12
        	fkfkf = s_diff[ tx[tx_nonzero]]
        	fx[tx_nonzero] = fkfkf
    	    	tvscl,rebin(fx[*,dim/2:dim-1],newdim_x,newdim_y,/SAMPLE), n2
		n2 +=1
 
	n2 = (n2/11+1)*11
	endif
endif


        ; Compute averages
        s_diff /= n_trans
        s_up_acc /= n_trans
        s_down_acc /= n_trans

        ;; -----------------------
        ;; Check raw signals saturation

        ; Slopes under the minimum signal  --> use diff slopes
        check_slopes = where(abs(s_diff) ne 0,count_check)
        if count_check gt 0 then begin
           dummy = where( abs(s_diff[check_slopes]) lt SIG_MIN, count)
           under_sig_min[ goodmodes[i]] = count  / (n_goodslopes*2.0)
	endif

        ; Slopes over the maximum signal ---> use raw slopes
        s_max = fltarr(n_slopes)
        for iii=0,n_slopes-1 do s_max[iii] = max([abs(s_up_acc[iii]), abs(s_down_acc[iii])])

        if count_check gt 0 then begin
           dummy = where( s_max[check_slopes] gt SIG_MAX, count)
           over_sig_max[ goodmodes[i]] = count  / (n_goodslopes*2.0)
	endif

        ; Slopes OK
        in_threshold[ goodmodes[i]] = 1.0 - under_sig_min[ goodmodes[i]] - over_sig_max[ goodmodes[i]] 



        ; Accumulate the slope difference, normalized for the applied step
        mmamp = minmax(hist)
        intmat[goodmodes[i],*] += s_diff  / (mmamp[1]-mmamp[0])
        mode_avg_counter[ goodmodes[i]] += n_trans
        intmat_counter[ goodmodes[i]] += 1

        ; Save normalization factor
        intmat_normalize[ goodmodes[i]] = (mmamp[1]-mmamp[0])

        amp_modes[ goodmodes[i]] = mmamp[1]

    endfor


endfor  ; files

;wait,1000
;stop

;    plot,convergenza,psym=-4
;    wait,100


n=min([n_goodmodes, 100])
modes_per_window = 70
for i=0,n_goodmodes-1 do begin

        if i mod modes_per_window eq 0 then begin
            window,i/modes_per_window, xsize=3200, ysize=1200,retain=2           
            !p.multi = [0, 21, 10, 0, 0]
        endif
        idx = where(modes_media[*,goodmodes[i]] ne 0, count_idx)
        if count_idx gt 0 then begin
		plot, modes_media[idx, goodmodes[i]], charsize=2, xmargin=[0,0], ymargin=[0,0];, ytickname=replicate(' ',n_elements(idx))
       		 oplot,[0,100],[0,0]
		plot, modes_rms[idx, goodmodes[i]], charsize=2, xmargin=[0,0], ymargin=[0,0], color=256L*255;, ytickname=replicate(' ',n_elements(idx))
		plot, modes_media[idx, goodmodes[i]] / modes_rms[idx, goodmodes[i]], charsize=2, xmargin=[0,0], ymargin=[0,0], color=255L;, ytickname=replicate(' ',n_elements(idx))
	endif
       	 xyouts, 0, 0, strtrim(goodmodes[i])
endfor

!p.multi=0


window,5, retain=2
plot,in_threshold,psym=-4,yrange=[0,1], /ystyle, title="Raw signal saturation", charsize=2
oplot,over_sig_max,color=255L,psym=-4
oplot,under_sig_min,color=255L*256L*256L,psym=-4

dummy = readfits(path_prefix+ path_sep() + get_params.modal_disturb_file, modal_hdr)
amp_modes = readfits( path_prefix + path_sep() + get_fits_keyword( modal_hdr, 'PP_AMP_F'))

; Save data
save, amp_modes, under_sig_min, over_sig_max, in_threshold, filename= PATH_PREFIX + path_sep() + OUTPUT_FILE+'_saturation.sav'

pippo=''
if keyword_set(INTERACTIVE) then read, pippo, prompt="Enter to continue"

; If the saturation check is on, display data, save data and exit
if keyword_set(SIG_CHECK) then begin
    exit
endif
    

;; ------------------------
;; Average multiple acquisitions

for i=0,n_modes-1 do begin
    if intmat_counter[i] gt 1 then intmat[i,*] /= intmat_counter[i]
endfor

for i=0,n_modes-1 do begin
    if mode_avg_counter[i] gt 1 then rms_raw[i] /= mode_avg_counter[i]
endfor





;; --------------------------
;; Interactive section

; Start an outer iteration that will not exit until a last valid mode is selected by the user

print,"rms, min, max: all values are computed on (s_up-s_down)/2" 

;; Tip-tilt removal from intmat
if keyword_set(REMOVE_TILT) then begin
  rms_prima = fltarr(n_modes)
  rms_dopo  = fltarr(n_modes)
  if n_goodmodes gt 2 then begin
        tip  = reform(intmat[0,*])
        tilt = reform(intmat[1,*])

        ; Generate a tt-only reconstructor
        mat = intmat[0:1,*]
        svdc, mat, w, u, v
        winv = fltarr(2,2)
        winv[0,0] = 1./w[0]
        winv[1,1] = 1./w[1]
      
        ttrec = v ## winv ## transpose(u)

	for i=2,n_goodmodes-1 do begin
            m = goodmodes[i]

	    modo = reform(intmat[m,*])
	    rms_prima[m] = sqrt(total(modo^2)/n_elements(modo))

	    tt = ttrec ## modo

            modo -= tip * tt[0]
            modo -= tilt * tt[1]

            rms_dopo[m] = sqrt(total(modo^2)/n_elements(modo))

            print, 'Mode ',m,' Removed tip-tilt coefficients: ', tt[0], tt[1],'   - rsm from ',rms_prima[m],' to ',rms_dopo[m]
            intmat[m,*] = modo

        endfor

  endif
endif

;; Focus removal from intmat
if keyword_set(REMOVE_FOCUS) then begin
  ff_removed = fltarr(n_modes)
  rms_prima = fltarr(n_modes)
  rms_dopo  = fltarr(n_modes)
  if n_goodmodes gt 5 then begin
        focus  = reform(intmat[3,*])

        ; Generate a tt-only reconstructor
        mat = intmat[3,*]
        svdc, mat, w, u, v
        winv = fltarr(1,1)
        winv[0,0] = 1./w[0]
      
        focusrec = v ## winv ## transpose(u)

	for i=0,n_goodmodes-1 do begin
            if i eq 3 then continue
            m = goodmodes[i]

	    modo = reform(intmat[m,*])
	    rms_prima[m] = sqrt(total(modo^2)/n_elements(modo))

	    ff = focusrec ## transpose(modo)

            modo -= focus * ff[0]
            ff_removed[i] = ff[0]

            rms_dopo[m] = sqrt(total(modo^2)/n_elements(modo))

            print, 'Mode ',m,' Removed focus coefficient: ', ff,'   - rsm from ',rms_prima[m],' to ',rms_dopo[m]
            intmat[m,*] = modo
        endfor

    ; Calcoliamo il coefficiente da togliere al fuoco come la media dei modi accanto
    ff_fuoco = (ff_removed[2]+ff_removed[4])/2.0
    intmat[3,*] -= focus * ff_fuoco
    print, 'Mode ',3,' Removed focus coefficient: ', ff_fuoco
  endif
endif


;intmat = viscacha_correction(intmat)


while 1 do begin

    for i=0,n_modes-1 do begin

        ; Recover original slopes
        s_diff = intmat[i,*] * intmat_normalize[i]
        if total(abs(s_diff)) eq 0 then begin
            print, 'Skipping mode',i
            continue
        endif
    
        ; Faster version
        slopes = s_diff[ tx[tx_nonzero]]
        fx[tx_nonzero] = slopes

        devs[i] = stddev(slopes/2.0)
        mins[i] = min(slopes/2.0)
        maxs[i] = max(slopes/2.0)
 
        print,"Mode # (rms) (min,max) (n_trans): ", i, ' ('+strtrim(devs[i],2)+') ('+strtrim(mins[i],2)+','+strtrim(maxs[i],2)+') ('+strtrim(mode_avg_counter[i],2)+')'

        if not keyword_set(NODISPLAY) then begin
            modes_per_window=100
            if i mod modes_per_window eq 0 then window, i/modes_per_window, xsize=1600, ysize=900, retain=2
    	    tvscl,rebin(fx[*,dim/2:dim-1],newdim_x,newdim_y,/SAMPLE), (i - i/modes_per_window*modes_per_window)
    	    ;tvscl,rebin(fx,newdim_x,newdim_y,/SAMPLE), (i - i/modes_per_window*modes_per_window)
        endif

    endfor

    ; Automatic exit for non-interactive programs
    if keyword_set(NMODES) then begin
        LAST = NMODES-1
        break
    endif

    if keyword_set(INTERACTIVE) then begin
        read, LAST, prompt="Enter last valid mode (-1 to repeat):"
        if LAST ne -1 then break
    endif

endwhile

window,2, retain=2
plot,maxs,psym=-4,yrange=minmax(mm_raw), /ystyle, title="Slopes min/max/rms"
oplot,mm_raw[*,0], color=128L*256
oplot,mm_raw[*,1],color=128L
oplot,rms_raw,color=255L*256*256
oplot,mins,color=255L
oplot,devs,psym=-4,color=255L*256

dummy=''
if keyword_set(INTERACTIVE) then begin
    read, dummy, prompt="Enter to continue"
endif

; Cut away rejected modes. Special case for LAST=2: include focus
if LAST eq 2 then begin
    intmat = [intmat[0:1,*], intmat[3,*]]
endif else begin
    intmat = intmat[0:LAST, *]
endelse

; Binning 2 mode cut

;if binning eq 2 then begin
;	badmodes = [128, 130, 140, 142, 143, 146, 147, 149]
;	n = n_elements(intmat[*,0])
;	if n ge badmodes[0] then begin
;		print,'Binning=2! Applying "testadimoro" correction'
;		intmat[badmodes,*]=0
;	endif
;endif

;BADMODES=[235, 236, 260, 261, 288, 289, 310, 311, 312, 343, 344, 369, 370, 371, 372, 373, 378, 379, 388, 389, 398, 399]


 ; Bad modes in KL_v102
;BADMODES = [370]

;BADMODES = [114,115,116,131,132,146]


if keyword_set(BADMODES) then begin
	n = n_elements(intmat[*,0])
	if n ge BADMODES[0] then begin
		print,'WARNING:  REMOVING MODES: '
		print,BADMODES
		intmat[BADMODES,*]=0
                w = where(total(intmat,2) ne 0)
                intmat = intmat[w,*]
	endif
endif



;      12345678
hdr =['M2C',     GET_PARAMS.M2C, $
      'BINNING', strtrim(binning,2), $
      'M_DIST_F', GET_PARAMS.MODAL_DISTURB_FILE, $
      'C_DIST_F', GET_PARAMS.CMD_DISTURB_FILE, $
      'IM_ACQ_D', INTMAT_ACQ_DIR, $
      'IM_MODES', strtrim(LAST+1,2), $
      'SKIP_FRM', strtrim(SKIP_FRAME,2), $
      'AVG_FRM', strtrim(AVG_FRAME,2), $
      'pupils', get_fits_keyword( wfshdr, 'sc.PUPILS')]
    

if keyword_set(OUTPUT_FILE) then begin 
        writefits, PATH_PREFIX + path_sep() + OUTPUT_FILE, intmat, $
                   make_fits_header( intmat, HDR = hdr, FILETYPE='intmat', APPEND_HDR = wfshdr)

        save,maxs,mins,devs,mm_raw,rms_raw,filename= PATH_PREFIX + path_sep() + OUTPUT_FILE+'_data.sav'
endif

end

