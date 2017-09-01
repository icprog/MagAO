; Bin a bin1 interaction matrix to the specified binning.
; Pupils must have been previously generated with the makePupils_fromBin1 python procedure,
; from the same pupils used to acquire the intmat
;
;
; base:            modal basis
; intmat_tracknum: tracking number of the intmat to bin
; binning:         target binning
; pupbinned_tracknum: tracking number of the binned pupils
; NMODES         : optional cut to max number of modes
; PUPBIN1        : optional bin1 pupils (if not specified in the intmat FITS header)
;

pro bin_intmat, base, intmat_tracknum, binning, pupbinned_tracknum, NMODES= NMODES, PUPBIN1 = PUPBIN1

prefix = getenv('ADOPT_ROOT')+'/calib/'+format_side(getenv('ADOPT_SIDE'))+'/adsec/M2C'
pup_prefix = getenv('ADOPT_ROOT')+'/calib/'+format_side(getenv('ADOPT_SIDE'))+'/WFS/ccd39/LUTs'

path = prefix+'/'+base+'/RECs/'
intmatfile = path+'Intmat_'+intmat_tracknum+'.fits'
intmat = readfits(intmatfile, hdr)
if not keyword_set(PUPBIN1) then begin 
    pupils = get_fits_keyword(hdr, 'pupils')
endif else begin
    pupils = '/bin1/'+PUPBIN1
endelse

pupilsdir = pup_prefix+pupils
indpup_bin1 = (read_ascii(pupilsdir+'/indpup')).field1
indpup1 = indpup_bin1[ 0:n_elements(indpup_bin1)/4-1]
nsubap1 = n_elements(indpup1)

pupilsdir_binned = pup_prefix+'/bin'+strtrim(binning,2)+'/'+pupbinned_tracknum
indpup_binned = (read_ascii(pupilsdir_binned+'/indpup')).field1
indpup2 = indpup_binned[ 0:n_elements(indpup_binned)/4-1]
nsubap2 = n_elements(indpup2)

f = fltarr(80,80)

if not keyword_set(NMODES) then nmodes = n_elements(intmat[*,0])
intmat_binned= fltarr(nmodes, 1600)

modes_per_window=100
dim=320
dim2 = 320
if binning eq 3 then dim2 = 312

for mode=0,nmodes-1 do begin

	j = mode*4

	if j mod modes_per_window eq 0 then window, j/modes_per_window, xsize=1600, ysize=900, retain=2

	; sx
	f[indpup1] = intmat[mode, 0:nsubap1*2-1:2]
	f_binned = bin_ccd39(f, binning) / binning^2
        intmat_binned[mode, 0:nsubap2*2-1:2] = f_binned[indpup2]

	; display
	tvscl,rebin(f,dim,dim,/sample), (j - j/modes_per_window*modes_per_window)
	tvscl,rebin(f_binned,dim2,dim2,/sample), (j - j/modes_per_window*modes_per_window)+1

        
	; sy
	f[indpup1] = intmat[mode, 1:nsubap1*2:2]
	f_binned = bin_ccd39(f, binning) / binning^2
        intmat_binned[mode, 1:nsubap2*2:2] = f_binned[indpup2]

	; display
	tvscl,rebin(f,dim,dim,/sample), (j - j/modes_per_window*modes_per_window)+2
	tvscl,rebin(f_binned,dim2,dim2,/sample), (j - j/modes_per_window*modes_per_window)+3

	print,'bin 1 mode ',mode,': minmax =', minmax(intmat[mode,*])
	print,'bin '+strtrim(binning,2)+' mode ',mode,': minmax =', minmax(intmat_binned[mode,*])

endfor
        

tracknum = get_tracknum()
intmatfile = 'Intmat_'+tracknum+'.fits'
recfile = 'Rec_'+tracknum+'.fits'

writefits, path+intmatfile, intmat_binned
gen_reconstructor_gui, PATH_PREFIX=path, INPUT_FILE=intmatfile, OUTPUT_FILE = recfile, FINAL_DIM = [1600,672]
print,'Written intmat/Rec ',tracknum


end
