
; Prende nmodi da intmat1 e li sostituisce ai modi corrispondenti di intmat2

pro giunta_intmat, intmat1, intmat2, nmodi, BASE = BASE

if not keyword_set(BASE) then BASE = 'KL_v2'
path = getenv('ADOPT_ROOT')+'/calib/'+format_side(getenv('ADOPT_SIDE'))+'/adsec/M2C'+base+'/RECs/'

filename1 = path+'Intmat_'+intmat1+'.fits'
filename2 = path+'Intmat_'+intmat2+'.fits'

im1 = readfits(filename1)
im2 = readfits(filename2)

nmodi1 = n_elements(im1[*,0])
if nmodi1 lt nmodi then begin
	print,'First intmat is too small: ',nmodi1,' modes instead of ',nmodi
	return
endif

im2[0:nmodi-1,*] = im1[0:nmodi-1, *]

tracknum = get_tracknum()
intmatfile = 'Intmat_'+tracknum+'.fits'
recfile = 'Rec_'+tracknum+'.fits'

writefits, path+intmatfile, im2

gen_reconstructor_gui, PATH_PREFIX=path, INPUT_FILE=intmatfile, OUTPUT_FILE = recfile, FINAL_DIM = [1600,672]

print,'Written intmat/Rec ',tracknum


end
