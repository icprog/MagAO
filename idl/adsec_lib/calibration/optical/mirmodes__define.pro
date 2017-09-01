;+
; Defines an object representing a mirror modes set.
;
; INPUTS:
;
;	OPD_DIR_LIST	: (String array) List of tracknums (directories) containing 4D processed (.sav) measurements.
;
;	DATA_DIR		: (String) Directory where tracknum directories are located.
;
;	OC				: (Float) Percentage of central obscuration
;
;	MODEBORDER		: (long) From mode#0 to #modeborder only PISTON will be removed. From #modeborder+1
;						onwards, PISTON, TIP, AND TILT will be removed.
;-
FUNCTION mirmodes::Init, opd_dir_list, data_dir=data_dir, oc=oc, modeborder=modeborder

	if n_elements(data_dir) eq 0 then data_dir = '/towerdata/adsec_calib/if_functions'

	if not file_test(data_dir,/dir) then begin
		message, 'DATA_DIR not found!',/info
		return,0
	endif

	if test_type(opd_dir_list, /string, N_ELEMENTS=ndirs) then begin
		message, 'opd_dir_list must be a string array',/info
		return,0
	endif

	self._data_dir = data_dir
	self._ndirs = ndirs
	self._opd_dir_list = ptr_new(opd_dir_list)
	self._save_dir = data_dir+path_sep()+'mirmodes_'+(strsplit(opd_dir_list[0], '_', /extract))[0]
	if not file_test(self._save_dir,/dir) then file_mkdir, self._save_dir

	adsec_save_file = filepath(root=data_dir, sub=opd_dir_list[0], 'adsec_save.sav')
	if not file_test(adsec_save_file) then begin
		message, 'adsec_save.sav file not found', /info
		return,0
	endif
	restore, adsec_save_file
	self._adsec_save = ptr_new(adsec_save,/no_copy)

	if not keyword_set(oc) then self._oc = 0.111 else self._oc = oc
	if not keyword_set(modeborder) then self._modeborder = 10L
	self._radmax = 0.995

	save, opd_dir_list, filename=filepath(root=self._save_dir, 'tracknum_list.sav')
	return,1
end

function mirmodes::data_dir
	return, self._data_dir
end

function mirmodes::opd_dir_list
	return, *(self._opd_dir_list)
end

function mirmodes::ndirs
	return, self._ndirs
end

function mirmodes::save_dir
	return, self._save_dir
end

function mirmodes::adsec_save
	return, *self._adsec_save
end

function mirmodes::n_w_acts
	return, n_elements((*self._adsec_save).act_w_cl)
end

;+
; Shows the data on the info.txt file
;-
pro mirmodes::summary_opds

	opd_dir_list = self->opd_dir_list()
	ndirs = self->ndirs()
	ncycles = lonarr(ndirs)
	amp = fltarr(ndirs)
	firstlast = fltarr(ndirs,2)
	missing = bytarr(ndirs)
	valid_dir = bytarr(ndirs)

	sort_opd_dir_list = opd_dir_list[sort(opd_dir_list)]

	for ii=0, ndirs-1 do begin
		infofile = filepath('info.txt', root=self._data_dir, sub=sort_opd_dir_list[ii])
		if not file_test(infofile) then begin
			message, infofile+' does not exist',/info
			continue
		endif
		sss = read_ascii_structure(infofile)
		ncycles[ii] = n_elements(sss.template)
		amp[ii] = sss.mode_amp * 1e9	;nm
		file_list = file_search(filepath(root=self._data_dir, sub=sort_opd_dir_list[ii], "mode_*.sav"), count=nmodes)
		sort_file_list = file_list[sort(file_list)]
		modenum = fix(strmid(sort_file_list, 6,3, /reverse))
		firstlast[ii,*] = minmax(modenum)
		missing_modes = setdifference(indgen(max(modenum)-min(modenum)+1)+min(modenum), modenum, success=found_diff)
		missing[ii] = found_diff
		valid_dir[ii] = 1B
	endfor

	print, format='(%"tracknum \t\t nn \t amp [nm] \t first \t  last \t missing")'
	for ii=0, ndirs-1 do $
  	  if valid_dir[ii] then print, sort_opd_dir_list[ii], ncycles[ii], amp[ii], firstlast[ii,0], firstlast[ii,1], missing[ii] $
							   , FORMAT='(%"%s \t %d \t %6.1f \t \t %d \t %d \t %b ")'

end

;+
; Get all opds from different folders, and save them in the save_dir folder
; NOTE: if several measurements of the same mode exist, the average opd is computed.
;-
pro mirmodes::collect_opds

	ndirs = self->ndirs()
	opd_dir_list = self->opd_dir_list()

	;get all files:
	nfiles = 0
	for ii=0, ndirs-1 do begin
		file_list1 = file_search(filepath(root=self._data_dir, sub=opd_dir_list[ii], "mode_*.sav"), count=count)
		nfiles += count
		if ii eq 0 then file_list = file_list1 else file_list=[file_list,file_list1]
	endfor

	nmodes = max(fix(strmid(file_list, 6,3, /reverse)))

	for ii=0, nmodes do begin
		idx = where(strmatch( file_list, '*mode_'+string(ii,format='(I03)')+'*'))
		if idx[0] eq -1 then continue
		nn = n_elements(idx)
		for jj=0, nn-1 do begin
			restore, file_list[idx[jj]]
			if jj eq 0 then begin
				umask = mask
				amp1 = applied_amp
				if mode_normalized_to_amp then opd1 = opd else opd1 = opd/applied_amp
			endif else begin
				umask += mask
				amp1 += applied_amp
				if mode_normalized_to_amp then opd1 += opd else opd1 += opd/applied_amp
			endelse
		endfor
		applied_amp = amp1/float(nn)
		uidx = where(umask)
		avopd = opd1[uidx] / umask[uidx]
		opd = make_array(size(opd1,/dim), type=size(opd1,/type))
		opd[uidx] = avopd
		mask = umask gt 0
		mode_normalized_to_amp = 1
		save, opd, mask, applied_amp, mode_normalized_to_amp, filename=filepath('mode_'+string(ii,format='(I03)')+'.sav', root=self._save_dir)
	endfor
end


;+
; Mask Calibration
;
;-
pro mirmodes::mask_calibration, SHOW=SHOW

	save_dir = self->save_dir()

	; Directory where Influence function measurements are located;
	;------------------------------------------------------------
	file_list = file_search(save_dir, "mode_*.sav", count=nmodes)
	if nmodes eq 0 then begin
		self->collect_opds
		file_list = file_search(save_dir, "mode_*.sav", count=nmodes)
	endif
	print, 'number of mode_*.sav files found:', nmodes
	if nmodes eq 0 then return
	sort_file_list = file_list[sort(file_list)]

	; Restore first file to retrieve some variables
	restore, file_list[0], /VERB
	dim = size(opd,/dim)

	;Retrieve the masks:
	masktot = fltarr(dim[0],dim[1])
	validpix = fltarr(nmodes)
	for ii=0,nmodes-1 do begin
		restore, sort_file_list[ii]
		masktot = masktot + mask
		validpix[ii] = total(mask)
	endfor

	; Intersection of valid points
	intersect_mask  = masktot eq nmodes

	; Union of valid points
	union_mask = masktot gt 0

	save, masktot, intersect_mask, union_mask, validpix, filename=filepath(root=save_dir,'masks_data.sav'), /compress

	if keyword_set(SHOW) THEN image_show, union_mask-float(intersect_mask), /as, /sh, title='Union-Intersect'


	; Analysis based on Union mask:
	;--------------------------------------------------

	;fit union mask
	guess = [(dim[0]-1)/2., (dim[1]-1)/2., dim[0]/2., self._oc]
	pupfit1 = pupilfit(union_mask, GUESS=guess, MASK=fit_union_mask, /CIRCLE, DISP=SHOW)
	print, 'Union Mask:'
	print, 'summary fitting results:'
	print, 'pupil radius (pix): ', pupfit1[2]
	print, 'pupil center (pix): ', pupfit1[0:1]
	print, 'central obscuration (%)', pupfit1[3]

	IF keyword_set(SHOW) then begin
		window,/free
		image_show, fit_union_mask-float(union_mask), /as, /sh, title='fit_union_mask - union_mask
	ENDIF

	;Take fitting parameters from fitted union mask except for the OC (forced to self._oc)
	my_union_mask = mk_pupil(pupfit1[0], pupfit1[1], floor(pupfit1[2]), floor(pupfit1[2]), dim[0], dim[1], eps=self._oc)

	IF keyword_set(SHOW) then begin
		window,/free
		image_show, my_union_mask-float(union_mask), /as, /sh, title='my_union_mask - union_mask'
	ENDIF

	;find coordinates of valid points in union mask.
	;gmask_big = fit_union_mask
	gmask_big = my_union_mask
	idx_gmask_big = where(gmask_big)
	xx = idx_gmask_big mod dim[0]
	yy = idx_gmask_big / dim[0]
	xrange= minmax(xx)
	yrange= minmax(yy)
	DpupPix_a = xrange[1]-xrange[0] + 1
	DpupPix_b = yrange[1]-yrange[0] + 1
	DpupPix = max([DpupPix_a, DpupPix_b])
	if DpupPIx_a ne DpupPix_b then message, 'WARNING: Diameter in x and y directions not the same!',/info

	;extract union mask:
	gmask1 = gmask_big[xrange[0]:xrange[1],yrange[0]:yrange[1]]
	if DpupPix mod 2 ne 0 then Dpix = DpupPix + 1 else Dpix = DpupPix
	gmask = fltarr(Dpix,Dpix)
	gmask[0,0]=gmask1
	idx_gmask = where(gmask)

	IF keyword_set(SHOW) then begin
		window,/free
		plot, validpix/total(union_mask)*100., /ynozero, xtitle='mode number', ytitle='valid points (%)', xstyle=1, charsize=1.2
	ENDIF
	print, 'total pix in union mask:', total(union_mask)


	pupfit = pupfit1 ;select pupfit to save
	save, xrange, yrange, DpupPix, Dpix, pupfit, gmask_big, idx_gmask_big, gmask, idx_gmask $
		 , filename=filepath(root=save_dir,'union_mask_extraction.sav'), /compress


	;rebin by two the union mask
	Dpix_bin = Dpix/2
	gmask1_bin = rebin(gmask, Dpix_bin, Dpix_bin)
	idx_gmask_bin = where(gmask1_bin eq 1)
	gmask_bin = fltarr(Dpix_bin, Dpix_bin)
	gmask_bin[idx_gmask_bin] = 1.

	guess_bin_a = [(dpix_bin-1)/2., (dpix_bin-1)/2., dpix_bin/2., self._oc]
	pupfit_bin_a = pupilfit(gmask_bin, GUESS=guess_bin_a, MASK=fit_gmask_bin_a, /CIRCLE, DISP=SHOW)

	pupfit_bin = pupfit_bin_a	;;Select which pupfit to save
	xx_bin = ( (idx_gmask_bin mod Dpix_bin) - pupfit_bin[0]) / pupfit_bin[2]
	yy_bin = ( (idx_gmask_bin  /  Dpix_bin) - pupfit_bin[1]) / pupfit_bin[2]
	gmask_bin_1d = float(gmask_bin[idx_gmask_bin])

	save, gmask_bin, idx_gmask_bin, pupfit_bin, Dpix_bin, xx_bin, yy_bin, gmask_bin_1d $
		, filename=filepath(root=save_dir,'union_gmask_rebin.sav'), /compress


	; Analysis based on Intersection mask:
	;--------------------------------------------------

	;gmask_big = intersect_mask * fit_union_mask
	gmask_big = intersect_mask * my_union_mask
	idx_gmask_big = where(gmask_big)
	gmask1 = gmask_big[xrange[0]:xrange[1],yrange[0]:yrange[1]]
	gmask = fltarr(Dpix,Dpix)
	gmask[0,0]=gmask1
	idx_gmask = where(gmask)
	save, xrange, yrange, DpupPix, Dpix, pupfit, gmask_big, idx_gmask_big, gmask, idx_gmask $
		 , filename=filepath(root=save_dir,'intersection_mask_extraction.sav'), /compress

	; Produced a binned global mask
	gmask1_bin = rebin(gmask, Dpix_bin, Dpix_bin)
	idx_gmask_bin = where(gmask1_bin); eq 1)
	gmask_bin = fltarr(Dpix_bin, Dpix_bin)
	gmask_bin[idx_gmask_bin] = 1.
	gmask_bin_1d = float(gmask_bin[idx_gmask_bin])

	xx_bin = ( (idx_gmask_bin mod Dpix_bin) - pupfit_bin[0]) / pupfit_bin[2]
	yy_bin = ( (idx_gmask_bin  /  Dpix_bin) - pupfit_bin[1]) / pupfit_bin[2]

	save, gmask_bin, idx_gmask_bin, pupfit_bin, Dpix_bin, xx_bin, yy_bin, gmask_bin_1d $
		, filename=filepath(root=save_dir,'intersect_gmask_rebin.sav'), /compress
end


;+
; PRODUCE_IFMAT
;
; Generate IF matrix
;-
pro mirmodes::produce_ifmat, EXTRAPOL=EXTRAPOL
	save_dir = self->save_dir()
	big_mask_savfile    = filepath(root=save_dir,'intersection_mask_extraction.sav')
	binned_mask_savfile = filepath(root=save_dir,'intersect_gmask_rebin.sav')
	if ~file_test(big_mask_savfile) or ~file_test(binned_mask_savfile) then self->mask_calibration,/show
	if ~file_test(big_mask_savfile) or ~file_test(binned_mask_savfile) then return
	restore, big_mask_savfile
	restore, binned_mask_savfile

	file_list = file_search(save_dir, "mode_*.sav", count=nmodes)
	sort_file_list = file_list[sort(file_list)]

	; Restore first file to retrieve some variables
	restore, file_list[0], /VERB
	dim = size(opd,/dim)
	mindim = min(dim)

	; Initialize variables
	np = n_elements(idx_gmask_bin)
	missing_np = lonarr(nmodes)
	mode_rms  = fltarr(nmodes)
	amp_mod = fltarr(nmodes)
	IFmatrix = dblarr(nmodes,np)

	coeffhist = fltarr(3,nmodes)
	filt_mode_rms = fltarr(nmodes)

	for ii=0, nmodes-1 do begin
		print, 'processing OPD number: ', strtrim(ii,2)
		restore, sort_file_list[ii]
		amp_mod[ii] = applied_amp
		if MODE_NORMALIZED_TO_AMP eq 0 then opd_temp = opd / amp_mod[ii] else opd_temp = opd	; Normalize IF to unitary command

		mode_idx = where(mask)
		xx1 = float(mode_idx mod dim[0])
		yy1 = float(mode_idx  /  dim[0])
		opd_temp_1 = (opd_temp - total(opd_temp*mask)/total(mask))*mask	;remove piston
		mode_rms[ii] = stddev(opd_temp_1[mode_idx])
		print, 'stddev:', mode_rms[ii]

		;extrapolate missing points
		mask_diff = (float(gmask_big) - mask)*gmask_big
		missing_np[ii] = total(mask_diff)
		print, 'number of missing points:', missing_np[ii]
		if keyword_set(EXTRAPOL) then begin
			if missing_np[ii] ne 0 then begin
				triangulate, xx1, yy1, tr, b
				opd_temp_2 = gmask_big * trigrid(xx1, yy1, opd_temp_1[mode_idx], tr $
                	  		, XOUT=indgen(dim[0]), YOUT=indgen(dim[1]), extrapolate=b )
    		endif else opd_temp_2 = opd_temp_1
    	endif else opd_temp_2 = opd_temp_1

		;extract and rebin
		opd_temp_3 = fltarr(Dpix,Dpix)
		opd_temp_3[idx_gmask] = opd_temp_2[idx_gmask_big]					;extract to a square array.
		opd_temp_4 = rebin(opd_temp_3, Dpix_bin, Dpix_bin)*gmask_bin		;rebin

		;remove some Zernike modes:
		if ii le self._modeborder then FILT_ZERN=[1] else FILT_ZERN = [1,2,3]

		fit = surf_fit(xx_bin,yy_bin,opd_temp_4[idx_gmask_bin],FILT_ZERN, COEFF=coeff, /ZERN)
		coeffhist[0,ii] = coeff
		opd_temp_4[idx_gmask_bin] = opd_temp_4[idx_gmask_bin] - fit
		filt_mode_rms[ii] = stddev(opd_temp_4[idx_gmask_bin])
		IFmatrix[ii,*] = opd_temp_4[idx_gmask_bin]	;extract only points valid in rebinned gmask
	endfor

	print, 'IFmatrix computed...'
	IFmatrix_fname = filepath(root=save_dir,'IFmatrix.sav')
	save, IFmatrix, missing_np, mode_rms, amp_mod, coeffhist, filt_mode_rms, filename=IFmatrix_fname, /compress
end


;+
; CHECK_NOISE
;
; Checks the level of noise
;-
pro mirmodes::check_noise

	IFmat_fname = filepath(root=self._save_dir,'IFmatrix.sav')
	if not file_test(IFmat_fname) then begin
		message, 'IFmatrix.sav not found',/info
		return
	endif
	restore, IFmat_fname
	nmodes = (size(IFmatrix,/dim))[0]
	np = float((size(IFmatrix,/dim))[1])

	;if n_elements(total_act_num) EQ 0 then total_act_num = nmodes
	total_act_num = self->n_w_acts()

	zerns_rms = fltarr(nmodes)
	for ii=0, nmodes-1 do zerns_rms[ii] = sqrt(total(coeffhist[1:*,ii]^2.))

	; normalized or not normalized, that is the question.
	;if keyword_set(NORM) then fact=amp_mod else fact=replicate(1.,nmodes)
	fact = amp_mod

	; modal amplitude, and modal rms
	;---------------------------------------
	window,/free
	plot_io, amp_mod*1e9, xtitle='mode number', ytitle='nm surf', xstyle=1, charsize=1.5, yrange=[1, max(amp_mod*1e9)]
	oplot, amp_mod*1e9/sqrt(float(total_act_num)), color=250L*255L
	oplot, filt_mode_rms*fact*1e9, color=250L ; else oplot, mode_rms*fact*1e9, color=250L
	legend, ['amplitude applied', 'theoretical surf rms', 'estimated surf rms'], linestyle=[0,0,0], color=[0, 250L*255L,250L], /bottom

	; spurious TT estimation:
	;------------------------------------------------------
	window,/free
	plot_io, filt_mode_rms*fact*1e9 , xtitle='mode number', ytitle='surf nm rms', xstyle=1, charsize=1.5, yrange=[1,max(filt_mode_rms*fact*1e9)]
	oplot, zerns_rms*fact*1e9, color=250
	legend, ['estimated surf rms', 'spurious TT rms'], linestyle=[0,0], color=[0,250L], charsize=1.0,/right
end


;+
; SHOWME_THE_MODES
;
; MAKES YOU SEE THE MODES IN 2D FOR EASY INSPECTION
;-
pro mirmodes::showme_the_modes, mode_num_idx=mode_num_idx, oc=oc, radmax=radmax, ct=ct, zoom=zoom, nrows=nrows, ncol=ncol

	if n_elements(oc) eq 0 and n_elements(radmax) eq 0 then begin
		IFmat_fname = filepath(root=self._save_dir,'IFmatrix.sav')
		binned_mask_savfile = filepath(root=self._save_dir,'intersect_gmask_rebin.sav')
	endif else begin
		if n_elements(oc) eq 0 then oc=self._oc
		if n_elements(radmax) eq 0 then radmax=self._radmax
		info = '_oc'+strtrim(string(oc,format='(f5.3)'),2)+'_maxr'+strtrim(string(radmax,format='(f5.3)'),2)
		IFmat_fname = filepath(root=self._save_dir,'IFmatrix'+info+'.sav')
		binned_mask_savfile = filepath(root=self._save_dir,'intersect_gmask_rebin'+info+'.sav')
	endelse

	if ~file_test(IFmat_fname) then begin
		message, IFmat_fname+' not found',/info
		return
	endif
	restore, IFmat_fname
	nmodes = (size(IFmatrix,/dim))[0]
	if n_elements(mode_num_idx) eq 0 then mode_num_idx = lindgen(nmodes)
	nmodes = n_elements(mode_num_idx)

	restore, binned_mask_savfile

	if not keyword_set(ct) then ct=3
	if not keyword_set(zoom) then zoom=0.25
	if ( (not keyword_set(nrows)) AND (not keyword_set(ncol)) ) then ncol=nmodes/3
	if not keyword_set(ncol)  then ncol  = ceil(nmodes/float(nrows))
	if not keyword_set(nrows) then nrows = ceil(nmodes/float(ncol))
	npag = ceil(nmodes/(float(nrows)*float(ncol)))

	loadct,ct
	window,/free,xsize=round(Dpix_bin*zoom)*ncol,ysize=round(Dpix_bin*zoom)*nrows
	erase,0

	temp = fltarr(Dpix_bin,Dpix_bin)
	for ii=0, nmodes-1 do begin
		temp[idx_gmask_bin] = IFmatrix[mode_num_idx[ii],*]
		tvscl, congrid(temp,round(Dpix_bin*zoom), round(Dpix_bin*zoom)),ii
	endfor

end


;+
; TWEAK_IFMAT
;
; ADJUSTS THE CENTRAL OBSCURAITON AND THE MAX RADIUS (0<RAD<1) OF THE MODES
;-
pro mirmodes::tweak_ifmat, oc=oc, radmax=radmax

	if n_elements(oc) eq 0 then oc=self._oc
	if n_elements(radmax) eq 0 then radmax=self._radmax

	IFmat_fname = filepath(root=self._save_dir,'IFmatrix.sav')
	if ~file_test(IFmat_fname) then begin
		message, 'IFmatrix.sav not found',/info
		return
	endif
	restore, IFmat_fname
	nmodes = (size(IFmatrix,/dim))[0]

	binned_mask_savfile = filepath(root=self._save_dir,'intersect_gmask_rebin.sav')
	restore, binned_mask_savfile

	;Compute a reduced mask with a 0.995 of the radius.
	;This mask basically eliminates a one-pixel ring at the perimeter.
	;==================================================================
	rr = sqrt(xx_bin^2 + yy_bin^2)
	redmask1d = gmask_bin_1d
	idx_out = where(rr ge radmax, complement=idx_in)
	redmask1d[idx_out] = 0.
	redmask = fltarr(dpix_bin,dpix_bin)
	redmask[idx_gmask_bin]=redmask1d
	nmask = total(gmask_bin)
	idx_redmask = where(redmask)
	n_redmask = total(redmask)


	;Increase the central obscuration:
	idx1 = where(rr gt oc)
	ocmask1d = gmask_bin_1d
	ocmask1d[idx1]=0.
	ocinv = fltarr(dpix_bin,dpix_bin)
	ocinv[idx_gmask_bin]=ocmask1d
	redmask = (redmask-ocinv)*redmask
	idx_redmask = where(redmask)
	n_redmask = total(redmask)

	idx_new = setintersection(idx_in,idx1)


	;Apply the mask to the saved IFmatrix:
	;==================================================================
	;window,0 & window,1

	IFmat_red = fltarr(nmodes,n_redmask)
	temp = fltarr(dpix_bin,dpix_bin)
	for ii=0, nmodes-1 do begin
		temp[idx_gmask_bin] = IFmatrix[ii,*]
		temp *= redmask
		IFmat_red[ii,*] = temp[idx_redmask]
	;	wset,0 & caca=fltarr(Dpix_bin,Dpix_bin) & caca[idx_gmask_bin]=IFmatrix[ii,*] & shade_surf, caca & aff, caca, zo=0.5
	;	wset,1 & shade_surf, temp & aff, temp, zo=0.5
	;	wait,0.05
	endfor

	;save results:
	;=================================================================
	IFmatrix = temporary(IFmat_red)
	info = '_oc'+strtrim(string(oc,format='(f5.3)'),2)+'_maxr'+strtrim(string(radmax,format='(f5.3)'),2)
	save, IFmatrix, filename=filepath(root=self._save_dir, 'IFmatrix'+info+'.sav'),/compress

	;Save modified pupil data:
	gmask_bin = redmask
	idx_gmask_bin = idx_redmask
	xx_bin = xx_bin[idx_new]
	yy_bin = yy_bin[idx_new]
	gmask_bin_1d = gmask_bin_1d[idx_new]

	;save, gmask_bin, idx_gmask_bin, pupfit_bin, Dpix_bin, xx_bin, yy_bin, gmask_bin_1d, filename=ASMmodes_dir+'intersect_gmask_rebin_redmask.sav', /compress
	save, gmask_bin, idx_gmask_bin, pupfit_bin, Dpix_bin, xx_bin, yy_bin, gmask_bin_1d, filename=filepath(root=self._save_dir,'intersect_gmask_rebin'+info+'.sav'), /compress
end


;+
; INV_IFMAT
;
; COMPUTES THE PSEUDO_INVERT OF THE IFMATRIX
;-
pro mirmodes::inv_ifmat, oc=oc, radmax=radmax, N_MODES_TO_DROP=N_MODES_TO_DROP

	if n_elements(N_MODES_TO_DROP) eq 0 then N_MODES_TO_DROP=1L
	if n_elements(oc) eq 0 and n_elements(radmax) eq 0 then begin
		IFmat_fname = filepath(root=self._save_dir,'IFmatrix.sav')
		binned_mask_savfile = filepath(root=self._save_dir,'intersect_gmask_rebin.sav')
	endif else begin
		if n_elements(oc) eq 0 then oc=self._oc
		if n_elements(radmax) eq 0 then radmax=self._radmax
		info = '_oc'+strtrim(string(oc,format='(f5.3)'),2)+'_maxr'+strtrim(string(radmax,format='(f5.3)'),2)
		IFmat_fname = filepath(root=self._save_dir,'IFmatrix'+info+'.sav')
		binned_mask_savfile = filepath(root=self._save_dir,'intersect_gmask_rebin'+info+'.sav')
	endelse

	if ~file_test(IFmat_fname) then begin
		message, IFmat_fname+' not found',/info
		return
	endif
	restore, IFmat_fname
	restore, binned_mask_savfile

	; Invert IFmatrix
	print, 'Inverting IFmatrix...'
	inv_IFmatrix = pseudo_invert(IFmatrix, EPS=1e-4, W_VEC=ww, U_MAT=uu, V_MAT=vv, INV_W=inv_ww,  IDX_ZEROS=idx, COUNT_ZEROS=count, /VERBOSE, N_MODES_TO_DROP=N_MODES_TO_DROP)

	window,/free
	nmodes = (size(IFmatrix,/dim))[0]
	caca = fltarr(dpix_bin,dpix_bin)
	caca[idx_gmask_bin] = IFmatrix ## vv[nmodes-1,*]
	image_show, caca,/as,/sh, title='last eigenmode mode'

	undefine, uu, vv
	print, 'IFmatrix inverted.'

	; Compute Cross-product matrix of IFs
	print, 'Computing DMcoupling...'
	np = total(gmask_bin)
	DMcoupling = transpose(IFmatrix) ## IFmatrix / float(np)

	if n_elements(info) eq 0 then fname = 'inv_IFmatrix.sav' else fname='inv_IFmatrix'+info+'.sav'
	save, DMcoupling, inv_IFmatrix, IFmatrix, filename=filepath(root=self._save_dir, fname), /compress
end



pro mirmodes::free
	if ptr_valid(self._opd_dir_list) then ptr_free, self._opd_dir_list
	if ptr_valid(self._adsec_save) then ptr_free, self._adsec_save
end

pro mirmodes__define
    struct = { mirmodes , $
    	_data_dir		   : ""			, $
    	_ndirs			   : 0L			, $
        _opd_dir_list      : ptr_new()	, $
        _adsec_save		   : ptr_new()  , $
        _save_dir		   : ""			, $
        _oc				   : 0.			, $
        _radmax			   : 0.			, $
        _modeborder		   : 0L			, $
        _recompute         : 0B           $
    }
end


