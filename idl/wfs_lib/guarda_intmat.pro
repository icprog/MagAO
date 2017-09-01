;
; Versione di gen_intmat specializzata per l'uso dalla GUI. Cambiano solo alcuni parametri di input/output
; PATH_PREFIX: prefix for all files
; INTMAT_ACQ_DIR : directory where the .sav files from SwitchBCU are located, with trailing slash. They also contains a reference to the modal disturb file
; OUTPUT_FILE: .fits file where the intmat is saved
;
; SKIP_FRAME: how many frames to skip after a push-pull step
; AVG_FRAME : how many frames to average
;


pro guarda_intmat, FILE1 = FILE1, FILE2 = FILE2, SCALE=SCALE

if not keyword_set(SCALE) then SCALE=1

binning=1

n_modes = 672
n_slopes = 1600


intmat1 = fltarr( n_modes, n_slopes)
intmat1[0,0] = readfits(FILE1, wfshdr1)

intmat2 = fltarr( n_modes, n_slopes)
intmat2[0,0] = readfits(FILE2, wfshdr2)

;; --------------------------
;; Interactive section

; Start an outer iteration that will not exit until a last valid mode is selected by the user

print,"rms, min, max: all values are computed on (s_up-s_down)/2" 

;filename =  '/towerdata/adsec_calib/M2C/mirrorStandard/intmatAcq/20090617_190718/system.fits'
;dummy = readfits(filename, wfshdr)

pupname1 = get_fits_keyword( wfshdr1, 'pupils')
pupname2 = get_fits_keyword( wfshdr2, 'pupils')

pupPath1 = getenv('ADOPT_ROOT')+"/calib/wfs/current/ccd39/LUTs/"+ pupname1 + path_sep()
pupPath2 = getenv('ADOPT_ROOT')+"/calib/wfs/current/ccd39/LUTs/"+ pupname2 + path_sep()

; Read slope reorder tables (for the signal display)
tx1 = read_ascii(pupPath1 +"slopex")
tx1 = tx1.field1
tx2 = read_ascii(pupPath2 +"slopex")
tx2 = tx2.field1
dim = 80/binning

fx1 = fltarr(dim,dim)
fx2 = fltarr(dim,dim)

tx1_nonzero = where(tx1 ne -1)
tx2_nonzero = where(tx2 ne -1)


    for i=0,n_modes-1 do begin

        ; Recover original slopes
        s_diff1 = intmat1[i,*] 
        if total(abs(s_diff1)) eq 0 then begin
            print, 'Skipping mode',i
            continue
        endif
    
        ; Faster version
        slopes1 = s_diff1[ tx1[tx1_nonzero]]
        fx1[tx1_nonzero] = slopes1

        ; Recover original slopes
        s_diff2 = intmat2[i,*] 
        if total(abs(s_diff2)) eq 0 then begin
            print, 'Skipping mode',i
            continue
        endif
    
        ; Faster version
        slopes2 = s_diff2[ tx2[tx2_nonzero]]
        fx2[tx2_nonzero] = slopes2

        newdim_x = (80/dim)*dim*4
        newdim_y = (80/dim)*dim

	newdim_x *= SCALE
	newdim_y *= SCALE

        mm1 =minmax(fx1)
	mm2 =minmax(fx2)
        pv1 =mm1[1]-mm1[0]
	pv2 =mm2[1]-mm2[0] 
	if pv2 eq 0 then pv2 =1e-6
	f = pv1/pv2
        fx = [fx1,fx2*f]

        if not keyword_set(NODISPLAY) then begin
            modes_per_window=900 / newdim_y
            if i mod modes_per_window eq 0 then window, i/modes_per_window, xsize=newdim_x, ysize=900, retain=2
    	    tvscl,rebin(fx[*,dim/2:dim-1],newdim_x,newdim_y,/SAMPLE), (i - i/modes_per_window*modes_per_window)
    	    ;tvscl,rebin(fx,newdim_x,newdim_y,/SAMPLE), (i - i/modes_per_window*modes_per_window)
	    if i eq 1 then stop
        endif

        ;stop
    endfor


dummy = ''
read, dummy, prompt="Enter to continue"

end
